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

#include "json.hpp"
#include "MessageOperations.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class TcpConnection
        : public boost::enable_shared_from_this<TcpConnection> {
public:
    ~TcpConnection();
    typedef boost::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context &io_context, std::string uid, MessageOperations &MessageOperations);

    TcpConnection(boost::asio::io_context &io_context, std::string uid, MessageOperations &messageOperations);

    bool connect(tcp::resolver::results_type &tcpEndpoint);

    bool getMessages();

private:
    bool requestUnreadMessages(boost::system::error_code &ec);
    std::tuple<json,size_t,size_t> readHeader(boost::system::error_code &ec);
    size_t readMessage(boost::system::error_code &ec, size_t messageSize);

    boost::asio::streambuf inbuf;
    std::istream inputStream{&inbuf};

    std::string uid_;
    tcp::socket socket_;
    boost::asio::io_context *io_context_;
    MessageOperations &messageOperations_;
};


#endif //MESSAGINGCLIENT_TCPCONNECTION_H
