#ifndef __NET_ACCEPTOR_H__
#define __NET_ACCEPTOR_H__

#include "NetParam.h"
#include "TCPSocket.h"
#include "EventLoop.h"
#include "utility/LoggerMessage.h"
#include "TcpConnection.h"

namespace net
{

#if 1
    class Acceptor final
    {
    public:
        using FilterFuncType = std::function<bool(const socket::SocketAddr &peerAddr)>;
        using ConnectionFuncType = std::function<void(int connfd, const socket::SocketAddr &peerAddr)>;

    public:
        Acceptor(const socket::SocketAddr &listenAddr, const std::shared_ptr<event::EventLoop> &eventLoop)
            : mLocalAddr(listenAddr),
              mEventLoop(eventLoop),
              mListenSocket(socket::createTCPSocket())
        {
            mListenHandlerPtr = std::make_shared<event::EventHandler>(mListenSocket.getFD());
            mListenHandlerPtr->setReadFunc(std::bind(&Acceptor::connFunc, this));
            mListenHandlerPtr->enableReading();

            mEventLoop->registerEventHandler(mListenHandlerPtr);
            mEventLoop->update(mListenHandlerPtr);

            mListenSocket.setResueAddr(true);
            mListenSocket.setReusePort(true);
            mListenSocket.bind(listenAddr);
        }
        ~Acceptor()
        {
        }

        const int getFD() const { return mListenSocket.getFD(); }

        const socket::SocketAddr getLocalAddr() const { return mLocalAddr; }

        void setVistorsDest(const std::shared_ptr<std::queue<std::pair<int, socket::SocketAddr>>> &visitorsQueuePtr) { mVisitorsPtr = visitorsQueuePtr; }

        void setFilterFunc(const FilterFuncType &func) { mFilterFunc = std::move(func); }

        void setConnectionFunc(const ConnectionFuncType &func) { mConnectionFunc = std::move(func); }

        void listen()
        {
            mListenSocket.listen();
        }

        void run()
        {
            mEventLoop->loop();
        }

    private:
        void connFunc()
        {
            socket::SocketAddr clisa;
            auto connfd = mListenSocket.accept(clisa);
            // std::cout << "connfd = " << connfd << " address: " << clisa.getHostAndServiceNUM() << std::endl;
            if (connfd > 0)
            {
                auto ptr = mVisitorsPtr.lock();
                if (ptr)
                {
                    ptr->push(std::make_pair(connfd, clisa));
                }
                if (mConnectionFunc)
                {
                    if (mFilterFunc)
                    {
                        if (mFilterFunc(clisa))
                        {
                            mConnectionFunc(connfd, clisa);
                        }
                        else
                        {
                            // will trigger 4-way handshake and TIME_WAIT, but the blacklist should be short commonly
                            // or use SO_LINGER reset
                            ::close(connfd);
                            LOG_INFO << clisa.getHostAndServiceNUM() << " is forbidden to access the server and close the fd " << connfd;
                        }
                        return;
                    }

                    mConnectionFunc(connfd, clisa);
                }
            }
        }

    private:
        socket::SocketAddr mLocalAddr;

        std::shared_ptr<event::EventLoop> mEventLoop;

        socket::TCPSocket mListenSocket;

        std::shared_ptr<event::EventHandler> mListenHandlerPtr;

        FilterFuncType mFilterFunc;
        ConnectionFuncType mConnectionFunc;

        std::weak_ptr<std::queue<std::pair<int, socket::SocketAddr>>> mVisitorsPtr;
    };

#endif

#if 0

    class Acceptor final
    {
    public:
        using ConnectionFuncType = std::function<void(int connfd, const socket::SocketAddr &peerAddr)>;

    public:
        Acceptor(const socket::SocketAddr &listenAddr, PollerType which)
            : mLocalAddr(listenAddr),
              mListenSocket(socket::createTCPSocket())
        {
            if (which == PollerType::EPOLL_POLLER)
                mPoller = std::make_unique<event::EpollPoller>();
            else if (which == PollerType::POLL_POLLER)
                mPoller = std::make_unique<event::PollPoller>();
            else
                mPoller = std::make_unique<event::EpollPoller>();

            mListenHandlerPtr = std::make_shared<event::EventHandler>(mListenSocket.getFD());
            // mListenHandlerPtr->setReadFunc(std::bind(&Acceptor::testFunc, this));
            mListenHandlerPtr->setReadFunc(std::bind(&Acceptor::connFunc, this));
            mListenHandlerPtr->enableReading();
            mPoller->registerEventHandler(mListenHandlerPtr);
            mPoller->update(mListenHandlerPtr);

            mListenSocket.setResueAddr(true);
            mListenSocket.setReusePort(true);
            mListenSocket.bind(listenAddr);
        }
        ~Acceptor()
        {
        }

        const int getFD() const { return mListenSocket.getFD(); }

        const socket::SocketAddr getLocalAddr() const { return mLocalAddr; }

        void getVistorsDest(const std::shared_ptr<std::queue<std::pair<int, socket::SocketAddr>>> &visitorsQueuePtr) { mVisitorsPtr = visitorsQueuePtr; }

        void setConnectionFunc(const ConnectionFuncType &func)
        {
            mConnectionFunc = std::move(func);
        }

        void listen()
        {
            mListenSocket.listen();
        }

        void run()
        {
            while (true)
            {
                std::vector<std::shared_ptr<net::event::EventHandler>> eventHandlers;

                mPoller->poll(-1, eventHandlers);

                for (auto &eh : eventHandlers)
                {
                    eh->doEvnet();
                }
            }
        }

    private:
        void testFunc()
        {
            socket::SocketAddr clisa;
            auto connfd = mListenSocket.accept(clisa);
            std::cout << "connfd = " << connfd << " address: " << clisa.getHostAndServiceNUM() << std::endl;
            if (connfd >= 0)
            {
            }
            else
            {
                ::close(connfd);
            }
        }
        void connFunc()
        {
            socket::SocketAddr clisa;
            auto connfd = mListenSocket.accept(clisa);
            std::cout << "connfd = " << connfd << " address: " << clisa.getHostAndServiceNUM() << std::endl;
            if (connfd > 0)
            {
                auto ptr = mVisitorsPtr.lock();
                if (ptr)
                {
                    // std::cout << "push new visitor\n";
                    ptr->push(std::make_pair(connfd, clisa));
                }
                if (mConnectionFunc)
                {
                    mConnectionFunc(connfd, clisa);
                }
            }
        }

    private:
        socket::TCPSocket mListenSocket;
        socket::SocketAddr mLocalAddr;
        std::shared_ptr<event::EventHandler> mListenHandlerPtr;
        std::unique_ptr<event::Poller> mPoller;

        ConnectionFuncType mConnectionFunc;

        std::weak_ptr<std::queue<std::pair<int, socket::SocketAddr>>> mVisitorsPtr;
    };

#endif

}

#endif