/* ########################################### */
/* ########################################### */
/* ######不管写的是hit,还是shit,尽量备注########## */
/* #######不然过几天就不知道写的是什么了########### */
/* ########################################### */
/* ########################################### */

/*
 *
 *
 */

#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

#include <iostream>

#include "utility/LoggerMessage.h"

namespace net
{
    namespace socket
    {
        class Socket
        {
        public:
            explicit Socket(int sockfd) : mFD{sockfd}
            {
            }
            virtual ~Socket()
            {
                ::close(mFD);
                LOG_INFO << "close socket fd: " << mFD;
            }

            int getFD() const { return mFD; }

            void close()
            {
                ::close(mFD);
            }

            void setKeepAlive(bool on)
            {
                int opt = on ? 1 : 0;
                if (::setsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
                {
                    ; // TODO ERROR
                }
            }

            void setResueAddr(bool on)
            {
                int opt = on ? 1 : 0;
                if (::setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
                {
                    ; // TODO ERROR
                }
            }

            void setReusePort(bool on)
            {
                int opt = on ? 1 : 0;
                if (::setsockopt(mFD, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
                {
                    ; // TODO ERROR
                }
            }

            int getSocketError()
            {
                int opt;
                socklen_t optlen = static_cast<socklen_t>(sizeof(opt));

                if (::getsockopt(mFD, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0)
                {
                    return errno;
                }
                else
                {
                    return opt;
                }
            }

        protected:
            int mFD;
        };

        int createTCPSocket();
        int createUDPSocket();
        void setNonBlackAndCloseOnExec(int sockfd);
        bool isNonBlocking(int sockfd);
        int createNonblockingTCPSocket();
        int createNonblockingUDPSocket();
    }
}

#endif