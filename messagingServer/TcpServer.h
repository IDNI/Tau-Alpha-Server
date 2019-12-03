// LICENSE
// This software is free for use and redistribution while including this
// license notice, unless:
// 1. is used for commercial or non-personal purposes, or
// 2. used for a product which includes or associated with a blockchain or other
// decentralized database technology, or
// 3. used for a product which includes or associated with the issuance or use
// of cryptographic or electronic currencies/coins/tokens.
// On all of the mentioned cases, an explicit and written permission is required
// from the Author (Ohad Asor).
// Contact ohad@idni.org for requesting a permission. This license may be
// modified over time by the Author.
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
