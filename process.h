#pragma once

#include "wakeup.h"

#include <chrono>
#include <span>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>

class ChildProcess {
  public:
#ifdef __APPLE__
    static constexpr int send_flags = 0; // SO_NOSIGPIPE is set on the socket.
#else
    static constexpr int send_flags = MSG_NOSIGNAL;
#endif
    ChildProcess() = default;
    ChildProcess(std::span<const std::string> arguments, const std::string& role,
                 const std::string& error_log = {});
    ChildProcess(ChildProcess&& other) noexcept
        : socket(std::move(other.socket)), child(std::exchange(other.child, -1))
    {
    }
    ChildProcess& operator=(ChildProcess&&) noexcept;
    ~ChildProcess();
    [[nodiscard]] int stream() const { return socket.get(); }
    void stop(std::chrono::milliseconds patience);

  private:
    FileDescriptor socket;
    pid_t child = -1;
};
