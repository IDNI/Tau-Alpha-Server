//
// Created by ak on 18/09/2019.
//

#include "TcpConnection.h"

TcpConnection::TcpConnection(boost::asio::io_context &io_context)
        : io_context_(&io_context),
          socket_(io_context) {

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
//                boost::bind(&TcpConnection::handleConnect,
//                            this,
//                            boost::asio::placeholders::error));
    return true;
}

boost::shared_ptr<TcpConnection> TcpConnection::create(boost::asio::io_context &io_context) {
    return pointer(new TcpConnection(io_context));
}

void TcpConnection::sendMessage(std::string &uid,
                                std::set<std::string> const &destinations,
                                char *messageBuffer, size_t messageBufferSize) {
    json msg;
    msg["sender"] = uid;
    msg["action"] = "Send message";
    msg["destination"] = destinations;

    messageBuffer_ = messageBuffer;
    messageBufferSize_ = messageBufferSize;
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                  << "sending " << msg.dump() << std::endl;

    boost::shared_ptr<std::string> message(
            new std::string(msg.dump()));

    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " writing " << std::string(msg.dump()) << std::endl;

    boost::asio::async_write(socket_,
                             boost::asio::buffer(*message),
                             boost::bind(&TcpConnection::sendEndOfLine,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

}

void TcpConnection::sendEndOfLine(const boost::system::error_code &error, size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " writing" << std::endl;
    boost::asio::async_write(socket_,
                             boost::asio::buffer("\n"),
                             boost::asio::transfer_all(),
                             boost::bind(&TcpConnection::handleWrite,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

};

void TcpConnection::handleWrite(const boost::system::error_code &error, size_t bytes_transferred) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " written bytes " << bytes_transferred<< std::endl;
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
