cmake_minimum_required(VERSION 3.14)
project(sendMessage)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(sendMessage
        json.hpp
        main.cpp
        TcpClient.cpp
        TcpClient.h)
