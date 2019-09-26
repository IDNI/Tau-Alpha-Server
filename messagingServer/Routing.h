//
// Created by ak on 24/09/2019.
//

#ifndef MESSAGINGSERVER_ROUTING_H
#define MESSAGINGSERVER_ROUTING_H

#include <cstdint>
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::udp;

template<class T>
inline void hash_combine(std::size_t &s, const T &v) {
    std::hash<T> h;
    s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}


class Routing {
public:
    void saveRouteFromUdpEndpoint(udp::endpoint &remote_endpoint, std::string &uid) {
        uint32_t clientIp = remote_endpoint.address().to_v4().to_uint();
        uint16_t clientPort = remote_endpoint.port();
//        std::cout << __FUNCTION__ << " Got socket address " << clientIp << ":" << clientPort << std::endl;
        ClientIpPort clientIpPort{clientIp, clientPort};

        if (addressToClient.find(clientIpPort) == addressToClient.end()) {
            std::cout << "Socket address " << clientIp << ": " << clientPort
                      << ": " << uid
                      << " not found in map"
                      << std::endl;
            addressToClient[clientIpPort] = uid;
            clientToAddress[uid] = remote_endpoint;
        } else {
            std::cout << "Socket address " << clientIp << ":" << clientPort
                      << ": " << uid
                      << " found in map"
                      << std::endl;
        }
    }

    udp::endpoint getUdpEndpoint(std::string uid) {
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

private:
    struct ClientIpPort {
        uint32_t ip;
        uint16_t port;

        bool operator==(const ClientIpPort &other) const {
            return (ip == other.ip
                    && port == other.port);
        }
    };


    struct IpPort_hash_fn {
        std::size_t operator()(ClientIpPort const &clientIpPort) const {
            std::size_t res = 0;
            hash_combine(res, (uint32_t) (clientIpPort.ip));
            hash_combine(res, (uint16_t) (clientIpPort.port));
            return res;
        }
    };

    std::unordered_map<struct ClientIpPort, std::string,
            IpPort_hash_fn> addressToClient;

    std::unordered_map<std::string, udp::endpoint> clientToAddress;


};


#endif //MESSAGINGSERVER_ROUTING_H
