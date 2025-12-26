# CMake generated Testfile for 
# Source directory: /workspace/hpn
# Build directory: /workspace/hpn/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(SocketTest "/workspace/hpn/build/test_socket")
set_tests_properties(SocketTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hpn/CMakeLists.txt;49;add_test;/workspace/hpn/CMakeLists.txt;0;")
add_test(EventLoopTest "/workspace/hpn/build/test_eventloop")
set_tests_properties(EventLoopTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hpn/CMakeLists.txt;50;add_test;/workspace/hpn/CMakeLists.txt;0;")
add_test(BufferTest "/workspace/hpn/build/test_buffer")
set_tests_properties(BufferTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hpn/CMakeLists.txt;51;add_test;/workspace/hpn/CMakeLists.txt;0;")
add_test(TcpConnectionTest "/workspace/hpn/build/test_tcpconnection")
set_tests_properties(TcpConnectionTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hpn/CMakeLists.txt;52;add_test;/workspace/hpn/CMakeLists.txt;0;")
