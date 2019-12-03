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
// Created by ak on 29/09/2019.
//

#ifndef MESSAGINGSERVER_MESSAGEOPERATIONS_H
#define MESSAGINGSERVER_MESSAGEOPERATIONS_H

#include <set>
#include <unordered_map>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <fstream>

#include "json.hpp"
#include "UdpServer.h"

using json = nlohmann::json;

class MessageOperations {
public:
    MessageOperations(std::filesystem::path messagesRootDirectory, Routing &routing, UdpServer &udpServer);

    void save(json &actionData, std::istream &inputStream, size_t messageSize);

    void notifyRecipients(json &actionData);

    std::set<std::string>::iterator sent(std::string recipientName, std::set<std::string>::iterator &messagePath);

    std::set<std::string> &getUnsentMessagesFor(std::string recipientName);

    std::tuple<char *, size_t> getMessageHeader(std::string messagePath);
    std::tuple<char *, size_t> getMessage(std::string messagePath);
private:
    std::tuple<char *, size_t> loadFileIntoTheMemory(std::string fileName);
    char *loadStreamIntoTheMemory(std::istream &inStream,
                                                                          size_t streamSize);

    std::unordered_map<std::string, std::set<std::string>> unsentMessages;

    Routing &routing_;
    UdpServer &udpServer_;
    std::filesystem::path messagesRootDirectory_;
};


#endif //MESSAGINGSERVER_MESSAGEOPERATIONS_H
