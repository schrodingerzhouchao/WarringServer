#include "LoginMessage.h"
#include <cassert>

std::string message::code::login::CODE_MESSAGE(size_t code)
{
    switch (code)
    {
    case LOGIN_REGISTER_REQUEST_CODE:
        return "LOGIN_REGISTER_REQUEST_CODE";
        break;

        // more

    default:
        return {};
        break;
    }
}

namespace message
{
    namespace message_carrier
    {
        void LoginMessage::FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const
        {
            destVector.clear();

            destVector.emplace_back(std::make_pair(TYPE_TAG, TYPE));
            destVector.emplace_back(std::make_pair(SERVICE_TAG, SERVICE));
            destVector.emplace_back(std::make_pair(NEXT_SERVICE_TAG, NEXT_SERVICE));
            destVector.emplace_back(std::make_pair(CLIENT_ID_TAG, CLIENT_ID));
            destVector.emplace_back(std::make_pair(CLIENT_ADDRESS_TAG, CLIENT_ADDRESS));
            destVector.emplace_back(std::make_pair(MESSAGE_TAG, MESSAGE));
        }

        void LoginMessage::FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector)
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

namespace message
{
    // character base info message
    void CharacterBaseInfo::count()
    {
        countHelper();

        countCapacity(CHARACTER_NAME, CHARACTER_LEVEL, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string CharacterBaseInfo::serialize()
    {
        serializeHelper();

        pushMembers(CHARACTER_NAME, CHARACTER_LEVEL, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);

        return std::string(mData, MSG_LEN);
    }

    void CharacterBaseInfo::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, CHARACTER_NAME, CHARACTER_LEVEL, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);
    }

    // login request message
    void LoginRequestMessage::count()
    {
        countHelper();

        countCapacity(REQUEST_CODE, ACCOUNT, PASSWORD);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string LoginRequestMessage::serialize()
    {
        serializeHelper();

        pushMembers(REQUEST_CODE, ACCOUNT, PASSWORD);

        return std::string(mData, MSG_LEN);
    }

    void LoginRequestMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, REQUEST_CODE, ACCOUNT, PASSWORD);
    }

    // login response message
    void LoginResponseMessage::count()
    {
        countHelper();

        countCapacity(RESPONSE_CODE, TOKEN, GAME_GATEWAY_IP, GAME_GATEWAY_PORT, GAME_DS_IP, GAME_DS_PORT, CHARACTER_BASE_INFO_MESSAGES);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string LoginResponseMessage::serialize()
    {
        serializeHelper();

        pushMembers(RESPONSE_CODE, TOKEN, GAME_GATEWAY_IP, GAME_GATEWAY_PORT, GAME_DS_IP, GAME_DS_PORT, CHARACTER_BASE_INFO_MESSAGES);

        return std::string(mData, MSG_LEN);
    }

    void LoginResponseMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, RESPONSE_CODE, TOKEN, GAME_GATEWAY_IP, GAME_GATEWAY_PORT, GAME_DS_IP, GAME_DS_PORT, CHARACTER_BASE_INFO_MESSAGES);
    }

    // login character request message
    void LoginCharacterRequestMessage::count()
    {
        countHelper();

        countCapacity(REQUEST_CODE, ACCOUNT, PASSWORD, CHARACTER_NAME, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string LoginCharacterRequestMessage::serialize()
    {
        serializeHelper();

        pushMembers(REQUEST_CODE, ACCOUNT, PASSWORD, CHARACTER_NAME, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);

        return std::string(mData, MSG_LEN);
    }

    void LoginCharacterRequestMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, REQUEST_CODE, ACCOUNT, PASSWORD, CHARACTER_NAME, CHARACTER_GENDER, CHARACTER_OCCUPTION, CHARACTER_STATE);
    }

    void LoginCharacterRequestMessage::initAccount(const LoginRequestMessage &src, const std::string &msgName)
    {
        MSG_NAME = msgName;
        ACCOUNT = src.ACCOUNT;
        PASSWORD = src.PASSWORD;
    }

}