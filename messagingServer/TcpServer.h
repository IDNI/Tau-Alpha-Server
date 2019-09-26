//
// Created by ak on 17/09/2019.
//

#ifndef MESSAGINGSERVER_TCPSERVER_H
#define MESSAGINGSERVER_TCPSERVER_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "./TcpConnection.h"
#include "./Routing.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class TcpServer {
public:
    TcpServer(boost::asio::io_context &io_context, UdpServer &udpServer, Routing &routing,
              std::unordered_map<size_t, Message *> &messages,
              uint16_t portNumber)
            : io_context_(io_context),
              udpServer_(udpServer),
              routing_(routing),
              messages_(messages),
              acceptor_(io_context, tcp::endpoint(tcp::v4(), portNumber)) {
        startAccept();
    }

private:
    void startAccept() {
//        std::cout << "startAccept" << std::endl;
        TcpConnection::pointer newConnection =
                TcpConnection::create(io_context_, udpServer_, routing_, messages_);

        acceptor_.async_accept(newConnection->socket(),
                               boost::bind(&TcpServer::handleAccept,
                                           this,
                                           newConnection,
                                           boost::asio::placeholders::error));
    }

    void handleAccept(TcpConnection::pointer newConnection,
                      const boost::system::error_code &error) {
        if (error) {
            std::cerr << "handleAccept error " << error << std::endl;
        } else {
            std::cerr << "handleAccept accepted " << std::endl;
            newConnection->start();
        }

        startAccept();
    }

    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    UdpServer &udpServer_;
    Routing &routing_;
    std::unordered_map<size_t, Message *> messages_;
};


#endif //MESSAGINGSERVER_TCPSERVER_H
