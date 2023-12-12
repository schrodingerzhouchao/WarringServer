#ifndef __TEST_TCP_CLIENT_H__
#define __TEST_TCP_CLIENT_H__

#include "net/TCPSocket.h"
#include "net/IOBuffer.h"
#include "utility/Timer.h"
#include "utility/Logger.h"
#include "utility/LoggerMessage.h"

#include <stdlib.h>
#include <sys/wait.h>

#include <vector>
#include <thread>
#include <iostream>


#include "message/CustomMessages.h"

using namespace utility;
using namespace net::socket;
using namespace std;
using namespace message;

class Client
{
public:
    Client()
    {
        mSockfd = createTCPSocket();
        TCPSocket tcpsock(mSockfd);
        SocketAddr sa("192.168.119.128", 50005);
        tcpsock.connect(sa);
        tcpsock.setResueAddr(true);
    }

    ~Client()
    {
        ::close(mSockfd);
    }

    void echoFunc(int index)
    {
        std::string str = "this is a message abcdefg hijklmn opq rst uvw xyz" + std::to_string(index);
        size_t len = str.size();

        auto n = ::write(mSockfd, str.c_str(), len);
        if (n > 0)
        {
            LOG_INFO << "send data to server: " << str.substr(0, n);
        }
        else
        {
            LOG_ERROR << "send data to server error";
        }

        char buf[1024] = {0};

        n = ::read(mSockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << std::string(buf, n);
        }
        else
        {
            LOG_INFO << "get data from server code: " << std::to_string(n);
        }
    }

    void messageFunc(int index)
    {
        //LoginMessage loginMsg;
        //loginMsg.MSG_NAME = {0};
    }

private:
    int mSockfd = -1;
};

#endif