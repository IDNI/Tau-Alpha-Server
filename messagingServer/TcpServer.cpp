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

#include "TcpServer.h"

TcpServer::TcpServer(boost::asio::io_context &io_context,
                     MessageOperations &messageOperations,
                     uint16_t portNumber

) :
        io_context_(io_context),
        messageOperations_(messageOperations),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), portNumber)),
        context_(boost::asio::ssl::context::sslv23) {

    context_.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(&TcpServer::getPassword, this));
    context_.use_certificate_chain_file("server.pem");
    context_.use_private_key_file("server.pem", boost::asio::ssl::context::pem);
    context_.use_tmp_dh_file("dh2048.pem");

    std::cout << "TCP server listening on port " << portNumber << std::endl;
    startAccept();
}

std::string TcpServer::getPassword() const {
    return "test";
}

void TcpServer::startAccept() {
//        std::cout << "startAccept" << std::endl;
    acceptor_.async_accept([this](const boost::system::error_code &error, tcp::socket socket) {
        if (!error) {
            auto tcpConnection = std::make_shared<TcpConnection>(
                    std::move(socket),
                    context_,
                    messageOperations_);

            tcpConnection->start();
        }

        startAccept();
    });
}
