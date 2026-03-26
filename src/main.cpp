#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>

#define HANDLE_INT_RESULT(result)                                                                                      \
    {                                                                                                                  \
        if (result == -1)                                                                                              \
        {                                                                                                              \
            char buf[512] = {};                                                                                        \
            sprintf(buf, "%s:%d:", __FILE__, __LINE__ - 5);                                                            \
            perror(buf);                                                                                               \
            exit(1);                                                                                                   \
        }                                                                                                              \
    }

int main()
{
    int listener_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(listener_socket);

    int rtsp_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(listener_socket);

    int rtp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    HANDLE_INT_RESULT(listener_socket);

    int rtcp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    HANDLE_INT_RESULT(listener_socket);
}
