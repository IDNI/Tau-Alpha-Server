//
// Created by ak on 18/09/2019.
//

#ifndef MESSAGINGCLIENT_UDPCLIENT_H
#define MESSAGINGCLIENT_UDPCLIENT_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "./json.hpp"
#include "./TcpClient.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class UdpClient {
public:
    UdpClient(boost::asio::io_context &io_context, udp::endpoint &remote_endpoint,
              tcp::resolver::results_type &tcpEndpoint)
            : socket_(io_context, udp::endpoint(udp::v4(), 0)),
              remote_endpoint_(remote_endpoint),
              io_context_(&io_context),
              tcpEndpoint_(tcpEndpoint) {
        std::cout << __FILE__ << " io_context::stopped: " << io_context_ << " " << io_context_->stopped() << std::endl;

    }

    void ping(std::string uid) {
        json msg;
        msg["action"] = "ping";
        msg["uid"] = uid;

        boost::shared_ptr<std::string> message(
                new std::string(msg.dump()));
        // std::cout << __FILE__ << " " << __FUNCTION__ << " io_context::stopped: " << io_context_ << " "
//                  << io_context_->stopped() << std::endl;

//        std::cout << "Sending ping to " << remote_endpoint_.address() <<
//                  ":" << remote_endpoint_.port() << std::endl;

        //we send our name along with the ping
        socket_.async_send_to(boost::asio::buffer(*message),
                              remote_endpoint_,
                              boost::bind(&UdpClient::handleSend,
                                          this,
                                          "ping",
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
        sleep(5);
        ping(uid);
    }

    void send(std::string messageString) {

        boost::shared_ptr<std::string> message(new std::string(messageString));
        std::cout << "Sending " << messageString << std::endl;

        socket_.async_send_to(boost::asio::buffer(*message),
                              remote_endpoint_,
                              boost::bind(&UdpClient::handleSend,
                                          this,
                                          messageString,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
        // std::cout << __FILE__ << " " << __FUNCTION__ << "io_context::stopped: "
//                  << io_context_ << " " << io_context_->stopped()
//                  << std::endl;
        startReceive();
    }

    void startReceive() {
        std::cout << "startReceive" << std::endl;
        socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                boost::bind(&UdpClient::handleReceive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }

private:
    void handleReceive(const boost::system::error_code &error, size_t bytes_transferred) {
        std::cout << std::endl << "got message: "
                  << std::string(recv_buffer_.begin(), bytes_transferred) << std::endl
                  << std::endl;

        if (!error) {
            auto message = std::string(recv_buffer_.begin(), bytes_transferred);
            // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
//                      " " << message << std::endl;


            if (message != "pong") {

                json actionData;

                try {
                    actionData = json::parse(message);
                } catch (std::exception &e) {
                    std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                              << std::dec << __LINE__ << " : " << e.what() << std::endl;
                    return;
                }

                if ("Got message" == actionData["action"].get<std::string>()) {
                    //there's a message waiting for us on the server
                    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__;

                    TcpClient::pointer newConnection =
                            TcpClient::create(*io_context_);

                    try {
                        newConnection->connect(tcpEndpoint_);
                    } catch (boost::system::system_error &e) {
                        if (e.code() == boost::asio::error::not_found)
                            std::cerr << "connect returned not_found" << std::endl;
                        else
                            std::cerr << "connect returned error " << e.code() << std::endl;
                    }

                    auto messageId = actionData["messageId"].get<size_t>();

                    newConnection->getMessage(messageId);
                }
            }


//            processServerMessage();
//            send("Hello again");
//            boost::shared_ptr<std::string> message(
//                    new std::string("hello\n"));
//
//            socket_.async_send_to(boost::asio::buffer(*message),
//                                  remote_endpoint_,
//                                  boost::bind(&UdpClient::handleSend,
//                                              this, message));
//
            startReceive();
        }
    }

    void handleSend(std::string message,
                    const boost::system::error_code &error,
                    std::size_t bytes_transferred) {
        i++;
        std::cout << "async_send_to " << message << " return " << error << ": " <<
                  bytes_transferred << " transmitted " << i << std::endl;//        startReceive();
    }

    int i = 0;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 128> recv_buffer_{};
    boost::asio::io_context *io_context_;
    tcp::resolver::results_type &tcpEndpoint_;
};


#endif //MESSAGINGCLIENT_UDPCLIENT_H
