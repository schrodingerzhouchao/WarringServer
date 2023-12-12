#include "GameMessage.h"
#include <cassert>

namespace message
{
    void DSLoadOrUploadCharacterMessage::count()
    {
        countHelper();

        countCapacity(CODE, SCENE_ID, ACCOUNT, CHARACTER_NAME, CHARACTER_DATA);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string DSLoadOrUploadCharacterMessage::serialize()
    {
        serializeHelper();

        pushMembers(CODE, SCENE_ID, ACCOUNT, CHARACTER_NAME, CHARACTER_DATA);

        return std::string(mData, MSG_LEN);
    }

    void DSLoadOrUploadCharacterMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, CODE, SCENE_ID, ACCOUNT, CHARACTER_NAME, CHARACTER_DATA);
    }

    void DSMessage::count()
    {
        countHelper();

        countCapacity(CODE, DS_PORT,SCENE_ID, LOAD, SCENE_DATA);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string DSMessage::serialize()
    {
        serializeHelper();

        pushMembers(CODE, DS_PORT,SCENE_ID, LOAD, SCENE_DATA);

        return std::string(mData, MSG_LEN);
    }

    void DSMessage::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, CODE, DS_PORT,SCENE_ID, LOAD, SCENE_DATA);
    }

    void CharacterInfo::count()
    {
        countHelper();

        countCapacity(ACCOUNT, NAME, LEVEL, GENDER, OCCUPTION, STATE,
                      EXP, HEALTH, MANA, ARMOR, STAMINA, STRENGTH, AGILITY, INTELLECT,
                      SCENE_ID, SCENE_X, SCENE_Y, SCENE_Z);

        MSG_LEN += sizeof(size_t) * ARRAY_LIKE_LENGHTS.size();
    }

    std::string CharacterInfo::serialize()
    {
        serializeHelper();

        pushMembers(ACCOUNT, NAME, LEVEL, GENDER, OCCUPTION, STATE,
                    EXP, HEALTH, MANA, ARMOR, STAMINA, STRENGTH, AGILITY, INTELLECT,
                    SCENE_ID, SCENE_X, SCENE_Y, SCENE_Z);

        return std::string(mData, MSG_LEN);
    }

    void CharacterInfo::deserialize(const char *data)
    {
        deserializeHelper(data);

        popMembers(data, ACCOUNT, NAME, LEVEL, GENDER, OCCUPTION, STATE,
                   EXP, HEALTH, MANA, ARMOR, STAMINA, STRENGTH, AGILITY, INTELLECT,
                   SCENE_ID, SCENE_X, SCENE_Y, SCENE_Z);
    }
}

namespace message
{
    namespace message_carrier
    {

        void GameMessage::FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const
        {
            destVector.clear();

            destVector.emplace_back(std::make_pair(TYPE_TAG, TYPE));
            destVector.emplace_back(std::make_pair(SERVICE_TAG, SERVICE));
            destVector.emplace_back(std::make_pair(NEXT_SERVICE_TAG, NEXT_SERVICE));
            destVector.emplace_back(std::make_pair(CLIENT_ID_TAG, CLIENT_ID));
            destVector.emplace_back(std::make_pair(CLIENT_ADDRESS_TAG, CLIENT_ADDRESS));
            destVector.emplace_back(std::make_pair(MESSAGE_TAG, MESSAGE));
        }

        void GameMessage::FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector)
        {
            //assert(TAG_NUM == sourceVector.size());

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