#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

#include "./Routing.h"
#include "./TcpServer.h"
#include "./Message.h"
#include "./UdpServer.h"


int main(int argc, char **argv) {
    boost::thread_group threads;
    std::unordered_map<size_t, Message *> messages;

    uint16_t portNumber;
    if (argc > 1)
        portNumber = atoi(argv[1]);
    else
        portNumber = 10000;

    try {
        boost::asio::io_context io_context;

        Routing routing;

        UdpServer udpServer(io_context, routing, portNumber);

        TcpServer tcpServer(io_context, udpServer, routing, messages, portNumber);

        io_context.run();

    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    threads.join_all();

    return 0;
}