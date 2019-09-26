//
// Created by ak on 17/09/2019.
//

#ifndef MESSAGINGSERVER_UDPSERVER_H
#define MESSAGINGSERVER_UDPSERVER_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <map>

#include "./json.hpp"
#include "./Routing.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class UdpServer {
public:
    UdpServer(boost::asio::io_context &io_context, Routing &routing, uint16_t portNumber)
            : socket_(io_context, udp::endpoint(udp::v4(), portNumber)),
              routing_(routing) {
        startReceive();
    }

    void send(std::string messageString, udp::endpoint &remote_endpoint) {

        boost::shared_ptr<std::string> message(
                new std::string(messageString));

//        std::cout << "Sending: "
//                  << *message << ": " << message
//                  << " to "
//                  << remote_endpoint.address() << ":" << remote_endpoint.port()
//                  << std::endl;
        socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint,
                              boost::bind(&UdpServer::handleSend, this, message));

    }

private:
    void startReceive() {
//        std::cout << "startReceive" << std::endl;

        socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                boost::bind(&UdpServer::handleReceive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }


    void makeActionBasedOnMessage(json &receivedMessage, udp::endpoint &remote_endpoint) {
//        std::cout << __FILE__ << " : " << __FUNCTION__ << " : "
//                  << std::dec << __LINE__ << std::endl;

        if ("ping" == receivedMessage["action"].get<std::string>()) {
//            std::cout << "processing PING from "
//                      << remote_endpoint.address()
//                      << ":"
//                      << remote_endpoint.port()
//                      << std::endl;

            //we need to check his authorization
            //checkUserAuthorization();
            //we need to find that user in our map
            std::string uid = receivedMessage["uid"].get<std::string>();

            routing_.saveRouteFromUdpEndpoint(remote_endpoint, uid);

            send("pong", remote_endpoint);
        }
    }

    void processIncomingMessage(std::string incomingMessage,
            //we need to copy remote_endpoint object
            //to separate it from the async_read
                                udp::endpoint remote_endpoint) {
        json parsedMessage = parseIncomingMessage(incomingMessage);

//        std::cout << "parsedMessage.size(): " << parsedMessage.size()
//                  << std::endl;

        if (parsedMessage.size()) makeActionBasedOnMessage(parsedMessage, remote_endpoint);
    }

    json parseIncomingMessage(std::string &incomingMessage) {
//        std::cout << "got message: "
//                  << incomingMessage << std::endl;

        json msg;

        try {
            msg = json::parse(incomingMessage);
        } catch (std::exception &e) {
            std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                      << std::dec << __LINE__ << " : " << e.what() << std::endl;
        }


        if (msg.contains("action")) {
//            std::cout << "parsed message: "
//                      << msg["action"].get<std::string>()
//                      << " size " << msg.size()
//                      << std::endl;
        } else {
            msg.clear();
        }

        return msg;
    }

    //    int counter =0;
    void handleReceive(const boost::system::error_code &error, std::size_t
    bytes_transferred) {
        if (!error) {
            //we pass the load we got from UDP packet as a string
            processIncomingMessage(std::string(recv_buffer_.begin(), bytes_transferred),
                                   remote_endpoint_);

//            boost::shared_ptr<std::string> message(
//                    new std::string(std::string(std::to_string(counter++)+"\n")));
//
//            std::cout << "Sending: "
//                      << *message << ": "<< message << std::endl;
//            socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
//                                  boost::bind(&UdpServer::handleSend, this, message));
//
            startReceive();
        }
    }

    void handleSend(boost::shared_ptr<std::string> message) {
//        std::cout << "Sent "
//                  << message << std::endl;
    }

    Routing &routing_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 128> recv_buffer_{};
};

#endif //MESSAGINGSERVER_UDPSERVER_H
