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
        Session::shutdown();
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

    unsigned session_workers = 2, server_workers = std::thread::hardware_concurrency() - 1 - session_workers;

    if (server_workers < 1)
    {
        // minimum 2 threads total to listen/serve and watch sessions in parallel
        session_workers = 1;
        server_workers = 0;
    }

    Session::watch_streams(session_workers);

    std::vector<std::jthread> thread_pool;
    thread_pool.reserve(server_workers);

    // Start server
    server.listen();
    for (unsigned i = 0; i < server_workers; ++i)
    {
        thread_pool.emplace_back([&]() { server.serve(); });
    }
    server.serve();

    std::cout << "Shutting down gracefully.\n";
}
