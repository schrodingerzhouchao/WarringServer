#include "Socket.h"

namespace net
{
    namespace socket
    {
        int createTCPSocket()
        {
            return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        }

        int createUDPSocket()
        {
            return ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        }

        void setNonBlackAndCloseOnExec(int sockfd)
        {
            auto flags = ::fcntl(sockfd, F_GETFL, 0);
            flags |= O_NONBLOCK;
            auto res = ::fcntl(sockfd, F_SETFL, flags);

            flags = ::fcntl(sockfd, F_GETFD, 0);
            flags |= FD_CLOEXEC;
            res = ::fcntl(sockfd, F_SETFD, flags);
        }

        bool isNonBlocking(int sockfd)
        {
            auto flags = ::fcntl(sockfd, F_GETFL, 0);
            if (flags & O_NONBLOCK)
                return true;
            return false;
        }

        int createNonblockingTCPSocket()
        {
            return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        }
        int createNonblockingUDPSocket()
        {
            return ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
        }
    }
}