//
// Created by ak on 18/09/2019.
//

#ifndef MESSAGINGCLIENT_TCPCLIENT_H
#define MESSAGINGCLIENT_TCPCLIENT_H


#include <ctime>
#include <set>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "./json.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class TcpConnection {
public:
    TcpConnection(boost::asio::io_context &io_context,
                  boost::asio::ssl::context& ssl_context);

    ~TcpConnection();

    bool connect(tcp::resolver::results_type &tcpEndpoint);

    void sendMessage(std::string &uid,
                     std::set<std::string> const &destinations,
                     char *messageBuffer, size_t messageBufferSize);

private:
    char *messageBuffer_ = nullptr;
    size_t messageBufferSize_ = 0;

    void sendEndOfLine(const boost::system::error_code &error, size_t bytes_transferred);

    void handleWrite(const boost::system::error_code &error, size_t bytes_transferred);

    void handleConfirm(const boost::system::error_code &error,
                       std::size_t bytes_transferred);

    void sendFile();

    void handleMessageSent(const boost::system::error_code &error,
                           std::size_t bytes_transferred);

    bool verifyCertificate(bool &preverified, boost::asio::ssl::verify_context& ctx);

    boost::asio::streambuf inbuf;
    std::istream is{&inbuf};

    boost::asio::ssl::stream<tcp::socket> socket_;
    boost::asio::io_context *io_context_;
};


#endif //MESSAGINGCLIENT_TCPCLIENT_H
