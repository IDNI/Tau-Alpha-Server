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

#include "TcpConnection.h"

TcpConnection::TcpConnection(tcp::socket socket,
        boost::asio::ssl::context &ssl_context,
                             MessageOperations &messageOperations)
        : socket_(std::move(socket), ssl_context),
          messageOperations_(messageOperations) {
}


void TcpConnection::start() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " handshaking SSL" << std::endl;
#endif

    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    auto self(shared_from_this());
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    socket_.async_handshake(boost::asio::ssl::stream_base::server,
                            [this, self](const boost::system::error_code &error) {
                                if (!error) {
                                    doRead();
                                } else {
                                    std::cerr << "SSL ha    ndshake error " << error << std::endl;
                                }

                            });
}

void TcpConnection::doRead() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " waiting for command" << std::endl;
#endif
    //reading command
    //it can be "send message" or "get messages"
    boost::asio::async_read_until(socket_,
                                  inbuf,
                                  "\n",
                                  boost::bind(&TcpConnection::handleReadAction,
                                              shared_from_this(),
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
}

boost::system::error_code error;

void TcpConnection::executeAction() {
    if ("Send message" == actionData["action"].get<std::string>()) {
        runSendMessage();
    } else if ("Get messages" == actionData["action"].get<std::string>()) {
        runActionGetMessages(actionData["recipientName"].get<std::string>());
    }
}

size_t TcpConnection::sendMessageHeader(std::string messagePath) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif

    auto[messageHeaderBuffer, messageHeaderBufferFileSize] = messageOperations_.getMessageHeader(messagePath);
    if (messageHeaderBuffer == nullptr) return 0;

#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << "\n"
                 << "writing header size " << messageHeaderBufferFileSize
                 << std::endl;
#endif
    boost::system::error_code ec;
    size_t bytes_transferred = boost::asio::write(socket_,
                                                  boost::asio::buffer(messageHeaderBuffer,
                                                                      messageHeaderBufferFileSize), ec);
    delete[] messageHeaderBuffer;
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                 << " write message header bytes_transferred " << bytes_transferred << std::endl;
#endif

    if (ec && ec != boost::asio::error::eof) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return 0;
    }

    return bytes_transferred;
}

size_t TcpConnection::sendMessageBody(std::string messagePath) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif

    auto[messageBuffer, messageBufferFileSize] = messageOperations_.getMessage(messagePath);
    if (messageBuffer == nullptr) return 0;

#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << "\n"
                 << "writing message size " << messageBufferFileSize
                 << std::endl;
#endif

    boost::system::error_code ec;
    size_t bytes_transferred = boost::asio::write(socket_,
                                                  boost::asio::buffer(messageBuffer, messageBufferFileSize), ec);
    delete[] messageBuffer;

    if (ec && ec != boost::asio::error::eof) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return 0;
    }

#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                 << " write message bytes_transferred " << bytes_transferred << std::endl;
#endif

    return bytes_transferred;
}


void TcpConnection::runActionGetMessages(std::string recipientName) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif

    std::set<std::string> &messages = messageOperations_.getUnsentMessagesFor(recipientName);

    size_t bytes_transferred = 0;

    for (auto messagePath = messages.begin(); messagePath != messages.end();) {
#ifdef DEBUG_COUT
        std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                         << "\ngot message for " << recipientName << ": " << *messagePath
                         << std::endl;
#endif

        if (!sendMessageHeader(*messagePath)) {
            ++messagePath;
            continue;
        }

        if (!sendMessageBody(*messagePath)) {
            ++messagePath;
            continue;
        }

        messagePath = messageOperations_.sent(recipientName, messagePath);
    }


#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " END sending" << std::endl;
#endif
    socket_.shutdown();
}

void TcpConnection::runSendMessage() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif

    boost::asio::async_write(socket_,
                             boost::asio::buffer("Ready for file\n"),
                             boost::asio::transfer_all(),
                             boost::bind(&TcpConnection::handleReadyForMessage,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::handleReadAction(const boost::system::error_code &error,
                                     std::size_t bytes_transferred) {
    // #ifdef DEBUG_COUT std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl; #endif
    processAction(error, bytes_transferred);

    if (actionData.size() == 0) return;

    // #ifdef DEBUG_COUT std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl; #endif
    executeAction();
}

void TcpConnection::handleReadyForMessage(const boost::system::error_code &error,
                                          std::size_t bytes_transferred) {
    // #ifdef DEBUG_COUT std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl; #endif
    boost::asio::async_read(socket_,
                            inbuf,
                            boost::asio::transfer_all(),
                            boost::bind(&TcpConnection::handleMessage,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::handleMessage(const boost::system::error_code &error,
                                  std::size_t bytes_transferred) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << "got message size " << bytes_transferred
                 << std::endl;
#endif

    //save message
    messageOperations_.save(actionData, inputStream, bytes_transferred);
    inbuf.consume(bytes_transferred);

    //notify recipients of a new message and save to unsent for them
    messageOperations_.notifyRecipients(actionData);
}

void TcpConnection::parseIncomingMessage(std::string &incomingMessage) {
#ifdef DEBUG_COUT
    std::cout << "got message: "
                 << incomingMessage << std::endl;
#endif

    try {
        actionData = json::parse(incomingMessage);
    } catch (std::exception &e) {
        std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                  << std::dec << __LINE__ << " : " << e.what() << std::endl;
        return;
    }


    if (actionData.contains("action")) {
//            std::cout << "parsed message: "
//                      << actionData["action"].get<std::string>()
//                      << " size " << actionData.size()
//                      << std::endl;
    } else {
        actionData.clear();
    }
}

void TcpConnection::processAction(const boost::system::error_code &error,
                                  std::size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    std::string incomingMessage;
    std::getline(inputStream, incomingMessage);

    if (error)
        std::cerr << __FUNCTION__ << " error " << error.message() << " bytes_transferred " << bytes_transferred
                  << std::endl;

//        std::cout << "handleRead got: " << incomingMessage << std::endl;

    parseIncomingMessage(incomingMessage);

    return;
}

TcpConnection::~TcpConnection() {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " : " << __FUNCTION__ << " : "
                 << std::dec << __LINE__ << " : " << "TcpConnection destroyed!" << std::endl;
#endif
}