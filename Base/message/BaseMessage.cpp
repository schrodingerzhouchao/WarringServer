
#include "BaseMessage.h"

namespace message
{

    void BaseMessage::count()
    {
        countHelper();

        // then countCapacity() if there are some new members

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string BaseMessage::serialize()
    {
        serializeHelper();

        // then pushMembers() if there are some new members

        return std::string(mData, MSG_LEN);
    }

    void BaseMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        // then popMembers() if there are some new members
    }

    void BaseMessage::fillArrayLike(const char *data)
    {
        if (ARRAY_LIKE_S_SIZE <= 0)
            return;
        for (size_t i = 0; i < ARRAY_LIKE_S_SIZE; ++i)
        {
            size_t len = 0;
            ::memcpy(&len, data + mCursor, sizeof(size_t));
            mCursor += sizeof(size_t);
            ARRAY_LIKE_LENGHTS.push_back(len);
        }
    }

    void BaseMessage::countHelper()
    {
        countCapacity(MSG_NAME, MSG_LEN, ARRAY_LIKE_S_SIZE);
    }

    void BaseMessage::serializeHelper()
    {
        mCursor = 0;
        mData = new char[MSG_LEN];

        //  MSG_LEN is first, then ARRAY_LIKE_S_SIZE, ARRAY_LIKE_LENGHTS
        pushMembers(MSG_LEN, ARRAY_LIKE_S_SIZE, ARRAY_LIKE_LENGHTS, MSG_NAME);
    }

    void BaseMessage::deserializeHelper(const char *data)
    {
        mCursor = 0;
        // MSG_LEN is first, then ARRAY_LIKE_S_SIZE
        popMembers(data, MSG_LEN, ARRAY_LIKE_S_SIZE);
        // fill ARRAY_LIKE_LENGHTS
        fillArrayLike(data);

        popMembers(data, MSG_NAME);
    }

}