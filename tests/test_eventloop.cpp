#include "../include/EventLoop.h"
#include "../include/Channel.h"
#include "../include/Socket.h"
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "..."; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

TEST(create_eventloop) {
    EventLoop loop;
}

TEST(read_event) {
    EventLoop loop;

    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret == 0);

    Channel channel(&loop, pipefd[0]);

    bool eventsHandled = false;
    channel.setReadCallback([&](){
        eventsHandled = true;

        // 读数据
        char buf[16];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));

        loop.quit();
    });

    channel.enableReading();

    write(pipefd[1], "test", 4);

    loop.loop();

    assert(eventsHandled);

    close(pipefd[0]);
    close(pipefd[1]);

}

TEST(multiple_channels){
    EventLoop loop;

    int pipe1[2], pipe2[2];
    assert(pipe(pipe1)==0);
    assert(pipe(pipe2)==0);

    Channel channel1(&loop, pipe1[0]);
    Channel channel2(&loop, pipe2[0]);

    int eventsHandled = 0;

    channel1.setReadCallback([&](){
        eventsHandled++;
        char buf[16];
        read(pipe1[0], buf, sizeof(buf));
        if(eventsHandled == 2) {
            loop.quit();
        }
    });

    channel2.setReadCallback([&](){
        eventsHandled++;
        char buf[16];
        read(pipe2[0], buf, sizeof(buf));
        if(eventsHandled == 2) {
            loop.quit();
        }
    });

    channel1.enableReading();
    channel2.enableReading();

    write(pipe1[1], "test1", 5);
    write(pipe2[1], "test2", 5);

    loop.loop();

    assert(eventsHandled == 2);

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

}

TEST(remove_channel){
    EventLoop loop;

    int pipefd[2];
    assert(pipe(pipefd) == 0);

    Channel channel(&loop, pipefd[0]);

    channel.setReadCallback([&](){
        assert(false);
    });

    channel.enableReading();
    channel.remove();

    write(pipefd[1], "test", 4);

    int pipe2[2];
    assert(pipe(pipe2) == 0);
    Channel channel2(&loop, pipe2[0]);
    channel2.setReadCallback([&](){
        char buf[16];
        read(pipe2[0], buf, sizeof(buf));
        loop.quit();
    });
    write(pipe2[1], "quit", 4);

    loop.loop();

    close(pipefd[0]);
    close(pipefd[1]);
    close(pipe2[0]);
    close(pipe2[1]);

}



int main() {
    std::cout << "=== EventLoop Tests ===" << std::endl;
    RUN_TEST(create_eventloop);
    RUN_TEST(read_event);
    RUN_TEST(multiple_channels);

    std::cout << "\nALL tests passed!" << std::endl;
    return 0;
}