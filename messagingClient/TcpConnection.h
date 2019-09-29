//
// Created by ak on 18/09/2019.
//

#ifndef MESSAGINGCLIENT_TCPCONNECTION_H
#define MESSAGINGCLIENT_TCPCONNECTION_H


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

class TcpConnection
        : public boost::enable_shared_from_this<TcpConnection> {
public:
    typedef boost::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context &io_context, std::string uid);

    TcpConnection(boost::asio::io_context &io_context, std::string uid);

    bool connect(tcp::resolver::results_type &tcpEndpoint);

    bool getMessages();

private:
    char *messageBuffer_ = nullptr;
    size_t messageBufferSize_ = 0;

    void sendEndOfLine(const boost::system::error_code &error, size_t bytes_transferred);

    void handleReadyForMessage(const boost::system::error_code &error, std::size_t bytes_transferred);

    void handleMessage(const boost::system::error_code &error, std::size_t bytes_transferred);

    void handleWrite(const boost::system::error_code &error, size_t bytes_transferred);

    void handleConfirm(const boost::system::error_code &error, std::size_t bytes_transferred);

    void sendFile();

    void handleMessageSent(const boost::system::error_code &error, std::size_t bytes_transferred);

    void handleConnect(const boost::system::error_code &error);

    size_t messageId_;
    boost::asio::streambuf inbuf;
    std::istream is{&inbuf};

    std::string uid_;
    tcp::socket socket_;
    boost::asio::io_context *io_context_;
};


#endif //MESSAGINGCLIENT_TCPCONNECTION_H
