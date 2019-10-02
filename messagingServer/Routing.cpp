//
// Created by ak on 24/09/2019.
//

#include "Routing.h"

bool Routing::saveRouteFromUdpEndpoint(udp::endpoint &remote_endpoint, std::string &uid) {
    uint32_t clientIp = remote_endpoint.address().to_v4().to_uint();
    uint16_t clientPort = remote_endpoint.port();
//        std::cout << __FUNCTION__ << " Got socket address " << clientIp << ":" << clientPort << std::endl;
    ClientIpPort clientIpPort{clientIp, clientPort};

    if (addressToClient.find(clientIpPort) == addressToClient.end()) {
        #ifdef DEBUG_COUT
 std::cout << "Socket address " << clientIp << ": " << clientPort
                  << ": " << uid
                  << " not found in map"
                  << std::endl;
 #endif
        addressToClient[clientIpPort] = uid;
        clientToAddress[uid] = remote_endpoint;
        return false;
    } else {
//        std::cout << "Socket address " << clientIp << ":" << clientPort
//                  << ": " << uid
//                  << " found in map"
//                  << std::endl;
    }

    return true;
}

udp::endpoint Routing::getUdpEndpoint(std::string uid) {
    udp::endpoint destination;
    try {
        destination = clientToAddress.at(uid);
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    } catch (...) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    }
//        std::cout << "endpoint ip " << destination.address() << std::endl;
//        std::cout << "endpoint port " << destination.port() << std::endl;
//        std::cout << "endpoint protocol " << (destination.protocol() == boost::asio::ip::udp::v4()) << std::endl;
    return destination;
}

