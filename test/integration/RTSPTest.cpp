#include "RTSPTest.h"

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "../src/Server/Server.h"

bool do_sleep()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    return true;
}

RTSPTest::RTSPTest()
{
    start_server();

    fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(fd);

    struct sockaddr_in6 addr{};
    addr.sin6_port = htons(RTSP_PORT);
    addr.sin6_addr = in6addr_loopback;
    addr.sin6_family = AF_INET6;

    int connection;
    do
    {
        connection = connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    } while (connection == -1 && errno == ECONNREFUSED && do_sleep());

    HANDLE_INT_RESULT(connection);
}

RTSPTest::~RTSPTest()
{
    close(fd);

    HANDLE_INT_RESULT(kill(child_pid, SIGINT));

    int status = 0;
    HANDLE_INT_RESULT(waitpid(child_pid, &status, 0));

    if (!WIFEXITED(status))
    {
        std::cout << "Child process (" << child_pid << ") failed to exit normally.\n";
    }
}

int RTSPTest::get_fd() const
{
    return fd;
}

void RTSPTest::start_server()
{
    child_pid = fork();
    HANDLE_INT_RESULT(child_pid);

    if (!child_pid)
    {
        // child process execution path
        HANDLE_INT_RESULT(execl(BINARY_DIR "/RTP-Server", "RTP-Server", nullptr));
    }
}

int RTSPTest::receive(char *buffer, size_t buffer_size)
{
    struct pollfd config{};
    config.fd = fd;
    config.events = POLLIN;

    int event_count = poll(&config, 1, 1000);

    if (event_count < 1)
    {
        return event_count;
    }

    return recv(fd, buffer, buffer_size, 0);
}
