#ifndef __NET_IO_BUFFER_H__
#define __NET_IO_BUFFER_H__

#include "NetParam.h"

#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <queue>
#include <string>
#include <memory>

#include <iostream>

namespace net
{
    class IOBuffer
    {
    public:
        explicit IOBuffer(int fd)
            : mFD(fd),
              mInitSize(IO_BUFFER_INITIAL_SIZE()),
              mMaxSize(IO_BUFFER_MAX_SIZE())
        {
            mData.resize(mInitSize);
            mData.reserve(mMaxSize);
            mCurrSize = mInitSize;
        }
        ~IOBuffer()
        {
            mData.clear();
        }

        IOBuffer(const IOBuffer &) = delete;
        IOBuffer &operator=(const IOBuffer &) = delete;
        IOBuffer(IOBuffer &&) = delete;
        IOBuffer &operator=(IOBuffer &&) = delete;

        void put(const char *buf, const size_t len)
        {
            auto strPtr = std::make_unique<std::string>(buf, len);
            mDataQueue.push(std::move(strPtr));
        }
        void put(const std::string &str)
        {
            auto strPtr = std::make_unique<std::string>(str);
            mDataQueue.push(std::move(strPtr));
        }

        std::unique_ptr<std::string> get()
        {
            if (!mDataQueue.empty())
            {
                std::unique_ptr<std::string> strPtr = std::move(mDataQueue.front());
                mDataQueue.pop();
                return strPtr;
            }
            return nullptr;
        }

        size_t getCurrentSize() const { return mCurrSize; }
        int getFD() const { return mFD; }
        bool empty() const { return mDataQueue.empty(); }

        ssize_t read()
        {
            ssize_t n = 0;

            // if(0)
            if (mCurrSize < mMaxSize)
            {
                char extraBuff[mMaxSize - mCurrSize];
                iovec vec[2];
                vec[0].iov_base = &*mData.begin();
                vec[0].iov_len = mCurrSize;
                vec[1].iov_base = extraBuff;
                vec[1].iov_len = sizeof(extraBuff);

                n = ::readv(mFD, vec, 2);
                if (n < 0)
                {
                    if (errno != EWOULDBLOCK)
                        ::perror("TODO IOBUffer::read::readv()");
                }
                else
                {
                    auto multiple = n / mCurrSize;
                    if (multiple < 1)
                    {
                        if (++mDecayNum >= mContinuousDecayLimit)
                            mCurrSize = mInitSize;
                    }
                    else
                    {
                        mData.append(extraBuff, n - mCurrSize);
                        mDecayNum = 0;
                        mCurrSize = ((multiple + 1) * mInitSize) < mMaxSize ? ((multiple + 1) * mInitSize) : mMaxSize;
                    }
                }
            }
            else
            {
                n = ::read(mFD, &*mData.begin(), mCurrSize);
                if (n < 0)
                {
                    if (errno != EWOULDBLOCK)
                        ::perror("TODO IOBuffer::read::readv()");
                }
            }

            if (n > 0)
            {
                put(mData.data(), n);
                mData.clear();
                mData.resize(mCurrSize);
            }
            return n;
        }

        ssize_t write(const char *buf, size_t len)
        {
            ssize_t n = 0;

            n = ::write(mFD, buf, len);
            if (n < 0)
            {
                if (errno != EWOULDBLOCK)
                    ::perror("TODO IOBUffer::write::write()");
            }
            else
            {
                size_t remaining = len - n;
                if (remaining > 0)
                {
                    put(buf + n, remaining);
                }
            }

            return n;
        }

    private:
        const int mFD;

        std::string mData;
        size_t mCurrSize;
        const size_t mInitSize;
        const size_t mMaxSize;

        // if the size of read is smaller than mCurrSize more than mContinuousDecayLimit times in succession,
        // should decrease mCurrSize appropriately
        const unsigned short mContinuousDecayLimit = IO_BUFFER_DECAY_LIMIT();
        unsigned short mDecayNum = 0;

        std::queue<std::unique_ptr<std::string>> mDataQueue;
    };

}

#endif