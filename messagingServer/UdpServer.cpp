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

#include "UdpServer.h"

UdpServer::UdpServer(boost::asio::io_context &io_context, Routing &routing, uint16_t portNumber)
        : socket_(io_context, udp::endpoint(udp::v4(), portNumber)),
          routing_(routing) {
    std::cout << "UDP server listening on port " << portNumber << std::endl;
    startReceive();
}

void UdpServer::send(std::string messageString, udp::endpoint &remote_endpoint) {

    boost::shared_ptr<std::string> message(
            new std::string(messageString));

//        std::cout << "Sending: "
//                  << *message << ": " << message
//                  << " to "
//                  << remote_endpoint.address() << ":" << remote_endpoint.port()
//                  << std::endl;
    socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint,
                          boost::bind(&UdpServer::handleSend, this, message));

}

void UdpServer::startReceive() {
//        std::cout << "startReceive" << std::endl;

    socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            boost::bind(&UdpServer::handleReceive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}


void UdpServer::makeActionBasedOnMessage(json &receivedMessage, udp::endpoint &remote_endpoint) {
//        std::cout << __FILE__ << " : " << __FUNCTION__ << " : "
//                  << std::dec << __LINE__ << std::endl;

    if ("ping" == receivedMessage["action"].get<std::string>()) {
//            std::cout << "processing PING from "
//                      << remote_endpoint.address()
//                      << ":"
//                      << remote_endpoint.port()
//                      << std::endl;

        //we need to check his authorization
        //checkUserAuthorization();
        //we need to find that user in our map
        std::string uid = receivedMessage["uid"].get<std::string>();

        bool found = routing_.saveRouteFromUdpEndpoint(remote_endpoint, uid);

        if(found) {
            send("pong", remote_endpoint);
        } else {
            //later this will be triggered in authorisation
            //because one UDP packet we send here can get lost easily
            json msg;
            msg["action"] = "Got messages";
            send(msg.dump(), remote_endpoint);
        }
    }
}

void UdpServer::processIncomingMessage(std::string incomingMessage,
                                       udp::endpoint remote_endpoint) {
    json parsedMessage = parseIncomingMessage(incomingMessage);

//        std::cout << "parsedMessage.size(): " << parsedMessage.size()
//                  << std::endl;

    if (parsedMessage.size()) makeActionBasedOnMessage(parsedMessage, remote_endpoint);
}

json UdpServer::parseIncomingMessage(std::string &incomingMessage) {
//        std::cout << "got message: "
//                  << incomingMessage << std::endl;

    json msg;

    try {
        msg = json::parse(incomingMessage);
    } catch (std::exception &e) {
        std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                  << std::dec << __LINE__ << " : " << e.what() << std::endl;
    }


    if (msg.contains("action")) {
//            std::cout << "parsed message: "
//                      << msg["action"].get<std::string>()
//                      << " size " << msg.size()
//                      << std::endl;
    } else {
        msg.clear();
    }

    return msg;
}

//    int counter =0;
void UdpServer::handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred) {
    if (!error) {
        //we pass the load we got from UDP packet as a string
        processIncomingMessage(std::string(recv_buffer_.begin(), bytes_transferred),
                               remote_endpoint_);

//            boost::shared_ptr<std::string> message(
//                    new std::string(std::string(std::to_string(counter++)+"\n")));
//
//            std::cout << "Sending: "
//                      << *message << ": "<< message << std::endl;
//            socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
//                                  boost::bind(&UdpServer::handleSend, this, message));
//
        startReceive();
    }
}

void UdpServer::handleSend(boost::shared_ptr<std::string> message) {
//        std::cout << "Sent "
//                  << message << std::endl;
}
