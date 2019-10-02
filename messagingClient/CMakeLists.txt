cmake_minimum_required(VERSION 3.14)
project(messagingClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-lboost_system -lboost_thread -lpthread")
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
        pthread
        stdc++fs
        )
