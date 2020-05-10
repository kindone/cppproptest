#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>
#include <functional>
#include <iostream>

#include "../api.hpp"

namespace PropertyBasedTesting {
namespace util {

template <typename RET>
struct PipeResult
{
    PipeResult() : success(false), size(0) {}
    PipeResult(const RET& ret) : success(true), size(sizeof(ret)), ret(ret) {}
    PipeResult(const std::string& error) : success(false), size(error.size()), msg(error) {}

    void* data()
    {
        if (success)
            return &ret;
        else
            return msg.c_str();
    }

    size_t dataLength() { return size; }

    bool success;
    size_t size;
    RET ret;
    std::string msg;
};

struct PROPTEST_API Pipe
{
    Pipe();
    ~Pipe();
    size_t read(void* buf, size_t size);
    size_t write(void* buf, size_t size);

    template <typename RET>
    size_t read(PipeResult<RET>& result)
    {
        read(&result.success, sizeof(result.success));
        read(&result.size, sizeof(result.size));
        if (result.success) {
            read(&result.ret, sizeof(result.ret));
        } else {
            char buf[100];
            read(buf, result.size);
            result.msg = std::string(buf, result.size);
        }

        return sizeof(result.success) + result.size;
    }
    template <typename RET>
    size_t write(PipeResult<RET>& result)
    {
        write(&result.success, sizeof(result.success));
        write(&result.size, sizeof(result.size));
        if (result.success) {
            write(&result.ret, sizeof(result.ret));
        } else
            write(const_cast<char*>(result.msg.c_str()), result.msg.size());

        return sizeof(result.success) + result.size;
    }

    void close();

private:
    int fd[2];
};

struct PROPTEST_API Fork
{
    Fork();

    bool isParent() const { return pid > 0; }
    bool isChild() const { return pid == 0; }
    pid_t getPid() const { return pid; }

    void exitNormal();
    void exitAbnormal();

private:
    pid_t pid;
};

template <typename RET>
RET safeCall(std::function<RET()> func)
{
    // prepare pipe for communication
    Pipe pipe;

    // fork here
    Fork forked;

    if (forked.isChild()) {
        try {
            RET result = func();
            PipeResult<RET> pipeResult(result);
            pipe.write<RET>(pipeResult);
            pipe.close();
            forked.exitNormal();
            return result;  // unreachable
        } catch (std::exception& e) {
            PipeResult<RET> pipeResult(std::string(e.what()));
            pipe.write<RET>(pipeResult);
            pipe.close();
            forked.exitAbnormal();
            throw;  // unreachable
        }
    }
    // parent
    else {
        int state = 0;
        pid_t got_pid = waitpid(forked.getPid(), &state, 0);
        RET result;
        std::cout << "got pid: " << got_pid << ", exited: " << WIFEXITED(state)
                  << ", exitstatus: " << WEXITSTATUS(state) << ", signaled: " << WIFSIGNALED(state)
                  << ", stopped: " << WIFSTOPPED(state) << ", termsig: " << WTERMSIG(state) << std::endl;
        if (WIFEXITED(state) == 0 /* || WEXITSTATUS(state) != 0*/) {
            // std::cerr << "forked process ended with error: state = " << state << std::endl;
            throw std::runtime_error("forked process ended with error: state = " + std::to_string(state));
        } else {
            PipeResult<RET> pipeResult;
            pipe.read(pipeResult);
            if (pipeResult.success)
                return pipeResult.ret;
            else
                throw std::runtime_error("forked process has thrown an exception: " + pipeResult.msg);
        }
    }
}

}  // namespace util
}  // namespace PropertyBasedTesting