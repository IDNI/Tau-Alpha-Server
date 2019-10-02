//
// Created by ak on 18/09/2019.
//

#include "UdpClient.h"

UdpClient::UdpClient(boost::asio::io_context &io_context,
                     udp::endpoint &remote_endpoint,
                     tcp::resolver::results_type &tcpEndpoint,
                     std::string uid,
                     MessageOperations &messageOperations)
        : socket_(io_context, udp::endpoint(udp::v4(), 0)),
          remote_endpoint_(remote_endpoint),
          io_context_(&io_context),
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

void UdpClient::send(std::string messageString) {

    boost::shared_ptr<std::string> message(new std::string(messageString));
#ifdef DEBUG_COUT
    std::cout << "Sending " << messageString << std::endl;
#endif

    socket_.async_send_to(boost::asio::buffer(*message),
                          remote_endpoint_,
                          boost::bind(&UdpClient::handleSend,
                                      this,
                                      messageString,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
    // std::cout << __FILE__ << " " << __FUNCTION__ << "io_context::stopped: "
//                  << io_context_ << " " << io_context_->stopped()
//                  << std::endl;
    startReceive();
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
                // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__;

                TcpConnection::pointer newConnection =
                        TcpConnection::create(*io_context_, uid_, messageOperations_);

                try {
                    newConnection->connect(tcpEndpoint_);
                } catch (boost::system::system_error &e) {
                    if (e.code() == boost::asio::error::not_found)
                        std::cerr << "connect returned not_found" << std::endl;
                    else
                        std::cerr << "connect returned error " << e.code() << std::endl;
                }

//                auto messageId = actionData["messageId"].get<size_t>();

                newConnection->getMessages();
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