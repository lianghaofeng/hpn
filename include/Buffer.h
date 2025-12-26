#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <sys/uio.h>

/**
 * 设计：
 * 1. 使用std::vector<char> 作为底层存储
 * 2. 维护readIndex和writeIndex
 * 3. 预留头部空间用于协议头
 * 4. 自动增长
 */

class Buffer{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    // 显示构造
    explicit Buffer(size_t initialSize = kInitialSize):
        buffer_(kCheapPrepend + initialSize),
        readIndex_(kCheapPrepend),
        writeIndex_(kCheapPrepend){}

    size_t readableBytes() const {
        return writeIndex_ - readIndex_;
    }

    size_t writableBytes() const {
        return buffer_.size() - writeIndex_;
    }

    size_t prependableBytes() const {
        return readIndex_;
    }

    // 返回刻度
    const char* peek() const {
        return begin() + readIndex_;
    }

    // 消费len字节的数据
    void retrieve(size_t len) {
        if(len < readableBytes()){
            readIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    // 消费所有数据
    void retrieveAll() {
        readIndex_ = kCheapPrepend;
        writeIndex_ = kCheapPrepend;
    }

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    // 获取可写位置指针
    char* beginWrite() {
        return begin() + writeIndex_;
    }

    const char* beginWrite() const{
        return begin() + writeIndex_;
    }

    void hasWritten(size_t len){
        writeIndex_ += len;
    }

    void ensureWritableBytes(size_t len){
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len){
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const std::string& str){
        append(str.data(), str.size());
    }

    // 从fd读取数据
    // 返回读取的字节数， -1表示错误
    ssize_t readFd(int fd, int* savedErrno);

private:
    char *begin(){
        return &*buffer_.begin(); 
    }

    const char *begin() const {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len) {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend){
            buffer_.resize(writeIndex_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + readIndex_,
                    begin() + writeIndex_,
                    begin() + kCheapPrepend);
            readIndex_ = kCheapPrepend;
            writeIndex_ = readIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;

};