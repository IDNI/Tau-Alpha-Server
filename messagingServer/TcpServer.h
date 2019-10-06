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
#include <boost/asio/ssl.hpp>

#include "TcpConnection.h"
#include "Routing.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class TcpServer {
public:
    TcpServer(boost::asio::io_context &io_context,MessageOperations &messageOperations,
              uint16_t portNumber);

private:
    void startAccept();

    std::string getPassword() const;

    boost::asio::io_context &io_context_;
    boost::asio::ssl::context context_;
    tcp::acceptor acceptor_;
    MessageOperations &messageOperations_;
};


#endif //MESSAGINGSERVER_TCPSERVER_H
