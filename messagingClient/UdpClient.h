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
#include <boost/asio/ssl.hpp>

#include "json.hpp"
#include "TcpConnection.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class MessageOperations;

class UdpClient {
public:
    UdpClient(boost::asio::io_context &io_context,
              boost::asio::ssl::context &ssl_context,
              udp::endpoint &remote_endpoint,
              tcp::resolver::results_type &tcpEndpoint, std::string uid, MessageOperations &messageOperations);

    void ping();

    void startReceive();

private:
    void handleReceive(const boost::system::error_code &error, size_t bytes_transferred);

    void handleSend(std::string message,
                    const boost::system::error_code &error,
                    std::size_t bytes_transferred);

    std::string uid_;
    int i = 0;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 128> recv_buffer_{};
    boost::asio::io_context &io_context_;
    boost::asio::ssl::context &ssl_context_;
    tcp::resolver::results_type &tcpEndpoint_;
    MessageOperations &messageOperations_;

};


#endif //MESSAGINGCLIENT_UDPCLIENT_H
