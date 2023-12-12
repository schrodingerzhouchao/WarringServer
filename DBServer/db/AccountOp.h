#ifndef __DBSERVER_DB_MONGODB_ACCOUNT_OPERATION_H__
#define __DBSERVER_DB_MONGODB_ACCOUNT_OPERATION_H__

#include <mongocxx/pool.hpp>

#include "Schema.h"
#include "Base/message/LoginMessage.h"
#include "Base/message/GatewayMessage.h"
#include "Base/message/GameMessage.h"
#include "Base/utility/LoggerMessage.h"

#include <optional>

namespace db_server
{
    namespace db
    {
        class AccountOp
        {
        public:
            AccountOp(mongocxx::pool &pool)
                : mPool{pool}
            {
            }

            void initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName);

            bool createIndex();

            int addNewAccount(const std::string &accountName, const std::string &password, const std::string &ip);

            int addNewCharacter(const std::string &accountName, const schema::player::CharacterBaseInfo &newCharacter);

            int addNewCharacter(const message::LoginCharacterRequestMessage &newCharacter);

            int removeCharacter(const std::string &accountName, const std::string &characterName);

            int updateCharacter(const message::CharacterInfo &characterInfo, bool last = false);

            int getCharacterNum(const std::string &accountName);

            std::optional<std::vector<std::string>> getAllCharacterName(const std::string &accountName);

            std::optional<std::vector<schema::player::CharacterBaseInfo>> getAllCharacterBaseInfo(const std::string &accountName);

            std::optional<std::vector<message::CharacterBaseInfo>> getAllCharacterBaseInfoFill2Message(const std::string &accountName);

            std::optional<message::CharacterBaseInfo> getOneCharacterBaseInfoFill2Message(const std::string &accountName, const std::string &characterName);

            int matchAccount(const std::string &accountName, const std::string &password);

            int matchAccountAndUpdateLoginState(const std::string &accountName, const std::string &password, bool tryLogin = true);

            int matchAccountAndCharacter(const std::string &accountName, const std::string &characterName);

        private:
            std::string mDBName;
            std::string mAccountCollectionName;
            std::string mCharacterCollectionName;

            mongocxx::pool &mPool;
        };
    }
}

#endif