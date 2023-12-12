#include "GatewayMessage.h"
#include <cassert>

namespace message
{
    void EnterGameRequestMessage::count()
    {
        countHelper();

        countCapacity(REQUEST_CODE, ACCOUNT, CHARACTER);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string EnterGameRequestMessage::serialize()
    {
        serializeHelper();

        pushMembers(REQUEST_CODE, ACCOUNT, CHARACTER);

        return std::string(mData, MSG_LEN);
    }

    void EnterGameRequestMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, REQUEST_CODE, ACCOUNT, CHARACTER);
    }

    void EnterGameResponseMessage::count()
    {
        countHelper();

        countCapacity(RESPONSE_CODE);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string EnterGameResponseMessage::serialize()
    {
        serializeHelper();

        pushMembers(RESPONSE_CODE);

        return std::string(mData, MSG_LEN);
    }

    void EnterGameResponseMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, RESPONSE_CODE);
    }

    void ChatRequestMessage::count()
    {
        countHelper();

        countCapacity(REQUEST_CODE, ACCOUNT, CHARACTER, CHANNEL, NOTE);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string ChatRequestMessage::serialize()
    {
        serializeHelper();

        pushMembers(REQUEST_CODE, ACCOUNT, CHARACTER, CHANNEL, NOTE);

        return std::string(mData, MSG_LEN);
    }

    void ChatRequestMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, REQUEST_CODE, ACCOUNT, CHARACTER, CHANNEL, NOTE);
    }
}

namespace message
{
    namespace message_carrier
    {
        void GatewayMessage::FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const
        {
            destVector.clear();

            destVector.emplace_back(std::make_pair(TYPE_TAG, TYPE));
            destVector.emplace_back(std::make_pair(SERVICE_TAG, SERVICE));
            destVector.emplace_back(std::make_pair(NEXT_SERVICE_TAG, NEXT_SERVICE));
            destVector.emplace_back(std::make_pair(CLIENT_ID_TAG, CLIENT_ID));
             destVector.emplace_back(std::make_pair(CLIENT_ADDRESS_TAG, CLIENT_ADDRESS));
            destVector.emplace_back(std::make_pair(MESSAGE_TAG, MESSAGE));
        }

        void GatewayMessage::FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector)
        {
            assert(TAG_NUM == sourceVector.size());

            size_t i = 0;
            TYPE = sourceVector.at(i++).second;
            SERVICE = sourceVector.at(i++).second;
            NEXT_SERVICE = sourceVector.at(i++).second;
            CLIENT_ID = sourceVector.at(i++).second;
            CLIENT_ADDRESS = sourceVector.at(i++).second;
            MESSAGE = sourceVector.at(i++).second;
        }

    }
}