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

#include "UdpClient.h"

UdpClient::UdpClient(boost::asio::io_context &io_context,
                     boost::asio::ssl::context &ssl_context,
                     udp::endpoint &remote_endpoint,
                     tcp::resolver::results_type &tcpEndpoint,
                     std::string uid,
                     MessageOperations &messageOperations)
        : socket_(io_context, udp::endpoint(udp::v4(), 0)),
          remote_endpoint_(remote_endpoint),
          io_context_(io_context),
          ssl_context_(ssl_context),
          tcpEndpoint_(tcpEndpoint),
          uid_(uid),
          messageOperations_(messageOperations) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " io_context::stopped: " << io_context_ << " " << io_context_->stopped() << std::endl;
#endif

}

void UdpClient::ping() {
    json msg;
    msg["action"] = "ping";
    msg["uid"] = uid_;

    boost::shared_ptr<std::string> message(
            new std::string(msg.dump()));
    // std::cout << __FILE__ << " " << __FUNCTION__ << " io_context::stopped: " << io_context_ << " "
//                  << io_context_->stopped() << std::endl;

//        std::cout << "Sending ping to " << remote_endpoint_.address() <<
//                  ":" << remote_endpoint_.port() << std::endl;

    //we send our name along with the ping
    socket_.async_send_to(boost::asio::buffer(*message),
                          remote_endpoint_,
                          boost::bind(&UdpClient::handleSend,
                                      this,
                                      "ping",
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
    sleep(5);
    ping();
}

void UdpClient::startReceive() {
#ifdef DEBUG_COUT
    std::cout << "startReceive" << std::endl;
#endif
    socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            boost::bind(&UdpClient::handleReceive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void UdpClient::handleReceive(const boost::system::error_code &error, size_t bytes_transferred) {
//    std::cout << std::endl << "Got message: "
//                  << std::string(recv_buffer_.begin(), bytes_transferred) << std::endl;

    if (!error) {
        auto message = std::string(recv_buffer_.begin(), bytes_transferred);
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
//                      " " << message << std::endl;


        if (message != "pong") {

            json actionData;

            try {
                actionData = json::parse(message);
            } catch (std::exception &e) {
                std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                          << std::dec << __LINE__ << " : " << e.what() << std::endl;
                return;
            }

            if ("Got messages" == actionData["action"].get<std::string>()) {
                //there's a message waiting for us on the server
                //so we must connect to it and go further
                auto tcpConnection =
                        std::make_shared<TcpConnection>
                                (io_context_,
                                 ssl_context_,
                                 uid_,
                                 messageOperations_);

                try {
                    tcpConnection->connect(tcpEndpoint_);
                } catch (boost::system::system_error &e) {
                    if (e.code() == boost::asio::error::not_found)
                        std::cerr << "connect returned not_found" << std::endl;
                    else
                        std::cerr << "connect returned error " << e.code() << std::endl;
                }

            }
        }


//            processServerMessage();
//            send("Hello again");
//            boost::shared_ptr<std::string> message(
//                    new std::string("hello\n"));
//
//            socket_.async_send_to(boost::asio::buffer(*message),
//                                  remote_endpoint_,
//                                  boost::bind(&UdpClient::handleSend,
//                                              this, message));
//
        startReceive();
    }
}

void UdpClient::handleSend(std::string message,
                           const boost::system::error_code &error,
                           std::size_t bytes_transferred) {
    i++;
#ifdef DEBUG_COUT
    std::cout << "async_send_to " << message << " return " << error << ": " <<
                  bytes_transferred << " transmitted " << i << std::endl;//        startReceive();
#endif
}