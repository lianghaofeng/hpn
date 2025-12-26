#include "TcpConnection.h"
#include "EventLoop.h"
#include <cassert>
#include <functional>
#include <sys/socket.h>
#include <unistd.h>

TcpConnection::TcpConnection(EventLoop *loop, Socket &&socket)
    : loop_(loop), socket_(std::move(socket)),
      channel_(new Channel(loop, socket_.fd())), state_(kConnecting) {}

TcpConnection::~TcpConnection() {
    assert(state_ == kDisconnected || state_ == kConnecting);
}

void TcpConnection::connectEstablished() {
    assert(state_ == kConnecting);
    setState(kConnected);

    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));

    channel_->enableReading();

    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
    }
    channel_->remove();
}

void TcpConnection::handleRead() {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(socket_.fd(), &savedErrno);
    if (n > 0) {
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        ssize_t n = ::write(socket_.fd(), outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);

            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();

                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            handleError();
        }
    }
}

void TcpConnection::handleClose() {
    assert(state_ == kConnected || state_ == kDisconnected);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());

    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }

    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::handleError() {
    int err;
    socklen_t errlen = sizeof(err);
    if (::getsockopt(socket_.fd(), SOL_SOCKET, SO_ERROR, &err, &errlen) == 0) {
        // 记录错误日志
    }
    // 出现错误后关闭连接
    handleClose();
}

void TcpConnection::send(const std::string &message) {
    send(message.data(), message.size());
}

void TcpConnection::send(const char *data, size_t len) {
    if (state_ == kConnected) {
        sendInLoop(std::string(data, len));
    }
}

void TcpConnection::sendInLoop(const std::string &message) {
    ssize_t nwrote = 0;
    size_t remaining = message.size();

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(socket_.fd(), message.data(), message.size());

        if (nwrote >= 0) {
            remaining = message.size() - nwrote;
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    // 连接断开
                }
                handleError();
                return;
            }
        }
    }

    // 如果还没发送完，将剩余数据写入输出缓冲区
    if (remaining > 0) {
        outputBuffer_.append(message.data() + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        shutdownInLoop();
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        // 关闭写端
        ::shutdown(socket_.fd(), SHUT_WR);
    }
}