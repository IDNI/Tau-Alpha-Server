//
// Created by ak on 18/09/2019.
//

#include "TcpConnection.h"

boost::shared_ptr<TcpConnection> TcpConnection::create(boost::asio::io_context &io_context, std::string uid) {
    return boost::shared_ptr<TcpConnection>(new TcpConnection(io_context, uid));
}

TcpConnection::TcpConnection(boost::asio::io_context &io_context, std::string uid)
: io_context_(&io_context),
socket_(io_context),
uid_(uid) {

}

bool TcpConnection::connect(tcp::resolver::results_type &tcpEndpoint) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " connecting to "
//                  << tcpEndpoint->host_name()
//                  << ":"
//                  << tcpEndpoint->service_name()
//                  << std::endl;
    // std::cout << __FILE__ << " " << __FUNCTION__ << " io_context::stopped: " << io_context_ << " "
//                  << io_context_->stopped() << std::endl;

    boost::asio::connect(
            socket_,
            tcpEndpoint);
    return true;
}

bool TcpConnection::getMessages() {
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
              json msg;
    msg["action"] = "Get messages";
    msg["recipientName"] = uid_;

    boost::shared_ptr<std::string> message(
            new std::string(msg.dump()));

    boost::asio::async_write(socket_,
                             boost::asio::buffer(*message),
                             boost::asio::transfer_all(),
                             boost::bind(&TcpConnection::sendEndOfLine,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

    return true;
}

void TcpConnection::sendEndOfLine(const boost::system::error_code &error, size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " writing" << std::endl;
    boost::asio::async_write(socket_,
                             boost::asio::buffer("\n"),
                             boost::asio::transfer_all(),
                             boost::bind(&TcpConnection::handleReadyForMessage,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

};

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
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                  << "got message size " << bytes_transferred  << std::endl;

    std::string messageString = {buffers_begin(inbuf.data()), buffers_end(inbuf.data())};

    std::cout << messageString << std::endl;

    //writing file
    std::ofstream messageContents(std::to_string(messageId_)+".msg");
    messageContents << messageString;
}

void TcpConnection::handleWrite(const boost::system::error_code &error, size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " written bytes " << bytes_transferred << std::endl;
    if (error)
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " ERROR: " << error.message()
                  << std::endl;
    boost::asio::async_read_until(socket_,
                                  inbuf,
                                  "\n",
                                  boost::bind(&TcpConnection::handleConfirm,
                                              this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));

//        start();
}

void TcpConnection::handleConfirm(const boost::system::error_code &error,
                   std::size_t bytes_transferred) {
    std::string confirm;
    std::getline(is, confirm);

    if (error)
        std::cerr << __FUNCTION__ << " error " << error.message() << " bytes_transferred " << bytes_transferred
                  << std::endl;

    std::cout << "handleConfirm got: " << confirm << std::endl;

    if (confirm == "Ready for file") {
        std::cout << "handleConfirm Ready for file" << std::endl;
        sendFile();
    }
}

void TcpConnection::sendFile() {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " sending" << std::endl;

    boost::asio::async_write(socket_,
                             boost::asio::buffer(messageBuffer_, messageBufferSize_),
                             boost::asio::transfer_all(),
                             boost::bind(&TcpConnection::handleMessageSent,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

}

void TcpConnection::handleMessageSent(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " sent!" << std::endl;
    delete[] messageBuffer_;
    socket_.close();
};

void TcpConnection::handleConnect(const boost::system::error_code &error) {

    if (error) {
        std::cerr << "handleConnect error " << error.message() << std::endl;
//            if (error == boost::asio::error::operation_aborted) return;
        return;
    }

    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " connected" << std::endl;

}
