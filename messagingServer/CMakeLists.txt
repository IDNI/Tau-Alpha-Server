# cmake_minimum_required(VERSION <specify CMake version here>)
project(messagingServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(messagingServer main.cpp TcpServer.cpp TcpServer.h UdpServer.cpp UdpServer.h TcpConnection.cpp TcpConnection.h Routing.cpp Routing.h Message.cpp Message.h MessageOperations.cpp MessageOperations.h)
TARGET_LINK_LIBRARIES( messagingServer
        boost_system
        boost_thread
        pthread
        stdc++fs
        )
