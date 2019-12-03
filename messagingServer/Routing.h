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
    bool saveRouteFromUdpEndpoint(udp::endpoint &remote_endpoint, std::string &uid);

    udp::endpoint getUdpEndpoint(std::string uid);

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
