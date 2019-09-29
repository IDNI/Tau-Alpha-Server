//
// Created by ak on 29/09/2019.
//

#include "MessageOperations.h"

MessageOperations::MessageOperations(std::filesystem::path messagesRootDirectory, Routing &routing, UdpServer &udpServer) :
        routing_(routing),
        udpServer_(udpServer),
        messagesRootDirectory_(messagesRootDirectory) {

    if(!(std::filesystem::exists(messagesRootDirectory))){
        if (std::filesystem::create_directory(messagesRootDirectory))
            std::cout << messagesRootDirectory << " successfully created!" << std::endl;
    }
}


void MessageOperations::save(json &actionData, std::string &messageString) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    messageHash = std::hash<std::string>{}(messageString);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
    std::cout << "Message time is " << oss.str() << std::endl;
    actionData["timeReceived"] = oss.str();
    oss.str("");
    oss.clear();
    oss << messagesRootDirectory_.string() << "/" << std::put_time(&tm, "%d-%m-%Y");
    if(!(std::filesystem::exists(oss.str()))){
        std::cout << oss.str() << " directory not found!" << std::endl;
        if (std::filesystem::create_directory(oss.str()))
            std::cout << oss.str() << " directory successfully created!" << std::endl;
    }


    oss << "/" << std::to_string(messageHash);
    for (auto &[key, recipientName] : actionData["destination"].items()) {
        unsentMessages[recipientName].insert(oss.str());
    }

    //save to disk
    std::cout << "saving file "<< oss.str() << ".json" << std::endl;
    std::ofstream messageHeader(oss.str()+".json");
    messageHeader << actionData << std::endl;

    std::cout << "saving file "<< oss.str() << ".msg" << std::endl;
    std::ofstream messageContents(oss.str()+".msg");
    messageContents << messageString;
}

void MessageOperations::notifyRecipients(json &actionData) {
    // std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    std::time_t now =     std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

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

void MessageOperations::sent(std::string recipientName, std::string messagePath) {
    unsentMessages[recipientName].erase(messagePath);
}

std::set<std::string> &MessageOperations::getUnsentMessagesFor(std::string recipientName) {
    return unsentMessages[recipientName];
}

std::tuple<char *, size_t> MessageOperations::getMessage(std::string messagePath) {
    return loadFileIntoTheMemory(
            messagePath + ".msg");
}

std::tuple<char *, size_t> MessageOperations::loadFileIntoTheMemory(std::string fileName) {
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (!fileName.length()) {
        return std::make_tuple(nullptr, 0);
    }

    std::ifstream inFile(fileName, std::ios::in | std::ios::binary);
    std::size_t fileSize = 0;
    if (!inFile) {
        std::cerr << "Can't open the file " << fileName << std::endl;
        return std::make_tuple(nullptr, 0);
    }

    inFile.seekg(0, inFile.end);
    fileSize = (std::size_t) inFile.tellg();
    inFile.seekg(0, inFile.beg);
    std::cout << "File size " << fileSize << std::endl;

    char *messageBuffer = new char[fileSize];

    inFile.read(messageBuffer, fileSize);
    if (inFile)
        std::cout << "Loaded file " << fileName << " into memory:"
                  << messageBuffer << " " << inFile.gcount() << std::endl;
    else
        std::cerr << "Only " << inFile.gcount() << " could be read" << std::endl;

    inFile.close();

    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    return std::make_tuple(messageBuffer, fileSize);
}