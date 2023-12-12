
/*
 *
 *
 */

#ifndef __NET_TCP_SOCKET_H__
#define __NET_TCP_SOCKET_H__

#include "Socket.h"
#include "SocketAddr.h"
#include <netinet/tcp.h>

namespace net
{
    namespace socket
    {
        class TCPSocket : public Socket
        {
        public:
            explicit TCPSocket(int fd) : Socket(fd)
            {
            }
            TCPSocket(int fd, bool nonBlocking) : Socket(fd)
            {
                if (nonBlocking)
                    setNonBlackAndCloseOnExec(mFD);
            }

            virtual ~TCPSocket() = default;

            void setTcpNoDelay(bool on)
            {
                int opt = on ? 1 : 0;
                if (::setsockopt(mFD, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
                {
                    ; // TODO ERROR
                }
            }

            void setLinger(int on, int lingerTime)
            {
                linger l;
                l.l_onoff = on;
                l.l_linger = lingerTime;
                if (::setsockopt(mFD, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0)
                {
                    ; // TODO ERROR
                }
            }

            void shutdownW()
            {
                if (::shutdown(mFD, SHUT_WR) < 0)
                {
                    ; // TODO ERROR
                }
            }

            int connect(const SocketAddr &addr)
            {
                auto addrin = addr.getAddrin();
                return ::connect(mFD, (sockaddr *)&addrin, sizeof(sockaddr));
            }

            int bind(const SocketAddr &addr)
            {
                auto addrin = addr.getAddrin();
                return ::bind(mFD, (sockaddr *)&addrin, sizeof(sockaddr));
            }

            int listen()
            {
                return ::listen(mFD, SOMAXCONN);
            }

            int accept(SocketAddr &addr) const
            {
                sockaddr_in addrin;
                socklen_t addrlen = sizeof(addrin);
                auto connfd = ::accept(mFD, (sockaddr *)&addrin, &addrlen);
                addr.setAddrin(addrin);
                return connfd;
            }

        private:
        };
    }
}

#endif