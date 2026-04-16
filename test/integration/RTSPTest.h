#include <gtest/gtest.h>

#define HANDLE_INT_RESULT(result)                                                                                      \
    {                                                                                                                  \
        if (result == -1)                                                                                              \
        {                                                                                                              \
            char buf[512] = {};                                                                                        \
            sprintf(buf, "%s:%d", __FILE__, __LINE__);                                                                 \
            perror(buf);                                                                                               \
            exit(EXIT_FAILURE);                                                                                        \
        }                                                                                                              \
    }

class RTSPTest : public testing::Test
{
  protected:
    RTSPTest();
    ~RTSPTest();

    int get_fd() const;

    int receive(char *buffer, size_t buffer_size);

  private:
    int fd = 0, child_pid = 0;

    void start_server();
};
