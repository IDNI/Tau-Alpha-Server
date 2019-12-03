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

#include "TcpConnection.h"

TcpConnection::TcpConnection(
        boost::asio::io_context &io_context,
        boost::asio::ssl::context &ssl_context,
        std::string uid, MessageOperations &messageOperations)
        : socket_(io_context, ssl_context),
          uid_(uid),
          messageOperations_(messageOperations) {

    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
            boost::bind(&TcpConnection::verifyCertificate, this, _1, _2));
}

TcpConnection::~TcpConnection() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " : " << __FUNCTION__ << " : "
                 << std::dec << __LINE__ << " : " << "TcpConnection destroyed!" << std::endl;
#endif
}

bool TcpConnection::connect(tcp::resolver::results_type &tcpEndpoint) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " connecting to "
                      << tcpEndpoint->host_name()
                      << ":"
                      << tcpEndpoint->service_name()
                      << std::endl;
#endif
    auto self(shared_from_this());

    boost::asio::async_connect(
            socket_.lowest_layer(),
            tcpEndpoint,
            [this, self](const boost::system::error_code &error,
                   const tcp::endpoint & /*endpoint*/) {
                if (!error) {
                    socket_.handshake(boost::asio::ssl::stream_base::client);
                    getMessages();
                } else {
                    std::cerr << "Connect failed: " << error.message() << "\n";
                }
            });


    return true;
}

bool TcpConnection::requestUnreadMessages(boost::system::error_code &ec) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    json msg;
    msg["action"] = "Get messages";
    msg["recipientName"] = uid_;

    boost::asio::write(socket_, boost::asio::buffer(msg.dump() + "\n"), ec);
    if (ec) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return false;
    }
    return true;
}

std::tuple<json, size_t, size_t> TcpConnection::readHeader(boost::system::error_code &ec) {
    json messageHeader;
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    size_t messageHeaderSize = boost::asio::read_until(socket_, inbuf, '\n', ec);

    if (ec) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
    }

#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                  << "got HEADER size " << messageHeaderSize << std::endl;
#endif

    if (messageHeaderSize == 0) {
#ifdef DEBUG_COUT
        std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
        return std::make_tuple(messageHeader, 0, 0);
    }

    std::string messageHeaderString(
            boost::asio::buffers_begin(inbuf.data()),
            boost::asio::buffers_begin(inbuf.data()) + messageHeaderSize);

#ifdef DEBUG_COUT
    std::cout << messageHeaderString << std::endl;
std::cout << "----------------HEADER-----------------------" << std::endl;
#endif

    try {
        messageHeader = json::parse(messageHeaderString);
    } catch (std::exception &e) {
        std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                  << std::dec << __LINE__ << " : " << e.what() << std::endl;
        return std::make_tuple(messageHeader, messageHeaderSize, 0);
    }
    //now we have the message size
    size_t messageSize = messageHeader["messageSize"].get<size_t>();

    inbuf.consume(messageHeaderSize);

    return std::make_tuple(messageHeader, messageHeaderSize, messageSize);
}

size_t TcpConnection::readMessage(boost::system::error_code &ec, size_t messageSize) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    size_t bytes_transferred = boost::asio::read(socket_, inbuf, boost::asio::transfer_exactly(messageSize), ec);
    if (ec && ec != boost::asio::error::eof) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
    }

#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                  << "got message size " << bytes_transferred << std::endl;
#endif

    return bytes_transferred;
}

bool TcpConnection::getMessages() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    boost::system::error_code ec;

    if (!requestUnreadMessages(ec)) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return false;
    }

    while (ec != boost::asio::error::eof) {
        auto[messageHeader, messageHeaderSize, messageSize] = readHeader(ec);

        if (!messageHeaderSize || !messageSize) {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                      " " << ec << std::endl;
            break;
        }

        readMessage(ec, messageSize);

#ifdef DEBUG_COUT
        std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                << " " << "total transferred " << messageSize
                << " + " << messageHeaderSize <<
                " = " << messageHeaderSize + messageSize << std::endl;
#endif

        messageOperations_.save(messageHeader, inputStream, messageSize);

    }

    return true;
}

bool TcpConnection::verifyCertificate(bool &preverified,
                                      boost::asio::ssl::verify_context &ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
}
