
/*
 * IPv4 socket address for now
 *
 */

#ifndef __NET_SOCKET_ADDRESS_H__
#define __NET_SOCKET_ADDRESS_H__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <cstring>

namespace net
{
    namespace socket
    {
        class SocketAddr final
        {
        public:
            SocketAddr() {}
            SocketAddr(uint16_t port)
            {
                ::memset(&mAddr, 0, sizeof(mAddr));
                mAddr.sin_family = AF_INET;
                mAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
                mAddr.sin_port = ::htons(port);
            }
            SocketAddr(const std::string &ip, uint16_t port)
            {
                ::memset(&mAddr, 0, sizeof(mAddr));
                mAddr.sin_family = AF_INET;
                if (::inet_pton(AF_INET, ip.c_str(), &mAddr.sin_addr) <= 0)
                {
                    ; // TODO ERROR
                }
                mAddr.sin_port = ::htons(port);
            }

            uint16_t getPort() const
            {
                return ::ntohs(mAddr.sin_port);
            }

            std::string getIP() const
            {
                char buff[INET_ADDRSTRLEN];
                return ::inet_ntop(AF_INET, &mAddr.sin_addr, buff, INET_ADDRSTRLEN);
            }

            std::string getHostAndService() const
            {
                char host[NI_MAXHOST];
                char service[NI_MAXSERV];
                if (::getnameinfo((sockaddr *)&mAddr, sizeof(mAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
                {
                    return std::string(host) + ":" + std::string(service);
                }
                else
                {
                    return "UNKNOWN";
                }
            }

            std::string getHostAndServiceNUM() const
            {
                char host[NI_MAXHOST];
                char service[NI_MAXSERV];
                if (::getnameinfo((sockaddr *)&mAddr, sizeof(mAddr), host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) == 0)
                {
                    return std::string(host) + ":" + std::string(service);
                }
                else
                {
                    return "UNKNOWN";
                }
            }

            sockaddr_in getAddrin() const { return mAddr; }
            void setAddrin(const sockaddr_in &addrin) { mAddr = addrin; }

        private:
            sockaddr_in mAddr;
        };
    }
}

#endif