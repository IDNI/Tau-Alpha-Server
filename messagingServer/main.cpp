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
#include <iostream>
#include <string>
#include <boost/asio.hpp>
//#include <boost/thread.hpp>
#include <filesystem>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

#include "Routing.h"
#include "TcpServer.h"
#include "MessageOperations.h"
#include "UdpServer.h"

int main(int argc, char **argv) {
//    boost::thread_group threads;
    std::filesystem::path dir = "messages";

    uint16_t portNumber;
    if (argc > 1) {
        std::cout << "Port set to: " << argv[1] << std::endl;
        portNumber = atoi(argv[1]);
    }
    else {
        std::cout << "Usage: " << argv[0] << " portToListen directoryForMessages" << std::endl;
        std::cout << "Example: " << argv[0] << " 10000 messages" << std::endl;
        std::cout << "\nUsing default port 10000" << std::endl;
        portNumber = 10000;
    }

    if(argc > 2) {
        std::cout << "Using directory " << argv[2] << std::endl;
        dir = argv[2];
    }
    else {
        std::cout << "Using default directory: " << dir << std::endl;
    }

    try {
        boost::asio::io_context io_context;

        Routing routing;

        UdpServer udpServer(io_context, routing, portNumber);

        MessageOperations messageOperations(dir, routing, udpServer);

        TcpServer tcpServer(io_context, messageOperations, portNumber);

        io_context.run();

    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

//    threads.join_all();

    return 0;
}