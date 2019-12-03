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
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/connect.hpp>
#include <fstream>      // std::ifstream
#include <tuple>
#include <signal.h>

using boost::asio::ip::tcp;

#include "TcpConnection.h"

std::tuple<char *, size_t> loadFileIntoTheMemory(char *fileName) {
    std::ifstream inFile(fileName, std::ios::in | std::ios::binary);
    std::size_t fileSize = 0;
    if (!inFile) {
        std::cout << "Can't open the file " << fileName << std::endl;
    }

    inFile.seekg(0, inFile.end);
    fileSize = (std::size_t) inFile.tellg();
    inFile.seekg(0, inFile.beg);
    std::cout << "File size " << fileSize << std::endl;

    char *messageBuffer = new char[fileSize];

    inFile.read(messageBuffer, fileSize);
    if (inFile) {
#ifdef DEBUG_COUT
        std::cout << "Loaded file " << fileName << " into memory:"
                  //                  << std::string(messageBuffer,fileSize)
                  << " fileSize " << fileSize << std::endl;
#endif
    } else {
        std::cerr << "Only " << inFile.gcount() << " could be read" << std::endl;
    }
    inFile.close();

    return std::make_tuple(messageBuffer, fileSize);
}

TcpConnection *connectToServer(std::string &server, std::string &port,
                               boost::asio::io_context &io_context,
                               boost::asio::ssl::context &ctx) {

    tcp::resolver tcpResolver(io_context);
    //adding server endpoint we plan to to connect
    tcp::resolver::results_type tcpEndpoint =
            tcpResolver.resolve(server,
                                port);


    TcpConnection *tcpClient = new TcpConnection(io_context,ctx);

    try {
        tcpClient->connect(tcpEndpoint);
    } catch (boost::system::system_error &e) {
        if (e.code() == boost::asio::error::not_found)
            std::cerr << "connect returned not_found" << std::endl;
        else
            std::cerr << "connect returned error " << e.code() << std::endl;

        return nullptr;
    }

    return tcpClient;
}

std::set<std::string> convertCommaSeparatedStringToSet(char *lineToConvert) {
    std::set<std::string> parsedString;
    std::stringstream ss(lineToConvert);
    std::string str;
    while (getline(ss, str, ',')) {
        parsedString.insert(str);
    }

    return parsedString;
}

int main(int argc, char **argv) {
    TcpConnection *tcpClient = nullptr;

    if (argc < 6) {
        std::cout << "Usage: "
                  << argv[0] << " serverIP serverPort myUID destinationUIDs(comma separated) fileToSend\n"
                  << "For example:\n"
                  << argv[0] << " 1.2.3.4 777 Andrei Fola,Isar,Ohad,Tomas ./text.txt\n";
    }

    std::string server = argv[1];
    std::string port = argv[2];
    std::string uid = argv[3];

    std::set<std::string> destinations = convertCommaSeparatedStringToSet(argv[4]);

    auto[messageBuffer, fileSize] = loadFileIntoTheMemory(argv[5]);

    try {
        //creating io queue for async actions
        boost::asio::io_context io_context;

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        ctx.load_verify_file("ca.pem");

        //connecting to the desired server
        tcpClient = connectToServer(server, port, io_context, ctx);

        if (tcpClient == nullptr) return 0;

        //sending the message to destinations
        tcpClient->sendMessage(uid, destinations, messageBuffer, fileSize);

        //executing jobs queue
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    if (tcpClient != nullptr) delete tcpClient;

    return 1;
}
