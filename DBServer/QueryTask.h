#ifndef __DB_SERVER_ACCOUNT_LOGIN_TASK_H__
#define __DB_SERVER_ACCOUNT_LOGIN_TASK_H__

#include "Base/utility/LoggerMessage.h"
#include "Base/message/LoginMessage.h"
#include "Base/message/GatewayMessage.h"
#include "Base/message/GameMessage.h"

#include "GameDataTable/GameDataCode.h"

#include "DBServConf.h"
#include "db/Database.h"
#include "cache/RCache.h"

#include <chrono>

namespace db_server
{
    namespace task
    {
        class QueryTask
        {
        public:
            QueryTask(db_server::DBServConf &conf, db::Database &database, cache::RCache &cache, sw::redis::Redis &servRegRedis)
                : mConf(conf),
                  mDB(database),
                  mCache(cache),
                  mServRegRedis(servRegRedis)
            {
            }

            void getLoginResponseMsg(message::message_carrier::LoginMessage &loginMsgCarrier);

            void getLoginCharacterResponseMsg(message::message_carrier::LoginMessage &loginMsgCarrier);

            void getGameResponseMsg(message::message_carrier::GameMessage &gameMsgCarrier);

            void getGameCharacterResponseMsg(message::message_carrier::GameMessage &gameMsgCarrier);

        private:
            std::string getMillisecondsSinceEpoch();

            std::string generateToken(const std::string &account);

            // return key with address of the low-load service
            std::string getLowLoadService(const std::string &serviceKey);

            std::string getLowDS(const std::string &dsKey);

            std::optional<std::pair<std::string, int>> getServiceAddress(const std::string &serviceKey);

        private:
            db_server::DBServConf &mConf;
            db::Database &mDB;
            cache::RCache &mCache;
            sw::redis::Redis &mServRegRedis;
        };
    }
}

#endif