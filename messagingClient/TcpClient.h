//
// Created by ak on 18/09/2019.
//

#ifndef MESSAGINGCLIENT_TCPCLIENT_H
#define MESSAGINGCLIENT_TCPCLIENT_H


#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "./json.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class TcpClient
        : public boost::enable_shared_from_this<TcpClient> {
public:
    typedef boost::shared_ptr<TcpClient> pointer;

    static pointer create(boost::asio::io_context &io_context) {
        return pointer(new TcpClient(io_context));
    }

    TcpClient(boost::asio::io_context &io_context)
            : io_context_(&io_context),
              socket_(io_context) {

    }

    bool connect(tcp::resolver::results_type &tcpEndpoint) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " connecting to "
//                  << tcpEndpoint->host_name()
//                  << ":"
//                  << tcpEndpoint->service_name()
//                  << std::endl;
        // std::cout << __FILE__ << " " << __FUNCTION__ << " io_context::stopped: " << io_context_ << " "
//                  << io_context_->stopped() << std::endl;

        boost::asio::connect(
                socket_,
                tcpEndpoint);
        return true;
    }

    bool getMessage(size_t messageId) {
        json msg;
        msg["action"] = "Get message";
        msg["messageId"] = messageId;

        messageId_ = messageId;

        boost::shared_ptr<std::string> message(
                new std::string(msg.dump()));

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(*message),
                                 boost::asio::transfer_all(),
                                 boost::bind(&TcpClient::sendEndOfLine,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

        return true;
    }

private:
    char *messageBuffer_ = nullptr;
    size_t messageBufferSize_ = 0;


    void sendEndOfLine(const boost::system::error_code &error, size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " writing" << std::endl;
        boost::asio::async_write(socket_,
                                 boost::asio::buffer("\n"),
                                 boost::asio::transfer_all(),
                                 boost::bind(&TcpClient::handleReadyForMessage,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

    };

    void handleReadyForMessage(const boost::system::error_code &error,
                            std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        boost::asio::async_read(socket_,
                                inbuf,
                                boost::asio::transfer_all(),
                                boost::bind(&TcpClient::handleMessage,
                                            shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void handleMessage(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                  << "got message size " << bytes_transferred  << std::endl;

        std::string messageString = {buffers_begin(inbuf.data()), buffers_end(inbuf.data())};

        std::cout << messageString << std::endl;

        //writing file
        std::ofstream messageContents(std::to_string(messageId_)+".msg");
        messageContents << messageString;
    }

    void handleWrite(const boost::system::error_code &error, size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " written bytes " << bytes_transferred << std::endl;
        if (error)
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " ERROR: " << error.message()
                      << std::endl;
        boost::asio::async_read_until(socket_,
                                      inbuf,
                                      "\n",
                                      boost::bind(&TcpClient::handleConfirm,
                                                  this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));

//        start();
    }

    void handleConfirm(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        std::string confirm;
        std::getline(is, confirm);

        if (error)
            std::cerr << __FUNCTION__ << " error " << error.message() << " bytes_transferred " << bytes_transferred
                      << std::endl;

        std::cout << "handleConfirm got: " << confirm << std::endl;

        if (confirm == "Ready for file") {
            std::cout << "handleConfirm Ready for file" << std::endl;
            sendFile();
        }
    }

    void sendFile() {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " sending" << std::endl;

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(messageBuffer_, messageBufferSize_),
                                 boost::asio::transfer_all(),
                                 boost::bind(&TcpClient::handleMessageSent,
                                             this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

    }

    void handleMessageSent(const boost::system::error_code &error,
                           std::size_t bytes_transferred) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " sent!" << std::endl;
        delete[] messageBuffer_;
        socket_.close();
    };

    void handleConnect(const boost::system::error_code &error) {

        if (error) {
            std::cerr << "handleConnect error " << error.message() << std::endl;
//            if (error == boost::asio::error::operation_aborted) return;
            return;
        }

        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " connected" << std::endl;

    }


    size_t messageId_;
    boost::asio::streambuf inbuf;
    std::istream is{&inbuf};

    tcp::socket socket_;
    boost::asio::io_context *io_context_;
};


#endif //MESSAGINGCLIENT_TCPCLIENT_H
