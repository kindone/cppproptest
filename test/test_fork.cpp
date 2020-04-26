#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>

class ForkTestCase : public ::testing::Test {
};

struct A {
    int a;
    int b;
};

template <typename RET>
RET safeCall(std::function<RET()> func) {
    // prepare pipe for communication
    int ctop[2];
    if (pipe(ctop)==-1)
        throw std::runtime_error("cannot initialize pipe");

    // fork here
    pid_t pid = fork();

    // fork failed
    if (pid < 0)
    {
        throw std::runtime_error("unable to fork");
    }
    // child
    else if (pid == 0)
    {
        try {
            auto result = func();
            write(ctop[1], &result, sizeof(result));
            close(ctop[0]);
            close(ctop[1]);
            exit(0);
            return result;
        }
        catch(...) {
            close(ctop[0]);
            close(ctop[1]);
            exit(-1);
        }
    }
    // parent
    else
    {
        int state = 0;
        pid_t got_pid = waitpid(pid, &state, 0);
        RET result;
        printf("WIFEXITED: %d\n", WIFEXITED(state));  // 3
        printf("WEXITSTATUS: %d\n", WEXITSTATUS(state)); // 4
        printf("WIFSIGNALED: %d\n", WIFSIGNALED(state)); // 5
        printf("WIFSTOPPED: %d\n", WIFSTOPPED(state)); // 6
        printf("WTERMSIG: %d\n", WTERMSIG(state));
        if(WIFEXITED(state) == 0 || WEXITSTATUS(state) != 0) {
            std::cerr << "forked process ended with error" << std::endl;
        }
        else {
            std::cout << "reading:" << std::endl;
            read(ctop[0], &result, sizeof(result));
        }
        close(ctop[1]);
        close(ctop[0]);
        std::cout << "forked process ended: " << state << std::endl;
        return result;
    }
}

// template <typename RET, typename ...ARGS>
// RET safeCall(std::function<RET(ARGS...)> func, ARGS&&...args) {

// }

TEST(ForkTestCase, SafeCall) {
    int result = safeCall<int>([]() {
        return 5;
    });

    std::cout << "safe call result: " << result << std::endl;

    result = safeCall<int>([]() {
        throw std::runtime_error("error!");
        return 4;
    });

    std::cout << "safe call result: " << result << std::endl;

    result = safeCall<int>([]() {
        int* a = nullptr;
        *a = 5;
        return *a;
    });

    std::cout << "safe call result: " << result << std::endl;
}

TEST(ForkTestCase, Fork)
{
    printf("--beginning of program--\n");
    int counter = 0;
    int ptoc[2];
    int ctop[2];

    if (pipe(ptoc)==-1)
    {
        fprintf(stderr, "Pipe Failed" );
        exit(1);
    }
    if (pipe(ctop)==-1)
    {
        fprintf(stderr, "Pipe Failed" );
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        // child process
        close(ptoc[1]);
        close(ctop[0]);

        printf("child process: counter=%d\n", ++counter);

        char message_from_parent[100];
        int len = read(ptoc[0], message_from_parent, 100);
                if(len > 0)
            printf("message from parent: %s\n", message_from_parent);
        close(ptoc[0]);

        write(ctop[1], "hello from child", 20);

        int *a = nullptr;
        printf("a: %d\n", *a);//kill
        close(ctop[1]);
    }
    else if (pid > 0)
    {
        // parent process
        close(ptoc[0]);
        close(ctop[1]);

        write(ptoc[1], "hello from parent", 20);
        close(ptoc[1]);

        printf("parent process: counter=%d\n", ++counter);

        int state = 0;
        pid_t got_pid = waitpid(pid, &state, 0);
        printf("got_pid=%d\n",  got_pid);   // 2
        printf("WIFEXITED: %d\n", WIFEXITED(state));  // 3
        printf("WEXITSTATUS: %d\n", WEXITSTATUS(state)); // 4
        printf("Done from parent\n");

        char message_from_child[100];
        int len = read(ctop[0], message_from_child, 100);
        if(len > 0)
            printf("message from child: %s\n", message_from_child);
        close(ctop[0]);

    }
    else
    {
        // fork failed
                printf("fork() failed!\n");
        exit(1);
    }

    printf("--end of program--\n");

    exit(0);
}