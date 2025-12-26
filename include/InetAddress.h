#pragma once

#include <netinet/in.h>
#include <string>

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
    InetAddress(const std::string& ip, uint16_t port);

    const sockaddr* getSockAddr() const;
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

private:
    union{
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};