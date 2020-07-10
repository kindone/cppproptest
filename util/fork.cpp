#include "fork.hpp"

namespace proptest {
namespace util {

Pipe::Pipe()
{
    fd[0] = -1;
    fd[1] = -1;
    if (pipe(fd) == -1)
        throw std::runtime_error("could not initialize pipe");
}

Pipe::~Pipe()
{
    close();
}

size_t Pipe::read(void* buf, size_t size)
{
    return ::read(fd[0], buf, size);
}

size_t Pipe::write(void* buf, size_t size)
{
    return ::write(fd[1], buf, size);
}

void Pipe::close()
{
    if (fd[0] != -1)
        ::close(fd[0]);
    if (fd[1] != -1)
        ::close(fd[1]);
    fd[0] = -1;
    fd[1] = -1;
}

Fork::Fork()
{
    pid = fork();
    if (pid < 0)
        throw std::runtime_error("unable to fork for safeCall, pid = " + std::to_string(pid));
}

void Fork::exitNormal()
{
    if (pid != 0)
        throw std::runtime_error("attempt to exit on non-child process");
    exit(0);
}

void Fork::exitAbnormal()
{
    if (pid != 0)
        throw std::runtime_error("attempt to exit on non-child process");
    exit(-1);
}

}  // namespace util
}  // namespace proptest
