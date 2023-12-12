#ifndef __DBSERVER_DB_MONGODB_CHARACTER_OPERATION_H__
#define __DBSERVER_DB_MONGODB_CHARACTER_OPERATION_H__

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
        class CharacterOp
        {
        public:
            CharacterOp(mongocxx::pool &pool)
                : mPool{pool}
            {
            }

            void initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName);

            bool createIndex();

            std::optional<message::CharacterInfo> getOneCharacterInfoFill2Message(const std::string &accountName, const std::string &characterName);

            std::string getOneCharacterInfoFill2Data(const std::string &accountName, const std::string &characterName);

        private:
            std::string mDBName;
            std::string mAccountCollectionName;
            std::string mCharacterCollectionName;

            mongocxx::pool &mPool;
        };
    }
}

#endif