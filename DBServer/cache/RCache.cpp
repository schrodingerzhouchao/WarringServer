#include "RCache.h"

namespace db_server
{
    namespace cache
    {
        int RCache::addCharacter(const std::string &characterKey, const std::unordered_map<std::string, std::string> &characterInfo)
        {
            try
            {
                return mRedis.hset(characterKey, characterInfo.begin(), characterInfo.end());
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        int RCache::deleteCharacter(const std::string &characterKey)
        {
            try
            {
                return mRedis.unlink(characterKey);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        int RCache::characterExisted(const std::string &characterKey)
        {
            try
            {
                return mRedis.exists(characterKey);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        std::optional<bool> RCache::setTTL(const std::string &characterKey, size_t secs)
        {
            try
            {
                return mRedis.expire(characterKey, secs);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        int RCache::getCharacter(const std::string &characterKey, message::CharacterInfo &characterInfo)
        {
            try
            {
                std::unordered_map<std::string, std::string> infos;
                mRedis.hgetall(characterKey, std::inserter(infos, infos.begin()));

                characterInfo.ACCOUNT = infos[game::field_name::FIELD_ACCOUNT];
                characterInfo.NAME = infos[game::field_name::FIELD_CHARACTER_NAME];
                characterInfo.LEVEL = std::stoi(infos[game::field_name::FIELD_CHARACTER_LEVEL]);
                characterInfo.GENDER = (infos[game::field_name::FIELD_CHARACTER_GENDER] == "true");
                characterInfo.OCCUPTION = std::stoi(infos[game::field_name::FIELD_CHARACTER_OCCUPTION]);
                characterInfo.STATE = std::stoi(infos[game::field_name::FIELD_CHARACTER_STATE]);
                characterInfo.EXP = std::stoi(infos[game::field_name::FIELD_EXP]);
                characterInfo.HEALTH = std::stoi(infos[game::field_name::FIELD_HEALTH]);
                characterInfo.MANA = std::stoi(infos[game::field_name::FIELD_MANA]);
                characterInfo.ARMOR = std::stoi(infos[game::field_name::FIELD_ARMOR]);
                characterInfo.STAMINA = std::stoi(infos[game::field_name::FIELD_STAMINA]);
                characterInfo.STRENGTH = std::stoi(infos[game::field_name::FIELD_STRENGTH]);
                characterInfo.AGILITY = std::stoi(infos[game::field_name::FIELD_AGILITY]);
                characterInfo.INTELLECT = std::stoi(infos[game::field_name::FIELD_INTELLECT]);
                characterInfo.SCENE_ID = std::stoi(infos[game::field_name::FIELD_SCENE_ID]);
                characterInfo.SCENE_X = std::stod(infos[game::field_name::FIELD_SCENE_X]);
                characterInfo.SCENE_Y = std::stod(infos[game::field_name::FIELD_SCENE_Y]);
                characterInfo.SCENE_Z = std::stod(infos[game::field_name::FIELD_SCENE_Z]);

                return 1;
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        int RCache::getCharacterData(const std::string &characterKey, std::string &characterData)
        {
            try
            {
                auto data = mRedis.hget(characterKey, game::field_name::FIELD_CHARACTER_DATA);
                characterData = data.has_value() ? data.value() : "";
                return characterData.empty() ? 0 : 1;
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }
    }
}