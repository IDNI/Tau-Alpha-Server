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

MessageOperations::MessageOperations(std::filesystem::path messagesRootDirectory) :
        messagesRootDirectory_(messagesRootDirectory) {

    if (!(std::filesystem::exists(messagesRootDirectory))) {
        if (std::filesystem::create_directory(messagesRootDirectory)) {
#ifdef DEBUG_COUT
            std::cout << messagesRootDirectory << " successfully created!" << std::endl;
#endif
        }
    }
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

