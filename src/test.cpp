#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    struct sockaddr_in ipv4_addr;
    
    std::cout << "sizeof(sockaddr_in) = " << sizeof(sockaddr_in) << std::endl;
    std::cout << "sizeof(sockaddr) = " << sizeof(sockaddr) << std::endl;
    
    // 获取地址
    std::cout << "\n内存地址：" << std::endl;
    std::cout << "&ipv4_addr = " << &ipv4_addr << std::endl;
    std::cout << "(sockaddr*)&ipv4_addr = " << (sockaddr*)&ipv4_addr << std::endl;
    
    // 实际内容对比
    std::cout << "\n结构体内容：" << std::endl;
    std::cout << "ipv4_addr.sin_family 地址: " << &ipv4_addr.sin_family << std::endl;
    std::cout << "ipv4_addr.sin_port 地址: " << &ipv4_addr.sin_port << std::endl;
    std::cout << "ipv4_addr.sin_addr 地址: " << &ipv4_addr.sin_addr << std::endl;
    
    return 0;
}