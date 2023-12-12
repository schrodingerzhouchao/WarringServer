#include "net/TcpServer.h"
#include "utility/Logger.h"
#include "utility/LoggerMessage.h"

#include <thread>
#include <limits>

#include "message/CustomMessages.h"

#include <sw/redis++/redis++.h>

using namespace std;
using namespace net;
using namespace utility;
using namespace message;


const size_t NEEDED_LENGTH = 52;

class Echo
{
public:
    void echoFunc(const std::shared_ptr<net::TcpConnection> &tcpConnPtr)
    {
        // auto msg = tcpConnPtr->tryGet(NEEDED_LENGTH, true);

        auto msg = tcpConnPtr->get();

        if (!msg.empty())
        {
            LOG_INFO << "[echoFunc] get message from " << tcpConnPtr->getIPPort() << " size = " << msg.length() << " message: " << msg;
            // tcpConnPtr->send(msg);

            tcpConnPtr->putToBuffer(msg);
        }
    }

private:
    // thread_local static std::string xyz;
};

// thread_local std::string Echo::xyz = {};

class RegisterAndLogin
{
public:
    RegisterAndLogin()
        : mRedis("tcp://127.0.0.1:6379")
    {
        sw::redis::ConnectionOptions connectionOptions;
        sw::redis::ConnectionPoolOptions poolOptions;

        mConnOptions.host = "127.0.0.1";
        mConnOptions.port = 6379;
        mConnOptions.socket_timeout = std::chrono::milliseconds(200);

        mConnPoolOptions.size = 10;
        mConnPoolOptions.wait_timeout = std::chrono::milliseconds(100);

        sw::redis::Redis redis(mConnOptions, mConnPoolOptions);

        mRedis = std::move(redis);

        std::cout << "construction\n";
    }
    ~RegisterAndLogin()
    {
        std::cout << "deconstruction\n";
    }

    void func(const std::shared_ptr<net::TcpConnection> &tcpConnPtr, net::IOBuffer *IOBuffPtr)
    {
        auto msg = IOBuffPtr->get();

        if (msg)
        {
        }
        else
        {
            std::cout << "[func] get nothing message\n";
        }
    }

private:
    // MessageLogin mLoginMsg;

    thread_local static BaseMessage mBaseMsg;

    thread_local static std::string mData;

    sw::redis::Redis mRedis;

    sw::redis::ConnectionOptions mConnOptions;
    sw::redis::ConnectionPoolOptions mConnPoolOptions;
};

thread_local std::string RegisterAndLogin::mData = {};
thread_local BaseMessage RegisterAndLogin::mBaseMsg = {};

void print1()
{
    LOG_DEBUG << "Tcp server background task run per 1 second";
}

void print5()
{
    LOG_DEBUG << "Tcp server background task run per 5 second";
}

void printEventLoopThreadIDs(const std::vector<std::thread::id> &ids)
{
    for (auto id : ids)
    {
        LOG_DEBUG << "EventLoopThread's id: " << id;
    }
}

void printEventLoopThreadIDs2(const TcpServer &tcpServer, const std::vector<std::thread::id> &ids)
{
    for (auto id : ids)
    {
        LOG_DEBUG << "EventLoopThread's id: " << id;

        auto el = tcpServer.getEventLoop(id);
        if (el)
        {
            LOG_DEBUG << "EventLoopThread's id: " << id << " EventLoop size: " << el->getEventSize();
        }
        else
        {
            LOG_DEBUG << "EventLoopThread's id: " << id << " not found EventLoop SHOULDN'T ";
        }
    }
}

void benchLoop()
{
    size_t i = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        if (i == std::numeric_limits<size_t>::max())
            i = 0;

        LOG_DEBUG << "benchLoop(): " << ++i;
    }
}

int main(int argc, const char *argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: " << argv[0] << " port\n";
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[1]);

    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();
    LOGGERPTR->setStandbyQueueMax(8);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    // std::this_thread::sleep_for(std::chrono::seconds(1));

    net::socket::SocketAddr sa(port);
    TcpServer tcpServer(sa, 4, "EchoServer");

    LOG_INFO << sa.getHostAndServiceNUM();

    auto ids = tcpServer.getEventLoopThreadIDs();

    Echo echo;
    TcpConnection::messageFuncType f = bind(&Echo::echoFunc, &echo, std::placeholders::_1);

    // RegisterAndLogin rAndL;
    // TcpConnection::messageFuncType f = bind(&RegisterAndLogin::func, &rAndL, std::placeholders::_1, std::placeholders::_2);

    tcpServer.linkTimer(twTimerPtr);

    // tcpServer.registerTimerTask(0_hours, 0_minutes, 1_seconds, -1, print1);

    // tcpServer.registerTimerTask(0_hours, 0_minutes, 5_seconds, -1, print5);

    // tcpServer.registerTimerTask(0_hours, 0_minutes, 7_seconds, -1, std::bind(printEventLoopThreadIDs, std::ref(ids)));

    // tcpServer.registerTimerTask(0_hours, 0_minutes, 7_seconds, -1, std::bind(printEventLoopThreadIDs2, std::ref(tcpServer), std::ref(ids)));

    // tcpServer.setBenchTask(benchLoop);

    tcpServer.setMessageFunc(f);

    tcpServer.start();

    return 0;
}