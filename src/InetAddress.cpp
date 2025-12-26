#include "InetAddress.h"
#include <arpa/inet.h>
#include <cstring>

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) : addr_{} {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port): addr_{}{
    addr_.sin_family= AF_INET;
    addr_.sin_port = htons(port);

    if(::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0){
        // 错误处理
    }
}

// 获取sockaddr指针
const sockaddr* InetAddress::getSockAddr() const{
    return reinterpret_cast<const sockaddr*>(&addr_);
}

// 二进制到ip
std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

// 二进制到ip+port
std::string InetAddress::toIpPort() const {
    char buf[INET_ADDRSTRLEN + 6]; //ip + : + port
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = ::strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    snprintf(buf + end, sizeof(buf) - end, ":%u", port);
    return buf;
}

// 获取端口号
uint16_t InetAddress::port() const{
    return ntohs(addr_.sin_port);
}