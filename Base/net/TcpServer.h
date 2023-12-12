#ifndef __NET_TCP_SERVER_H__
#define __NET_TCP_SERVER_H__

#include "EventLoopThreadpool.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "utility/SafeHashmap.h"
#include "utility/LoggerMessage.h"

#include <future>
#include <set>
#include <initializer_list>
#include <signal.h>

namespace net
{
    class IgnoreSigPipe
    {
    public:
        void operator()()
        {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };

    class TcpServer
    {
    public:
        TcpServer(const socket::SocketAddr &listenAddr, const size_t thdNum = 4, const std::string &servName = "", size_t bucketSize = 1031, bool noDelay = false)
            : mLocalAddr(listenAddr),
              mServName(servName),
              mNoDelay(noDelay),
              mTcpConnections(bucketSize),
              mTimerTasksThread(&TcpServer::timerWork, this)
        {
            IgnoreSigPipe();

            mAcceptorEventLoop = std::make_shared<event::EventLoop>(PollerType::EPOLL_POLLER);
            mAcceptor = std::make_unique<Acceptor>(listenAddr, mAcceptorEventLoop);

            mAcceptor->setConnectionFunc(std::bind(&TcpServer::newConnectionFunc, this, std::placeholders::_1, std::placeholders::_2));

            mEventLoopThreadpool = std::make_unique<event::EventLoopThreadpool>(thdNum, PollerType::EPOLL_POLLER);

            mEventLoopThdNum = mEventLoopThreadpool->getSize();
        }

        TcpServer(const TcpServer &) = delete;
        TcpServer &operator=(const TcpServer &) = delete;
        TcpServer(TcpServer &&) = delete;
        TcpServer &operator=(TcpServer &&) = delete;

        ~TcpServer()
        {
            if (mTimerTasksThread.joinable())
            {
                mTimerTasksThread.join();
            }

            for (auto &benchThdResult : mBenchThreadsResult)
            {
                benchThdResult.get();
            }
        }

        bool linkTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
        {
            mTimerWeak = timerPtr;
            return mTimerWeak.lock() != nullptr;
        }

        // params should satisfy the conditions: intervalSeconds << timeoutSeconds << resetThresholdSeconds
        // note: no checks in the implementation
        void setTimeoutParam(time_t intervalSeconds, time_t timeoutSeconds, time_t resetThresholdSeconds)
        {
            for (size_t i = 0; i < mEventLoopThdNum; ++i)
            {
                mEventLoopThreadpool->getEventLoop(i)->setTimeoutParam(intervalSeconds, timeoutSeconds, resetThresholdSeconds);
            }
        }

        bool registerTimerTask(size_t hours, size_t minutes, size_t seconds, int cyc, std::function<void()> func)
        {
            auto timerPtr = mTimerWeak.lock();
            if (timerPtr)
            {
                auto id = timerPtr->generateID();
                auto res = mTimerTasks.try_emplace(id, func);
                if (res.second)
                {
                    std::function<void()> f = std::bind(&TcpServer::pushTaskID, this, id);
                    utility::timer::TimerTask tt(hours, minutes, seconds, cyc, id, f);
                    timerPtr->addTask(tt);
                    return true;
                }
            }

            return false;
        }

        void setConnFilterFunc(const Acceptor::FilterFuncType &filterFunc) { mAcceptor->setFilterFunc(filterFunc); }

        void setConnNameFunc(const std::function<void(std::string)> &connNameFunc) { mConnNameFunc = std::move(connNameFunc); }

        void setMessageFunc(const TcpConnection::messageFuncType &msgFunc) { mMessageFunc = std::move(msgFunc); }

        void setCloseFunc(const std::function<void(std::string)> &closeFunc) { mCloseFunc = std::move(closeFunc); }

        // func should be a lang-time loop task
        void setBenchTask(std::function<void()> func)
        {
            auto res = std::async(std::launch::async, func);
            mBenchThreadsResult.push_back(std::move(res));
        }

        std::vector<std::thread::id> getEventLoopThreadIDs() const { return mEventLoopThreadpool->getEventLoopThreadIDs(); }

        std::shared_ptr<net::event::EventLoop> getEventLoop(const std::thread::id tid) const
        {
            return mEventLoopThreadpool->getEventLoop(tid);
        }

        bool getConnPtrRef(const std::string &connID, std::shared_ptr<net::TcpConnection> &connPtr)
        {
            return mTcpConnections.find(connID, connPtr);
        }

        size_t getActiveConnectionNum() const
        {
            size_t num = 0;
            for (size_t i = 0; i < mEventLoopThdNum; ++i)
            {
                if (mEventLoopThreadpool->getEventLoop(i))
                    num += mEventLoopThreadpool->getEventLoop(i)->getEventSize();
            }
            return num;
        }

        utility::SafeHashmap<std::string, std::shared_ptr<net::TcpConnection>> &getTcpConnectionsRef() { return mTcpConnections; }

        socket::SocketAddr &getLocalAddr() { return mLocalAddr; }

        std::string getIPPort() { return mLocalAddr.getHostAndServiceNUM(); }

        // thread-unsafe, call it when server init
        void addToBlacklist(std::initializer_list<std::string> ips) { mConnBlacklist.insert(std::begin(ips), std::end(ips)); }

        bool isInBlacklist(std::string ip) { return (mConnBlacklist.count(ip) != 0); }

        bool isStartCalled() const { return mStarted; }

        // shoule be called last
        void start()
        {
            mStarted = true;

            mAcceptor->listen();
            mAcceptor->run();
        }

    private:
        void newConnectionFunc(int connfd, const socket::SocketAddr &peerAddr)
        {
            auto index = mEventLoopThreadpool->getDesiredIndex(event::EventLoopThreadpool::DistributionType::ROUND_ROBIN);
            auto eventloopPtr = mEventLoopThreadpool->getEventLoop(index);
            if (eventloopPtr == nullptr)
                return;

            std::string connName = generateConnID(connfd, peerAddr);

            LOG_INFO << "get new conn: " << connName;
            auto tcpConnPtr = std::make_shared<TcpConnection>(connfd, mLocalAddr, peerAddr, eventloopPtr, connName, mNoDelay);
            tcpConnPtr->setName(connName);
            mTcpConnections.insert(connName, tcpConnPtr);

            if (mConnNameFunc)
            {
                mConnNameFunc(connName);
            }

            tcpConnPtr->setMessageFunc(mMessageFunc);
            tcpConnPtr->setCloseFunc(std::bind(&TcpServer::removeConnection, this, tcpConnPtr));
        }

        void removeConnection(std::shared_ptr<TcpConnection> &connPtr)
        {
            auto name = connPtr->getName();
            LOG_INFO << "erase conn: " << name;
            mTcpConnections.erase(name);
            if (mCloseFunc)
            {
                mCloseFunc(connPtr->getIPPort());
            }

            connPtr.reset();
            // LOG_DEBUG << "connPtr count: "<<connPtr.use_count();
        }

        std::string generateConnID(int connfd, const socket::SocketAddr &peerAddr)
        {
            return mServName + "_fd" + std::to_string(connfd) + "_" + peerAddr.getHostAndServiceNUM();
        }

        void timerWork()
        {
            while (true)
            {
                std::string id = {};

                {
                    std::unique_lock lock(mMutexOfTimerTasksThread);

                    mCondOfTimerTasksThread.wait(lock, [this]
                                                 { return !mTimeoutTaskIDQueue.empty(); });
                    if (!mTimeoutTaskIDQueue.empty())
                    {
                        id = mTimeoutTaskIDQueue.front();
                        mTimeoutTaskIDQueue.pop();
                    }
                }

                runTimerTask(id);
            }
        }

        void runTimerTask(const std::string &id)
        {
            auto it = mTimerTasks.find(id);
            if (it != mTimerTasks.end())
            {
                // LOG_DEBUG << "TcpServer::runTimerTask() get func and run it";
                it->second();
            }
        }

        void pushTaskID(std::string id)
        {
            std::unique_lock lock(mMutexOfTimerTasksThread);
            mTimeoutTaskIDQueue.push(id);
            mCondOfTimerTasksThread.notify_one();
        }

    private:
        socket::SocketAddr mLocalAddr;
        std::string mServName;
        bool mNoDelay;
        size_t mEventLoopThdNum = 4;
        std::shared_ptr<event::EventLoop> mAcceptorEventLoop;
        std::unique_ptr<Acceptor> mAcceptor;
        std::unique_ptr<event::EventLoopThreadpool> mEventLoopThreadpool;
        utility::SafeHashmap<std::string, std::shared_ptr<net::TcpConnection>> mTcpConnections;

        // invoke when new connection come in
        std::function<void(std::string)> mConnNameFunc;

        std::function<void(std::string)> mCloseFunc;

        TcpConnection::messageFuncType mMessageFunc;

        // save the timeout tasks'id
        std::queue<std::string> mTimeoutTaskIDQueue;
        std::unordered_map<std::string, std::function<void()>> mTimerTasks;

        // <ip>s of connection blacklist
        std::set<std::string> mConnBlacklist;

        std::thread mTimerTasksThread;
        std::mutex mMutexOfTimerTasksThread;
        std::condition_variable mCondOfTimerTasksThread;

        // bench tasks' results
        // these tasks are considered as lang-time loop taks, only call get() at destructor
        std::vector<std::future<void>> mBenchThreadsResult;

        std::weak_ptr<utility::timer::TimeWheelTimer> mTimerWeak;

        bool mStarted = false;
    };
}

#endif