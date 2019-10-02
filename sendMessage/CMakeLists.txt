cmake_minimum_required(VERSION 3.14)
project(sendMessage)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.68.0 COMPONENTS system REQUIRED)

include_directories(.)

add_executable(sendMessage
        json.hpp
        main.cpp
        TcpConnection.cpp
        TcpConnection.h)

TARGET_LINK_LIBRARIES(sendMessage
        boost_system
        )
