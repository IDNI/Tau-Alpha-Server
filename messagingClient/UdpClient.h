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
#include "TcpConnection.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class UdpClient {
public:
    UdpClient(boost::asio::io_context &io_context, udp::endpoint &remote_endpoint,
              tcp::resolver::results_type &tcpEndpoint, std::string uid);

    void ping();

    void send(std::string messageString);

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
    boost::asio::io_context *io_context_;
    tcp::resolver::results_type &tcpEndpoint_;

};


#endif //MESSAGINGCLIENT_UDPCLIENT_H
