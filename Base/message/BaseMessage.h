#ifndef __MESSAGE_BASE_MESSAGE_H__
#define __MESSAGE_BASE_MESSAGE_H__

/*
 * TODO limit the max size of buffer
 *
 * support primitive types, std::vector<primitive type>, std::string, and std::vector<std::string>, but not pointers
 * Note there is at most one vector<string>, and vector<string> must be serialized/deserialized at the end
 * Note there should be not '\0' in the std::string except of the end
 *
 */

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <iostream>

namespace message
{
    using MESSAGE_LENGTH_TYPE = size_t;

    const std::string MSG_NAME_NONE = "MSG_NAME_NONE";

    struct BaseMessage
    {
        std::string MSG_NAME = MSG_NAME_NONE;

        virtual ~BaseMessage()
        {
            delete[] mData;
            mData = nullptr;
        }

        // count needed capacity for members
        // note call it when members assigned
        virtual void count();

        virtual std::string serialize();

        virtual void deserialize(const char *data);

        virtual BaseMessage &getRef() { return *this; }

        size_t GET_MSG_LEN() const { return MSG_LEN; }

        size_t GET_ARRAY_LIKE_S_SIZE() const { return ARRAY_LIKE_S_SIZE; }

        size_t GET_ARRAY_LIKE_LENGTH_BY_INDEX(size_t index) const { return index >= ARRAY_LIKE_S_SIZE ? -1 : ARRAY_LIKE_LENGHTS.at(index); }

    protected:
        void countHelper();

        void serializeHelper();

        void deserializeHelper(const char *data);

        // count the members capacity
        template <typename T1, typename... Tn>
        void countCapacity(const T1 &arg1, const Tn &...args)
        {
            countLength(arg1);
            countCapacity(args...);
        }

        void countCapacity() {}

        template <typename T>
        void countLength(const T &val)
        {
            if (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                std::is_signed_v<T> || std::is_unsigned_v<T>)
            {
                MSG_LEN += sizeof(T);
            }
        }
        void countLength(const std::string &val)
        {
            MSG_LEN += val.length();
            ARRAY_LIKE_LENGHTS.push_back(val.length());

            ++ARRAY_LIKE_S_SIZE;
        }
        template <typename T>
        void countLength(const std::vector<T> &val)
        {
            if (std::is_same_v<T, std::string>)
            {
                countLength(val);
            }
            else
            {
                MSG_LEN += ((sizeof(T)) * (val.size()));
                ARRAY_LIKE_LENGHTS.push_back(val.size());

                ++ARRAY_LIKE_S_SIZE;
            }
        }
        void countLength(const std::vector<std::string> &strs)
        {
            for (const auto &str : strs)
            {
                MSG_LEN += str.length();
                ARRAY_LIKE_LENGHTS.push_back(str.length());

                ++ARRAY_LIKE_S_SIZE;
            }
        }

        // copy the members to char[]
        template <typename T1, typename... Tn>
        void pushMembers(const T1 &arg1, const Tn &...args)
        {
            pushMember(arg1);
            pushMembers(args...);
        }

        void pushMembers() {}

        template <typename T>
        void pushMember(const T &val)
        {
            if (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                std::is_signed_v<T> || std::is_unsigned_v<T>)
            {
                ::memcpy(mData + mCursor, &val, sizeof(T));
                mCursor += sizeof(T);
            }
        }
        void pushMember(const std::string &val)
        {
            ::memcpy(mData + mCursor, val.data(), val.length());
            mCursor += val.length();
        }

        template <typename T>
        void pushMember(const std::vector<T> &val)
        {
            if (std::is_same_v<T, std::string>)
            {
                pushMember(val);
            }
            else
            {
                auto typeLen = sizeof(T);

                for (const auto &m : val)
                {
                    ::memcpy(mData + mCursor, &m, typeLen);
                    mCursor += typeLen;
                }
            }
        }
        void pushMember(const std::vector<std::string> &strs)
        {
            for (const auto &str : strs)
            {
                ::memcpy(mData + mCursor, str.data(), str.length());
                mCursor += str.length();
            }
        }

        // get members from char[]
        template <typename T1, typename... Tn>
        void popMembers(const char *data, T1 &arg1, Tn &...args)
        {
            popMember(data, arg1);
            popMembers(data, args...);
        }

        void popMembers(const char *data) {}

        template <typename T>
        void popMember(const char *data, T &val)
        {
            if (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                std::is_signed_v<T> || std::is_unsigned_v<T>)
            {
                ::memcpy(&val, data + mCursor, sizeof(T));
                mCursor += sizeof(T);
            }
        }
        void popMember(const char *data, std::string &val)
        {
            auto len = ARRAY_LIKE_LENGHTS.at(mArrayLikeIndex++);
            val = std::string(data + mCursor, len);
            mCursor += len;
        }
        template <typename T>
        void popMember(const char *data, std::vector<T> &val)
        {
            if (std::is_same_v<T, std::string>)
            {
                popMember(data, val);
            }
            else
            {
                auto len = ARRAY_LIKE_LENGHTS.at(mArrayLikeIndex++);

                for (size_t i = 0; i < len; ++i)
                {
                    T tmp;
                    ::memcpy(&tmp, data + mCursor, sizeof(T));
                    mCursor += sizeof(T);
                    val.push_back(tmp);
                }
            }
        }
        void popMember(const char *data, std::vector<std::string> &strs)
        {
            for (size_t i = mArrayLikeIndex; i < ARRAY_LIKE_S_SIZE; ++i)
            {
                strs.emplace_back(std::string(data + mCursor, ARRAY_LIKE_LENGHTS.at(i)));
                mCursor += ARRAY_LIKE_LENGHTS.at(i);
            }
        }

        void fillArrayLike(const char *data);

    protected:
        // total length of mData
        size_t MSG_LEN = 0;
        char *mData = nullptr;

        // save the array-like member's length
        // note keep in a consistent order when serialization and deserialization
        std::vector<size_t> ARRAY_LIKE_LENGHTS;
        // ARRAY_LIKE_LENGHTS's length
        size_t ARRAY_LIKE_S_SIZE = 0;

        size_t mArrayLikeIndex = 0;

        // mData's cursor
        size_t mCursor = 0;
    };

}

#endif