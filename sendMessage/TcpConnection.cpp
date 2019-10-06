//
// Created by ak on 18/09/2019.
//

#include "TcpConnection.h"

TcpConnection::TcpConnection(boost::asio::io_context &io_context,
        boost::asio::ssl::context &ssl_context)
        : io_context_(&io_context),
          socket_(io_context, ssl_context) {
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
    // std::cout << __FILE__ << " " << __FUNCTION__ << " connecting to "
//                  << tcpEndpoint->host_name()
//                  << ":"
//                  << tcpEndpoint->service_name()
//                  << std::endl;
    // std::cout << __FILE__ << " " << __FUNCTION__ << " io_context::stopped: " << io_context_ << " "
//                  << io_context_->stopped() << std::endl;

    boost::asio::connect(
            socket_.lowest_layer(),
            tcpEndpoint);

    socket_.handshake(boost::asio::ssl::stream_base::client);
//                boost::bind(&TcpConnection::handleConnect,
//                            this,
//                            boost::asio::placeholders::error));
    return true;
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
            new std::string(msg.dump() + "\n"));

    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " writing " << std::string(msg.dump()) << std::endl;

    boost::asio::async_write(socket_,
                             boost::asio::buffer(*message),
                             boost::bind(&TcpConnection::handleWrite,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

}

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

#ifdef DEBUG_COUT
    std::cout << "handleConfirm got: " << confirm << std::endl;
#endif

    if (confirm == "Ready for file") {
#ifdef DEBUG_COUT
        std::cout << "handleConfirm Ready for file" << std::endl;
#endif
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
    socket_.shutdown();
};

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
