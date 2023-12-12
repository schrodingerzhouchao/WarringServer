#include "CustomMessages.h"
#include <cassert>

namespace message
{
    time_t TimestampMessage::getMillisecondsSinceEpoch()
    {
        std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        return millis.count();
    }

    void TimestampMessage::count()
    {
        countHelper();

        countCapacity(MILLISECONDS_SINCE_EPOCH);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string TimestampMessage::serialize()
    {
        serializeHelper();

        pushMembers(MILLISECONDS_SINCE_EPOCH);

        return std::string(mData, MSG_LEN);
    }

    void TimestampMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, MILLISECONDS_SINCE_EPOCH);
    }

    namespace message_carrier
    {
        void BaseMessage::FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const
        {
            destVector.clear();

            destVector.emplace_back(std::make_pair(TYPE_TAG, TYPE));
            destVector.emplace_back(std::make_pair(SERVICE_TAG, SERVICE));
            destVector.emplace_back(std::make_pair(NEXT_SERVICE_TAG, NEXT_SERVICE));
            destVector.emplace_back(std::make_pair(MESSAGE_TAG, MESSAGE));
        }

        void BaseMessage::FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector)
        {
            assert(TAG_NUM == sourceVector.size());

            size_t i = 0;
            TYPE = sourceVector.at(i++).second;
            SERVICE = sourceVector.at(i++).second;
            NEXT_SERVICE = sourceVector.at(i++).second;
            MESSAGE = sourceVector.at(i++).second;
        }
    }

    namespace test
    {
        void TestVecNormal::count()
        {
            countHelper();

            countCapacity(INTS, DOUBLES);

            MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
        }

        std::string TestVecNormal::serialize()
        {
            serializeHelper();

            pushMembers(INTS, DOUBLES);

            return std::string(mData, MSG_LEN);
        }

        void TestVecNormal::deserialize(const char *data)
        {
            deserializeHelper(data);

            popMembers(data, INTS, DOUBLES);
        }

        void TestStringsMessage::count()
        {
            countHelper();

            countCapacity(BOOL, INTS, STRINGS);

            MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
        }

        std::string TestStringsMessage::serialize()
        {
            serializeHelper();

            pushMembers(BOOL, INTS, STRINGS);

            return std::string(mData, MSG_LEN);
        }

        void TestStringsMessage::deserialize(const char *data)
        {
            deserializeHelper(data);

            popMembers(data, BOOL, INTS, STRINGS);
        }
        void MessagesPacker::count()
        {
            countHelper();

            countCapacity(MESSAGES);

            MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
        }

        std::string MessagesPacker::serialize()
        {
            serializeHelper();

            pushMembers(MESSAGES);

            return std::string(mData, MSG_LEN);
        }

        void MessagesPacker::deserialize(const char *data)
        {
            deserializeHelper(data);

            popMembers(data, MESSAGES);
        }
        void CLoginMessage::count()
        {
            countHelper();

            countCapacity(requestCode, account, password);

            MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
        }

        std::string CLoginMessage::serialize()
        {
            serializeHelper();

            pushMembers(requestCode, account, password);

            return std::string(mData, MSG_LEN);
        }

        void CLoginMessage::deserialize(const char *data)
        {
            deserializeHelper(data);

            popMembers(data, requestCode, account, password);
        }

        void SLoginMessage::count()
        {
            countHelper();

            countCapacity(requestResult, token);

            MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
        }

        std::string SLoginMessage::serialize()
        {
            serializeHelper();

            pushMembers(requestResult, token);

            return std::string(mData, MSG_LEN);
        }

        void SLoginMessage::deserialize(const char *data)
        {
            deserializeHelper(data);

            popMembers(data, requestResult, token);
        }
    }

}
