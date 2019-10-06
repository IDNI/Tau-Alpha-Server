#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <filesystem>

#include <signal.h>

#include "MessageOperations.h"
#include "UdpClient.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

int main(int argc, char **argv) {
    std::filesystem::path dir = "messages";
    boost::thread_group threads;

    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " server port UID [messages directory]\n";
        std::cout << "Example: " << argv[0] << " 127.0.0.1 777 Andrei messages\n";
        return 0;
    }
    std::string server = argv[1];
    std::string port = argv[2];
    std::string uid = argv[3];

    if (argc > 4) {
        std::cout << "Using directory " << argv[4] << std::endl;
        dir = argv[4];
    } else {
        std::cout << "Using default directory: " << dir << std::endl;
    }

    std::filesystem::path messagesRootDirectory = "messages";

    if (!(std::filesystem::exists(messagesRootDirectory))) {
        if (std::filesystem::create_directory(messagesRootDirectory))
            std::cout << messagesRootDirectory << " successfully created!" << std::endl;
    }

    MessageOperations messageOperations(dir);

    try {
        //creating io queue
        boost::asio::io_context io_context;

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

        ctx.load_verify_file("ca.pem");

        tcp::resolver tcpResolver(io_context);
        udp::resolver udpResolver(io_context);
        //adding server endpoint we plan to to connect
        tcp::resolver::results_type tcpEndpoint = tcpResolver.resolve(server,
                                                                      port);
        udp::endpoint udpEndpoint = *udpResolver.resolve(server,
                                                         port).begin();


        UdpClient udpClient(io_context, ctx, udpEndpoint,
                            tcpEndpoint,
                            uid,
                            messageOperations); //to fetch a message

        threads.create_thread(
                boost::bind(&UdpClient::startReceive, &udpClient));

        threads.create_thread([&]() {
            boost::asio::executor_work_guard<decltype(io_context.get_executor())>
                    work{io_context.get_executor()};
            io_context.run();
        });


        udpClient.ping();


    }

    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    threads.join_all();
    return 0;
}