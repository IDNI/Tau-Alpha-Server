//
// Created by ak on 17/09/2019.
//

#ifndef MESSAGINGSERVER_TCPCONNECTION_H
#define MESSAGINGSERVER_TCPCONNECTION_H

#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/scoped_array.hpp>

#include "./json.hpp"
#include "./UdpServer.h"
#include "./Message.h"

using json = nlohmann::json;

using boost::asio::ip::tcp;

class TcpConnection
        : public boost::enable_shared_from_this<TcpConnection> {
public:
    typedef boost::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context &io_context, UdpServer &udpServer, Routing &routing,
                          std::unordered_map<size_t, Message *> &messages) {
        return pointer(new TcpConnection(io_context, udpServer, routing, messages));
    }

    tcp::socket &socket() {
        return socket_;
    }

    void start() {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " waiting for command" << std::endl;
        //reading command
        //it can be "send message" or "get messages"
        boost::asio::async_read_until(socket_,
                                      inbuf,
                                      "\n",
                                      boost::bind(&TcpConnection::handleReadAction,
                                                  shared_from_this(),
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
    }

private:
    boost::system::error_code error;

    void executeAction() {
        if ("Send message" == actionData["action"].get<std::string>()) {
            runSendMessage();
        } else if ("Get message" == actionData["action"].get<std::string>()) {
            runGetMessage(actionData["messageId"].get<size_t>());
        }
    }

    std::tuple<char*, size_t> loadFileIntoTheMemory(std::string fileName) {
        std::ifstream inFile(fileName, std::ios::in | std::ios::binary);
        std::size_t fileSize = 0;
        if (!inFile) {
            std::cout << "Can't open the file " << fileName << std::endl;
        }

        inFile.seekg(0, inFile.end);
        fileSize = (std::size_t) inFile.tellg();
        inFile.seekg(0, inFile.beg);
        std::cout << "File size " << fileSize << std::endl;

        char *messageBuffer = new char[fileSize];

        inFile.read(messageBuffer, fileSize);
        if (inFile)
            std::cout << "Loaded file " << fileName << " into memory:"
                      <<messageBuffer<< " " << inFile.gcount() <<std::endl;
        else
            std::cerr << "Only " << inFile.gcount() << " could be read" << std::endl;

        inFile.close();

        return std::make_tuple(messageBuffer, fileSize);
    }

    void runGetMessage(size_t messageId) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        auto [messageBuffer, fileSize] = loadFileIntoTheMemory(
                std::to_string(messageId) + ".msg");
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        messageBuffer_ = messageBuffer;

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(messageBuffer, fileSize),
                                 boost::asio::transfer_all(),
                                 boost::bind(&TcpConnection::handleMessageSent,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void runSendMessage() {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        boost::asio::async_write(socket_,
                                 boost::asio::buffer("Ready for file\n"),
                                 boost::asio::transfer_all(),
                                 boost::bind(&TcpConnection::handleReadyForMessage,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void handleMessageSent(const boost::system::error_code &error,
                           std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " sent!" << std::endl;
        delete[] messageBuffer_;
        socket_.close();
    };

    void handleReadAction(const boost::system::error_code &error,
                          std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        processAction(error, bytes_transferred);

        if (actionData.size() == 0) return;

        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        executeAction();
    }

    TcpConnection(boost::asio::io_context &io_context, UdpServer &udpServer, Routing &routing,
                  std::unordered_map<size_t, Message *> &messages)
            : socket_(io_context),
              udpServer_(udpServer),
              routing_(routing),
              messages_(messages) {
    }

    void handleReadyForMessage(const boost::system::error_code &error,
                               std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        boost::asio::async_read(socket_,
                                inbuf,
                                boost::asio::transfer_all(),
                                boost::bind(&TcpConnection::handleMessage,
                                            shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void handleMessage(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << "got message size " << bytes_transferred << std::endl;

        std::string messageString = {buffers_begin(inbuf.data()), buffers_end(inbuf.data())};

        //we create a message in order to store and route it
        Message *message = new Message(messageString,
                                       actionData, // sender/recipients
                                       routing_, // routing table
                                       udpServer_ //for notification of recipients
        );

        //save to disk
        message->save(actionData, messageString);

        //notify recipients of a new message
        message->notifyRecipients(actionData);

        messages_[message->messageHash] = message;
    }

    void parseIncomingMessage(std::string &incomingMessage) {
        std::cout << "got message: "
                  << incomingMessage << std::endl;

        try {
            actionData = json::parse(incomingMessage);
        } catch (std::exception &e) {
            std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                      << std::dec << __LINE__ << " : " << e.what() << std::endl;
        }


        if (actionData.contains("action")) {
//            std::cout << "parsed message: "
//                      << actionData["action"].get<std::string>()
//                      << " size " << actionData.size()
//                      << std::endl;
        } else {
            actionData.clear();
        }
    }

    void processAction(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        std::string incomingMessage;
        std::getline(inputStream, incomingMessage);

        if (error)
            std::cerr << __FUNCTION__ << " error " << error.message() << " bytes_transferred " << bytes_transferred
                      << std::endl;

//        std::cout << "handleRead got: " << incomingMessage << std::endl;

        parseIncomingMessage(incomingMessage);

        return;
    }

    json actionData;

    tcp::socket socket_;
//    boost::array<char, 128> recv_buffer_{};
    char *messageBuffer_ = nullptr;
    boost::asio::streambuf inbuf;
    std::istream inputStream{&inbuf};
    std::string message_;
    UdpServer &udpServer_;
    Routing &routing_;
    std::unordered_map<size_t, Message *> messages_;

};


#endif //MESSAGINGSERVER_TCPCONNECTION_H
