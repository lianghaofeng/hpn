#include "../include/Buffer.h"
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <iostream>

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "..."; \
    name(); \
    std::cout << " PASSED" << std::endl; \
} while(0)

// 测试 1: 基本的 append 和 retrieve
TEST(test_buffer_append_retrieve) {
    Buffer buf;

    // 初始状态
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == Buffer::kInitialSize);
    assert(buf.prependableBytes() == Buffer::kCheapPrepend);

    // 写入数据
    const char* str = "hello";
    buf.append(str, strlen(str));

    assert(buf.readableBytes() == 5);
    assert(buf.writableBytes() == Buffer::kInitialSize - 5);

    // 读取数据
    std::string result = buf.retrieveAllAsString();
    assert(result == "hello");
    assert(buf.readableBytes() == 0);
    assert(buf.prependableBytes() == Buffer::kCheapPrepend);
}

// 测试 2: 自动增长
TEST(test_buffer_growth) {
    Buffer buf;

    // 写入大量数据，触发扩容
    std::string largeData(2000, 'x');
    buf.append(largeData);

    assert(buf.readableBytes() == 2000);

    std::string result = buf.retrieveAllAsString();
    assert(result == largeData);
}

// 测试 3: 预留空间维护
TEST(test_buffer_prepend_space) {
    Buffer buf;

    buf.append("hello", 5);
    buf.append("world", 5);

    assert(buf.readableBytes() == 10);

    // 读取部分数据
    buf.retrieve(5);
    assert(buf.readableBytes() == 5);

    // 预留空间应该增加了
    assert(buf.prependableBytes() == Buffer::kCheapPrepend + 5);
}

// 测试 4: 空间压缩
TEST(test_buffer_compact) {
    Buffer buf;

    // 写满初始空间
    std::string data1(800, 'a');
    buf.append(data1);

    // 读取大部分
    buf.retrieve(700);
    assert(buf.readableBytes() == 100);

    // 再写入数据，应该触发压缩而不是扩容
    std::string data2(300, 'b');

    std::cout << "Before append(data2):" << std::endl;
    std::cout << "  readIndex: " << buf.prependableBytes()<< std::endl;
    std::cout << "  writeIndex: " << buf.prependableBytes() + buf.readableBytes() << std::endl;
    std::cout << "  readable: " << buf.readableBytes() << std::endl;
    std::cout << "  writable: " << buf.writableBytes() << std::endl;
    std::cout << "  prependable: " << buf.prependableBytes() << std::endl;


    size_t oldSize = buf.readableBytes() + buf.writableBytes() + buf.prependableBytes();
    buf.append(data2);
    size_t newSize = buf.readableBytes() + buf.writableBytes() + buf.prependableBytes();

    std::cout << "After append(data2):" << std::endl;
    std::cout << "  readIndex: " << buf.prependableBytes()<< std::endl;
    std::cout << "  writeIndex: " << buf.prependableBytes() + buf.readableBytes() << std::endl;
    std::cout << "  readable: " << buf.readableBytes() << std::endl;
    std::cout << "  writable: " << buf.writableBytes() << std::endl;
    std::cout << "  prependable: " << buf.prependableBytes() << std::endl;
    std::cout << "  oldSize: " << oldSize << std::endl;
    std::cout << "  newSize: " << newSize << std::endl;

    // 压缩后，预留空间应该恢复到初始大小
    assert(buf.prependableBytes() == Buffer::kCheapPrepend);
    assert(buf.readableBytes() == 400);
}

// 测试 5: 从 fd 读取
TEST(test_buffer_readfd) {
    Buffer buf;

    // 创建 pipe
    int pipefd[2];
    assert(pipe(pipefd) == 0);

    // 写入数据到 pipe
    const char* msg = "test message from pipe";
    ssize_t nwrite = write(pipefd[1], msg, strlen(msg));
    assert(nwrite == static_cast<ssize_t>(strlen(msg)));

    // 从 pipe 读取到 buffer
    int savedErrno = 0;
    ssize_t n = buf.readFd(pipefd[0], &savedErrno);

    assert(n == static_cast<ssize_t>(strlen(msg)));
    assert(buf.readableBytes() == strlen(msg));

    std::string result = buf.retrieveAllAsString();
    assert(result == msg);

    close(pipefd[0]);
    close(pipefd[1]);
}

// 测试 6: peek 操作
TEST(test_buffer_peek) {
    Buffer buf;

    buf.append("hello", 5);

    // peek 不应该消费数据
    const char* data = buf.peek();
    assert(strncmp(data, "hello", 5) == 0);
    assert(buf.readableBytes() == 5);

    // 再次 peek 应该得到相同数据
    data = buf.peek();
    assert(strncmp(data, "hello", 5) == 0);

    // retrieve 才消费数据
    buf.retrieve(5);
    assert(buf.readableBytes() == 0);
}

// 测试 7: 空缓冲区
TEST(test_buffer_empty) {
    Buffer buf;

    assert(buf.readableBytes() == 0);

    // 读取空缓冲区
    std::string result = buf.retrieveAllAsString();
    assert(result.empty());

    // peek 空缓冲区
    const char* data = buf.peek();
    assert(data != nullptr);  // 指针有效但没有数据
}

// 测试 8: retrieveAsString 部分读取
TEST(test_buffer_retrieve_partial) {
    Buffer buf;

    buf.append("hello world", 11);

    std::string part1 = buf.retrieveAsString(5);
    assert(part1 == "hello");
    assert(buf.readableBytes() == 6);

    std::string part2 = buf.retrieveAsString(6);
    assert(part2 == " world");
    assert(buf.readableBytes() == 0);
}

// 测试 9: 大数据从 fd 读取 (测试 readv 的第二个缓冲区)
TEST(test_buffer_readfd_large) {
    Buffer buf;

    int pipefd[2];
    assert(pipe(pipefd) == 0);

    // 写入大量数据 (100KB)
    std::string largeData(100 * 1024, 'x');

    // 分批写入 (避免 pipe 缓冲区满)
    size_t written = 0;
    while (written < largeData.size()) {
        ssize_t n = write(pipefd[1], largeData.data() + written,
                         std::min(largeData.size() - written, size_t(8192)));
        if (n > 0) {
            written += n;
        }

        // 同时读取，避免阻塞
        if (written > 0) {
            int savedErrno = 0;
            buf.readFd(pipefd[0], &savedErrno);
        }
    }

    // 继续读取剩余数据
    while (buf.readableBytes() < largeData.size()) {
        int savedErrno = 0;
        ssize_t n = buf.readFd(pipefd[0], &savedErrno);
        if (n == 0) break;  // EOF
        if (n < 0 && savedErrno == EAGAIN) break;
    }

    assert(buf.readableBytes() == largeData.size());

    std::string result = buf.retrieveAllAsString();
    assert(result == largeData);

    close(pipefd[0]);
    close(pipefd[1]);
}

int main() {
    RUN_TEST(test_buffer_append_retrieve);
    RUN_TEST(test_buffer_growth);
    RUN_TEST(test_buffer_prepend_space);
    RUN_TEST(test_buffer_compact);
    RUN_TEST(test_buffer_readfd);
    RUN_TEST(test_buffer_peek);
    RUN_TEST(test_buffer_empty);
    RUN_TEST(test_buffer_retrieve_partial);
    RUN_TEST(test_buffer_readfd_large);

    std::cout << "\n=== All Buffer Tests Passed ===" << std::endl;
    return 0;
}
