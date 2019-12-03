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

#include "json.hpp"
#include "Routing.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class UdpServer {
public:
    UdpServer(boost::asio::io_context &io_context, Routing &routing, uint16_t portNumber);

    void send(std::string messageString, udp::endpoint &remote_endpoint);

private:
    void startReceive();

    void makeActionBasedOnMessage(json &receivedMessage, udp::endpoint &remote_endpoint);

    void processIncomingMessage(std::string incomingMessage, udp::endpoint remote_endpoint);

    json parseIncomingMessage(std::string &incomingMessage);

    void handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred);

    void handleSend(boost::shared_ptr<std::string> message);

    Routing &routing_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 128> recv_buffer_{};
};

#endif //MESSAGINGSERVER_UDPSERVER_H
