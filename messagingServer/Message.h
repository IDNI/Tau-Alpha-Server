//
// Created by ak on 24/09/2019.
//

#ifndef MESSAGINGSERVER_MESSAGE_H
#define MESSAGINGSERVER_MESSAGE_H

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <set>
#include "./json.hpp"

#include "./UdpServer.h"
#include "./Routing.h"

using json = nlohmann::json;


class Message {
public:
    Message(std::string message, json &actionData, Routing &routing, UdpServer &udpServer) :
            routing_(routing),
            udpServer_(udpServer) {
        messageHash = std::hash<std::string>{}(message);
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " message hash " << messageHash << std::endl;

    }

    void save(json &actionData, std::string &messageString) {
        //save to disk
        std::ofstream messageHeader(std::to_string(messageHash)+".json");
        messageHeader << actionData << std::endl;

        std::ofstream messageContents(std::to_string(messageHash)+".msg");
        messageContents << messageString;
    }

    void notifyRecipients(json &actionData) {
        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        if (!actionData["destination"].is_array()) return;

        for (auto &[key, recipientName] : actionData["destination"].items()) {

            auto endpoint = routing_.getUdpEndpoint(recipientName);

            if (endpoint.port() == 0) {
                // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " Endpoint not found for: " << recipientName << '\n';
                continue;
            }

            // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " Got message for: " << recipientName << '\n';

            json msg;
            msg["action"] = "Got message";
            msg["messageId"] = messageHash;
            udpServer_.send(msg.dump(), endpoint);
        }
    }

    size_t messageHash;
private:
    Routing &routing_;
    UdpServer &udpServer_;
};


#endif //MESSAGINGSERVER_MESSAGE_H
