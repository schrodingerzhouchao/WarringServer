#ifndef __REDIS_CACHE_H__
#define __REDIS_CACHE_H__

#include "DBServer/DBServConf.h"
#include "Base/utility/LoggerMessage.h"
#include "Base/message/GameMessage.h"
#include "GameDataTable/GameDataCode.h"

namespace db_server
{
    namespace cache
    {
        class RCache
        {
        public:
            RCache(const DBServConf &conf)
                : mConf(conf),
                  mRedis(mConf.getCacheConnOpts(), mConf.getCacheConnPoolOpts())
            {
            }

            std::string generateCharacterKey(const std::string &accountName, const std::string &characterName) { return accountName + ":" + characterName; }

            int addCharacter(const std::string &characterKey, const std::unordered_map<std::string, std::string> &characterInfo);

            int deleteCharacter(const std::string &characterKey);

            int characterExisted(const std::string &characterKey);

            std::optional<bool> setTTL(const std::string &characterKey, size_t secs = 300);

            int getCharacter(const std::string &characterKey, message::CharacterInfo &characterInfo);

            int getCharacterData(const std::string &characterKey, std::string &characterData);

        private:
            const DBServConf &mConf;

            sw::redis::Redis mRedis;
        };
    }

}

#endif