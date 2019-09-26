cmake_minimum_required(VERSION 3.14)
project(messagingClient)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(messagingClient
        CMakeLists.txt
        json.hpp
        main.cpp
        TcpClient.cpp
        TcpClient.h
        UdpClient.cpp
        UdpClient.h)
