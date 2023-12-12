#ifndef __DBSERVER_DB_MONGODB_H__
#define __DBSERVER_DB_MONGODB_H__

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/client_session.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

#include "AccountOp.h"
#include "CharacterOp.h"

#include <cassert>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

namespace db_server
{
    namespace db
    {
        class Database
        {
        public:
            Database(const std::string &url)
                : mURL{url},
                  mPool{mURL},
                  mAccountOp(mPool),
                  mCharacterOp(mPool)
            {
            }

            void initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName);

            void check();

            bool createIndex();

            // account operation
            int addNewAccount(const std::string &accountName, const std::string &password, const std::string &address);

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

            // 0: not match, 1: match, 2: match but already login
            int matchAccountAndUpdateLoginState(const std::string &accountName, const std::string &password, bool tryLogin = true);

            int matchAccountAndCharacter(const std::string &accountName, const std::string &characterName);

            // character operation
            std::optional<message::CharacterInfo> getOneCharacterInfoFill2Message(const std::string &accountName, const std::string &characterName);

            std::string getOneCharacterInfoFill2Data(const std::string &accountName, const std::string &characterName);

        private:
            mongocxx::instance mInstance{};
            mongocxx::uri mURL;
            mongocxx::pool mPool;

            std::string mDBName;
            std::string mAccountCollectionName;
            std::string mCharacterCollectionName;

            AccountOp mAccountOp;
            CharacterOp mCharacterOp;
        };

    }
}

#endif