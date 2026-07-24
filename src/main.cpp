#include <signal.h>

#include <iostream>
#include <thread>

#include "Server/Server.h"
#include "Server/Session.h"

// Global Server
Server server;

// Signal Handling
extern "C"
{
    void interrupt_handler(int signal_number)
    {
        server.interrupt();
    }
}

void set_up_signal_handlers()
{
    struct sigaction input{};
    input.sa_handler = interrupt_handler;

    struct sigaction output{};

    HANDLE_INT_RESULT(sigaction(SIGINT, &input, &output));
}

int main()
{
    set_up_signal_handlers();

    int session_workers = 2;
    int server_workers = std::thread::hardware_concurrency() - 1 - session_workers;

    if (server_workers < 1)
    {
        session_workers = 1;
        server_workers = 0;
    }

    std::vector<std::jthread> thread_pool;
    thread_pool.reserve(session_workers + server_workers);

    // Manage sessions
    for (int i = 0; i < session_workers; ++i)
    {
        thread_pool.emplace_back([]() { Session::watch_streams(); });
    }

    // Start server
    server.listen();
    for (int i = 0; i < server_workers; ++i)
    {
        thread_pool.emplace_back([&]() { server.serve(); });
    }
    server.serve();

    std::cout << "Shutting down gracefully.\n";
}
