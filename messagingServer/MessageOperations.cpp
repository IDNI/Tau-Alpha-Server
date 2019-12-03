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

#include "MessageOperations.h"

MessageOperations::MessageOperations(std::filesystem::path messagesRootDirectory, Routing &routing,
                                     UdpServer &udpServer) :
        routing_(routing),
        udpServer_(udpServer),
        messagesRootDirectory_(messagesRootDirectory) {

    if (!(std::filesystem::exists(messagesRootDirectory))) {
        if (std::filesystem::create_directory(messagesRootDirectory)) {
#ifdef DEBUG_COUT
            std::cout << messagesRootDirectory << " successfully created!" << std::endl;
#endif
        }
    }
}


void MessageOperations::save(json &actionData, std::istream &inputStream, size_t messageSize) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    char *messageBuffer = loadStreamIntoTheMemory(inputStream, messageSize);

    size_t messageHash =
            std::hash<std::string_view>{}(std::string_view(messageBuffer, messageSize));


    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
#ifdef DEBUG_COUT
    std::cout << "Message time is " << oss.str() << std::endl;
#endif
    actionData["timeReceived"] = oss.str();
    actionData["messageSize"] = messageSize;
    oss.str("");
    oss.clear();
    oss << messagesRootDirectory_.string() << "/" << std::put_time(&tm, "%d-%m-%Y");
    if (!(std::filesystem::exists(oss.str()))) {
#ifdef DEBUG_COUT
        std::cout << oss.str() << " directory not found!" << std::endl;
#endif
        if (std::filesystem::create_directory(oss.str())) {
#ifdef DEBUG_COUT
            std::cout << oss.str() << " directory successfully created!" << std::endl;
#endif
        }
    }


    oss << "/" << std::to_string(messageHash);
    for (auto &[key, recipientName] : actionData["destination"].items()) {
        unsentMessages[recipientName].insert(oss.str());
#ifdef DEBUG_COUT
        std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
                         << " unsentMessages["<<recipientName<<"] size "
                         << unsentMessages[recipientName].size()
                         << " added " << oss.str() <<  std::endl;
#endif
    }

    //save to disk
#ifdef DEBUG_COUT
    std::cout << "saving file " << oss.str() << ".json" << std::endl;
#endif
    std::ofstream messageHeader(oss.str() + ".json");
    messageHeader << std::string(actionData.dump() + "\n");
    messageHeader.close();

#ifdef DEBUG_COUT
    std::cout << "saving file " << oss.str() << ".msg" << std::endl;
#endif
    std::ofstream messageContents(oss.str() + ".msg");
    messageContents.write(messageBuffer, messageSize);
    messageContents.close();

    std::cout << "Got message: " << actionData.dump() << std::endl;
    delete[] messageBuffer;
}

void MessageOperations::notifyRecipients(json &actionData) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (!actionData["destination"].is_array()) return;

    for (auto &[key, recipientName] : actionData["destination"].items()) {
        auto endpoint = routing_.getUdpEndpoint(recipientName);

        if (endpoint.port() == 0) {
            // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " Endpoint not found for: " << recipientName << '\n';
            continue;
        }

        // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " Got message for: " << recipientName << '\n';

        json msg;
        msg["action"] = "Got messages";
//            msg["messageId"] = messageHash;
        udpServer_.send(msg.dump(), endpoint);
    }
}

std::set<std::string>::iterator
MessageOperations::sent(std::string recipientName, std::set<std::string>::iterator &messagePath) {
    return unsentMessages[recipientName].erase(messagePath);
}

std::set<std::string> &MessageOperations::getUnsentMessagesFor(std::string recipientName) {
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
              << " unsentMessages[" << recipientName << "] size "
              << unsentMessages[recipientName].size()
              << std::endl;

    return unsentMessages[recipientName];
}

std::tuple<char *, size_t> MessageOperations::getMessageHeader(std::string messagePath) {
    return loadFileIntoTheMemory(
            messagePath + ".json");
}

std::tuple<char *, size_t> MessageOperations::getMessage(std::string messagePath) {
    return loadFileIntoTheMemory(
            messagePath + ".msg");
}

char *MessageOperations::loadStreamIntoTheMemory(std::istream &inStream,
                                                 size_t streamSize) {
    char *messageBuffer = new char[streamSize];

    inStream.read(messageBuffer, streamSize);

    if (inStream) {
#ifdef DEBUG_COUT
        std::cout << "Loaded stream into memory"
                         //                  << std::string(messageBuffer,fileSize)
                         << " size " << streamSize << std::endl;
#endif
    } else {
        std::cerr << "Only " << inStream.gcount() << " could be read" << std::endl;
    }
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                 //                                                                    << messageBuffer
                 << std::endl;
#endif
    return messageBuffer;
}

std::tuple<char *, size_t> MessageOperations::loadFileIntoTheMemory(std::string fileName) {
#ifdef DEBUG_COUT
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    if (!fileName.length()) {
        return std::make_tuple(nullptr, 0);
    }

    std::ifstream inFile(fileName, std::ios::in | std::ios::binary);

    if (!inFile) {
        std::cerr << "Can't open the file " << fileName << std::endl;
        return std::make_tuple(nullptr, 0);
    }

    inFile.seekg(0, inFile.end);
    size_t fileSize = (std::size_t) inFile.tellg();
    inFile.seekg(0, inFile.beg);
#ifdef DEBUG_COUT
    std::cout << "File size " << fileSize << std::endl;
#endif

    char *messageBuffer = loadStreamIntoTheMemory(inFile, fileSize);

    inFile.close();

    return std::make_tuple(messageBuffer, fileSize);
}