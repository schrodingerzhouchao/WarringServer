#ifndef __NET_TCP_CONNECTION_H__
#define __NET_TCP_CONNECTION_H__

#include "NetParam.h"
#include "TCPSocket.h"
#include "EventHandler.h"
#include "EventLoop.h"
#include "IOBuffer.h"

#include <memory>

namespace net
{
    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        enum class ConnectionState
        {
            Disconnected,
            Connecting,
            Connected,
            Disconnecting
        };

        using messageFuncType = std::function<void(const std::shared_ptr<TcpConnection> &)>;
        using closeFuncType = std::function<void(const std::shared_ptr<TcpConnection> &)>;

    public:
        TcpConnection(int connfd, const socket::SocketAddr &local, const socket::SocketAddr &peer,
                      const std::shared_ptr<event::EventLoop> &eventLoop, const std::string &name = "", bool noDelay = false)
            : mConnectionSocket(connfd, true),
              mLocalAddr(local),
              mPeerAddr(peer),
              mEventLoop(eventLoop),
              mName(name),
              mInputBuffer(connfd),
              mOutputBuffer(connfd),
              mState(ConnectionState::Connected)
        {

            mConnectionSocket.setKeepAlive(true);
            mConnectionSocket.setTcpNoDelay(noDelay);

            mConnectionHandlerPtr = std::make_shared<event::EventHandler>(connfd);
            mConnectionHandlerPtr->setReadFunc(std::bind(&TcpConnection::readFunc, this));
            mConnectionHandlerPtr->setWriteFunc(std::bind(&TcpConnection::writeFunc, this));
            mConnectionHandlerPtr->setCloseFunc(std::bind(&TcpConnection::closeFunc, this));
            mConnectionHandlerPtr->setErrorFunc(std::bind(&TcpConnection::errorFunc, this));

            mConnectionHandlerPtr->enableReading();
            mConnectionHandlerPtr->disableWriting();

            mEventLoop->registerEventHandler(mConnectionHandlerPtr);
            // mEventLoop->edgeTrigger(mConnectionHandlerPtr, true);
            mEventLoop->update(mConnectionHandlerPtr);
        }

        TcpConnection(const TcpConnection &) = delete;
        TcpConnection &operator=(const TcpConnection &) = delete;
        TcpConnection(TcpConnection &&) = delete;
        TcpConnection &operator=(TcpConnection &&) = delete;

        ~TcpConnection()
        {
            mEventLoop->remove(mConnectionHandlerPtr);
        }

        void setTcpNoDelay(bool on)
        {
            mConnectionSocket.setTcpNoDelay(on);
        }

        void setName(const std::string &name) { mName = name; }
        const std::string getName() const { return mName; }
        const std::string getIP() const { return mPeerAddr.getIP(); }
        const std::string getIPPort() const { return mPeerAddr.getHostAndServiceNUM(); }

        // send message directly
        void send(const std::string &msg)
        {
            send(msg.c_str(), msg.size());
        }
        // send message directly
        void send(const char *data, size_t len)
        {
            auto msgPtr = mOutputBuffer.get();
            if (msgPtr == nullptr)
            {
                auto n = mOutputBuffer.write(data, len);

                if (len - n > 0 && n >= 0)
                {
                    mConnectionHandlerPtr->enableWriting();
                    mEventLoop->update(mConnectionHandlerPtr);
                }
            }
            else
            {
                auto n = mOutputBuffer.write(msgPtr->c_str(), msgPtr->size());

                mOutputBuffer.put(data, len);
                mConnectionHandlerPtr->enableWriting();
                mEventLoop->update(mConnectionHandlerPtr);
            }
        }

        // put message into buffer, enable writable
        void putToBuffer(const std::string &msg)
        {
            putToBuffer(msg.data(), msg.length());
        }
        // put message into buffer, enable writable
        void putToBuffer(const char *data, size_t len)
        {
            mOutputBuffer.put(data, len);
            mConnectionHandlerPtr->enableWriting();
            mEventLoop->update(mConnectionHandlerPtr);
        }

        std::string get()
        {
            auto msg = mInputBuffer.get();
            return *msg;
        }

        // try to get oldest message which it's length is len, and if there is incomplete data, store it into mIncompleteMessage
        // if takeAway, take away the message and clear mIncompleteMessage
        std::string tryGet(size_t len, bool takeAway)
        {
            std::string res = "";

            if (len <= 0)
            {
                // just get an oldest message buffer in queue
                if (mIncompleteMessage.empty())
                {
                    auto msg = mInputBuffer.get();
                    if (msg)
                        return *msg;
                }
                else
                {
                    if (takeAway)
                    {
                        res = mIncompleteMessage;
                        mIncompleteMessage.clear();

                        return res;
                    }
                    else
                    {
                        return mIncompleteMessage;
                    }
                }
            }

            // TODO limite if(len > ?)

            if (mIncompleteMessage.length() == len)
            {
                res = std::move(mIncompleteMessage);
                if (takeAway)
                {
                    mIncompleteMessage.clear();
                }
            }
            else if (mIncompleteMessage.length() > len)
            {
                res = mIncompleteMessage.substr(0, len);
                if (takeAway)
                {
                    mIncompleteMessage = mIncompleteMessage.substr(len);
                }
            }
            else
            {
                while (!mInputBuffer.empty())
                {
                    auto msgPtr = mInputBuffer.get();
                    if (msgPtr)
                    {
                        auto l = msgPtr->length();
                        if ((l + mIncompleteMessage.length()) == len)
                        {
                            res = mIncompleteMessage.append(msgPtr->data(), l);
                            if (takeAway)
                            {
                                mIncompleteMessage.clear();
                            }
                            break;
                        }
                        else if ((l + mIncompleteMessage.length()) < len)
                        {
                            mIncompleteMessage.append(msgPtr->data(), l);
                        }
                        else
                        {
                            mIncompleteMessage.append(msgPtr->data(), l);
                            res = mIncompleteMessage.substr(0, len);
                            if (takeAway)
                            {
                                mIncompleteMessage = mIncompleteMessage.substr(len);
                            }
                            break;
                        }
                    }
                }
            }

            return res;
        }

        void setMessageFunc(const messageFuncType &msgFunc) { mMessageFunc = std::move(msgFunc); }
        // void setSendFunc(const sendFuncType &sendFunc) { mSendFunc = std::move(sendFunc); }
        void setCloseFunc(const closeFuncType &closeFunc) { mCloseFunc = std::move(closeFunc); }

        void setCurrentTimestamp(time_t timestamp) { mCommunicationTimestamp = timestamp; }
        time_t getLastTimetamp() const { return mCommunicationTimestamp; }

        // ConnectionState getState() const { return mState; }

        // if force, skip 4-way handshake and send RST, note incarnation issue
        void activeClose(bool force = false)
        {
            if (force)
            {
                mConnectionSocket.setLinger(1, 0);
            }

            if (mCloseFunc)
            {
                mCloseFunc(shared_from_this());
            }
            else
            {
                mConnectionSocket.close();
            }
        }

        void testInfo()
        {
            std::cout << "FD: " << mConnectionSocket.getFD() << " local addr:" << mLocalAddr.getHostAndServiceNUM() << " peer addr:" << mPeerAddr.getHostAndService() << std::endl;
        }

    private:
        void readFunc()
        {
            auto n = mInputBuffer.read();
            if (n > 0)
            {
                if (mMessageFunc)
                {
                    mMessageFunc(shared_from_this());
                }
            }
            else if (n == 0)
            {
                closeFunc();
            }
            else
            {
                errorFunc();
            }
        }
        void writeFunc()
        {
            // std::cout << "TODO writeFunc " << std::endl;
            auto strPtr = mOutputBuffer.get();
            if (strPtr == nullptr)
            {
                mConnectionHandlerPtr->disableWriting();
                mEventLoop->update(mConnectionHandlerPtr);

                return;
            }

            auto n = mOutputBuffer.write(strPtr->c_str(), strPtr->size());
        }
        void closeFunc()
        {
            if (mCloseFunc)
                mCloseFunc(shared_from_this()); // mCloseFunc should remove this
            else
                mConnectionSocket.close();
        }
        void errorFunc()
        {
            auto err = mConnectionSocket.getSocketError();
            char errorBuffer[512];
            auto errStr = ::strerror_r(err, errorBuffer, strlen(errorBuffer));
            LOG_INFO << errStr;
        }

    private:
        socket::TCPSocket mConnectionSocket;
        const socket::SocketAddr mLocalAddr;
        const socket::SocketAddr mPeerAddr;
        std::shared_ptr<event::EventHandler> mConnectionHandlerPtr;

        std::shared_ptr<event::EventLoop> mEventLoop;
        std::string mName;

        time_t mCommunicationTimestamp = 0;

        IOBuffer mInputBuffer;
        IOBuffer mOutputBuffer;

        messageFuncType mMessageFunc;
        // sendFuncType mSendFunc;
        closeFuncType mCloseFunc;

        ConnectionState mState;

        std::string mIncompleteMessage;
    };
}

#endif
