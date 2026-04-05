#include <signal.h>

#include "Server/Server.h"

// Signal Handling
extern "C"
{
    void interrupt_handler(int signal_number)
    {
        // swallow
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

    Server server;

    server.listen_and_serve();
}
