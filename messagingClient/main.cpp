#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/connect.hpp>
#include <fstream>      // std::ifstream

#include <signal.h>

#include "TcpConnection.h"
#include "./UdpClient.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, char **argv) {
    boost::thread_group threads;

    if(argc < 4) {
        std::cout << "Usage: " << argv[0] << " server port UID\n";
        std::cout << "Example: " << argv[0] << " 127.0.0.1 777 Andrei\n";
        return 0;
    }
    std::string server = argv[1];
    std::string port = argv[2];
    std::string uid = argv[3];

    try {
        //creating io queue
        boost::asio::io_context io_context;


        tcp::resolver tcpResolver(io_context);
        udp::resolver udpResolver(io_context);
        //adding server endpoint we plan to to connect
//        tcp::resolver::results_type
        tcp::resolver::results_type tcpEndpoint = tcpResolver.resolve(argv[1],
                                                                      argv[2]);
        udp::endpoint udpEndpoint = *udpResolver.resolve(argv[1],
                                                         argv[2]).begin();


        std::cout << __FILE__<< " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        UdpClient udpClient(io_context, udpEndpoint,
                tcpEndpoint, uid); //to fetch a message

        threads.create_thread(
                boost::bind(&UdpClient::startReceive, &udpClient));

        threads.create_thread([&](){
            std::cout << __FILE__<< " io_context starting " << &io_context << std::endl;
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