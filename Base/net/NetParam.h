#ifndef __NET_PARAMETER_H__
#define __NET_PARAMETER_H__

#include <cstddef>

namespace net
{
    enum class PollerType
    {
        POLL_POLLER,
        EPOLL_POLLER
    };

    constexpr size_t IO_BUFFER_INITIAL_SIZE() { return 1024; }
    constexpr size_t IO_BUFFER_MAX_SIZE() { return 1024 * 64; }
    constexpr unsigned short IO_BUFFER_DECAY_LIMIT() { return 5; }

    constexpr int POLL_TIMEOUT_MS() { return 5000; }
}

#endif