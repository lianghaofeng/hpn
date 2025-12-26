cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIED ON)

# 启用所有警告，优化级别2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
# 生成调试信息，不优化方便调试
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(include)

set(SOURCES
    src/EventLoop.cpp
    src/Channel.cpp
    src/Socket.cpp
    src/Buffer.cpp
    src/TcpConnection.cpp
)

add_library(hpn STATIC
    ${SOURCES}
)

add_executable(test_eventloop
    tests/test_eventloop.cpp
)
target_link_libraries(test_eventloop hpn)

add_executable(test_socket
    tests/test_socket.cpp
)
target_link_libraries(test_socket hpn)

add_executable(test_buffer
    tests/test_buffer.cpp
)
target_link_libraries(test_buffer hpn)

add_executable(test_tcpconnection
    tests/test_tcpconnection.cpp
)
target_link_libraries(test_tcpconnection hpn)


# 启用ctest
enable_testing()
add_test(NAME SocketTest COMMAND test_socket)
add_test(NAME EventLoopTest COMMAND test_eventloop)
add_test(NAME BufferTest COMMAND test_buffer)
add_test(NAME TcpConnectionTest COMMAND test_tcpconnection)


