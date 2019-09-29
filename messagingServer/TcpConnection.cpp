//
// Created by ak on 17/09/2019.
//

#include "TcpConnection.h"

boost::shared_ptr<TcpConnection>
TcpConnection::create(boost::asio::io_context &io_context, MessageOperations &messageOperations) {
    return pointer(new TcpConnection(io_context, messageOperations));
}

tcp::socket &TcpConnection::socket() {
    return socket_;
}

void TcpConnection::start() {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " waiting for command" << std::endl;
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

TcpConnection::TcpConnection(boost::asio::io_context &io_context, MessageOperations &messageOperations)
        : socket_(io_context),
          messageOperations_(messageOperations) {
}


boost::system::error_code error;

void TcpConnection::executeAction() {
    if ("Send message" == actionData["action"].get<std::string>()) {
        runSendMessage();
    } else if ("Get messages" == actionData["action"].get<std::string>()) {
        runGetMessages(actionData["recipientName"].get<std::string>());
    }
}

void TcpConnection::runGetMessages(std::string recipientName) {
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    std::set<std::string> &messages = messageOperations_.getUnsentMessagesFor(recipientName);

    for (auto messagePath : messages) {
        auto[messageBuffer, fileSize] = messageOperations_.getMessage(messagePath);

        if (messageBuffer == nullptr) break;
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        size_t bytes_transferred = 0;

        try {
            bytes_transferred = boost::asio::write(socket_, boost::asio::buffer(messageBuffer, fileSize));
        } catch (std::exception &e) {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                      << " write message error " << e.what() << std::endl;
        }

        if (bytes_transferred) {
            messageOperations_.sent(recipientName, messagePath);
            delete[] messageBuffer;
        }
    }
    socket_.close();
}

void TcpConnection::runSendMessage() {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

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
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    processAction(error, bytes_transferred);

    if (actionData.size() == 0) return;

    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    executeAction();
}

void TcpConnection::handleReadyForMessage(const boost::system::error_code &error,
                                          std::size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
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
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << "got message size " << bytes_transferred << std::endl;

    std::string messageString = {buffers_begin(inbuf.data()), buffers_end(inbuf.data())};

    //save message
    messageOperations_.save(actionData, messageString);

    //notify recipients of a new message and save to unsent for them
    messageOperations_.notifyRecipients(actionData);
}

void TcpConnection::parseIncomingMessage(std::string &incomingMessage) {
    std::cout << "got message: "
              << incomingMessage << std::endl;

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
