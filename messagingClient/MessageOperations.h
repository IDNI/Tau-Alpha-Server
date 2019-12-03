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
// Created by ak on 30/09/2019.
//

#ifndef MESSAGINGCLIENT_MESSAGEOPERATIONS_H
#define MESSAGINGCLIENT_MESSAGEOPERATIONS_H

#include <iostream>
#include <set>
#include <unordered_map>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <fstream>

#include "json.hpp"

using json = nlohmann::json;

class MessageOperations {
public:
    MessageOperations(std::filesystem::path messagesRootDirectory);

    void save(json &actionData, std::istream &inputStream, size_t messageSize);

private:
    char *loadStreamIntoTheMemory(std::istream &inStream, size_t streamSize);

    std::filesystem::path messagesRootDirectory_;
};


#endif //MESSAGINGCLIENT_MESSAGEOPERATIONS_H
