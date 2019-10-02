cmake_minimum_required(VERSION 3.14)
project(messagingServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE ON)
find_package(Boost 1.68.0 COMPONENTS system REQUIRED)

add_executable(messagingServer
        main.cpp TcpServer.cpp TcpServer.h UdpServer.cpp UdpServer.h TcpConnection.cpp TcpConnection.h Routing.cpp Routing.h MessageOperations.cpp MessageOperations.h)
TARGET_LINK_LIBRARIES(messagingServer
        boost_system
        boost_thread
        pthread
        stdc++fs
        )

