cmake_minimum_required(VERSION 3.14)
project(messagingClient)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.68.0 COMPONENTS system REQUIRED)

include_directories(.)

add_executable(messagingClient
        CMakeLists.txt
        json.hpp
        main.cpp
        TcpConnection.cpp
        TcpConnection.h
        UdpClient.cpp
        UdpClient.h
        MessageOperations.cpp
        MessageOperations.h MessageOperations.cpp MessageOperations.h)

TARGET_LINK_LIBRARIES( messagingClient
        boost_system
        boost_thread
        ssl
        crypto
        pthread
        stdc++fs
        )
