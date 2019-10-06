//
// Created by ak on 17/09/2019.
//

#ifndef MESSAGINGSERVER_TCPCONNECTION_H
#define MESSAGINGSERVER_TCPCONNECTION_H

#include <set>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/scoped_array.hpp>
#include <boost/asio/ssl.hpp>

#include "json.hpp"
#include "UdpServer.h"
#include "MessageOperations.h"

using json = nlohmann::json;

using boost::asio::ip::tcp;

class TcpConnection
        : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(tcp::socket socket,
            boost::asio::ssl::context &ssl_context,
                  MessageOperations &messageOperations);

    ~TcpConnection();

    void start();

private:

    void doRead();

    void executeAction();

    size_t sendMessageHeader(std::string messagePath);

    size_t sendMessageBody(std::string messagePath);

    void runActionGetMessages(std::string recipientName);

    void runSendMessage();

    void handleReadAction(const boost::system::error_code &error,
                          std::size_t bytes_transferred);

    void handleReadyForMessage(const boost::system::error_code &error,
                               std::size_t bytes_transferred);

    void handleMessage(const boost::system::error_code &error,
                       std::size_t bytes_transferred);

    void parseIncomingMessage(std::string &incomingMessage);

    void processAction(const boost::system::error_code &error,
                       std::size_t bytes_transferred);

    json actionData;

    boost::system::error_code error;
    boost::asio::ssl::stream<tcp::socket> socket_;
    boost::asio::streambuf inbuf;
    std::istream inputStream{&inbuf};
    MessageOperations &messageOperations_;
};


#endif //MESSAGINGSERVER_TCPCONNECTION_H
