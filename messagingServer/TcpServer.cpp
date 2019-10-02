//
// Created by ak on 17/09/2019.
//

#include "TcpServer.h"

TcpServer::TcpServer(boost::asio::io_context &io_context,
                     MessageOperations &messageOperations,
                     uint16_t portNumber

):
io_context_ (io_context),
messageOperations_(messageOperations),
acceptor_(io_context, tcp::endpoint(tcp::v4(), portNumber)) {

    std::cout << "TCP server listening on port " << portNumber << std::endl;
    startAccept();
}

void TcpServer::startAccept() {
//        std::cout << "startAccept" << std::endl;
    TcpConnection::pointer newConnection =
            TcpConnection::create(io_context_, messageOperations_);

    acceptor_.async_accept(newConnection->socket(),
                           boost::bind(&TcpServer::handleAccept,
                                       this,
                                       newConnection,
                                       boost::asio::placeholders::error));
}

void TcpServer::handleAccept(TcpConnection::pointer newConnection,
                  const boost::system::error_code &error) {
    if (error) {
        std::cerr << "handleAccept error " << error << std::endl;
    } else {
#ifdef DEBUG_COUT
        std::cout << "handleAccept accepted " << std::endl;
#endif
        newConnection->start();
    }

    startAccept();
}
