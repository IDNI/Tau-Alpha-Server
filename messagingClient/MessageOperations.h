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
