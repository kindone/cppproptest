#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "fork.hpp"

class ForkTestCase : public ::testing::Test {
};

struct A
{
    int a;
    int b;
};

using namespace PropertyBasedTesting;

TEST(ForkTestCase, SafeCall1)
{
    auto result = util::safeCall<int>([]() { return 5; });

    EXPECT_EQ(result, 5);
}

TEST(ForkTestCase, SafeCall2)
{
    EXPECT_ANY_THROW(util::safeCall<int>([]() {
        throw std::runtime_error("error!");
        return 4;
    }));
}

TEST(ForkTestCase, SafeCall3)
{
    EXPECT_ANY_THROW(util::safeCall<int>([]() {
        int* a = nullptr;
        *a = 5;
        return *a;
    }));
}

TEST(ForkTestCase, Fork)
{
    printf("--beginning of test--\n");
    int counter = 0;
    int ptoc[2];
    int ctop[2];

    if (pipe(ptoc) == -1) {
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }
    if (pipe(ctop) == -1) {
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        // child process
        close(ptoc[1]);
        close(ctop[0]);

        printf("child process: counter=%d\n", ++counter);

        char message_from_parent[100];
        int len = read(ptoc[0], message_from_parent, 100);
        if (len > 0)
            printf("message from parent: %s\n", message_from_parent);
        close(ptoc[0]);

        if (0 >= write(ctop[1], "hello from child", 20))
            throw std::runtime_error("write error");

        int* a = nullptr;
        printf("a: %d\n", *a);  // kill
        close(ctop[1]);
    } else if (pid > 0) {
        // parent process
        close(ptoc[0]);
        close(ctop[1]);

        if (0 >= write(ptoc[1], "hello from parent", 20))
            throw std::runtime_error("write error");
        close(ptoc[1]);

        printf("parent process: counter=%d\n", ++counter);

        int state = 0;
        pid_t got_pid = waitpid(pid, &state, 0);
        printf("got_pid=%d\n", got_pid);                  // 2
        printf("WIFEXITED: %d\n", WIFEXITED(state));      // 3
        printf("WEXITSTATUS: %d\n", WEXITSTATUS(state));  // 4
        printf("Done from parent\n");

        char message_from_child[100];
        int len = read(ctop[0], message_from_child, 100);
        if (len > 0)
            printf("message from child: %s\n", message_from_child);
        close(ctop[0]);

    } else {
        // fork failed
        printf("fork() failed!\n");
    }

    printf("--end of test--\n");
}