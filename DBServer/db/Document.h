#ifndef __DBSERVER_DB_MONGODB_Dpcument_H__
#define __DBSERVER_DB_MONGODB_Dpcument_H__

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/collection.hpp>

#include "Base/message/LoginMessage.h"
#include "Base/message/GameMessage.h"
#include "Schema.h"

namespace db_server
{
    namespace db
    {
        class MakeDocHelper
        {
        public:
            static bsoncxx::document::value makeDocNewAccount(const std::string &accountName, const std::string &password, const std::string &address);

            static bsoncxx::document::value makeDocAccountAndCharacter(const std::string &accountName, const std::string &characterName);

            static bsoncxx::document::value makeDocAccountInsertNewCharacter(const schema::player::CharacterBaseInfo &newCharacter);

            static bsoncxx::document::value makeDocAccountInsertNewCharacter(const message::LoginCharacterRequestMessage &newCharacter);

            static bsoncxx::document::value makeDocAccountUpdateCharacter(const message::CharacterBaseInfo &characterBaseInfo);

            static bsoncxx::document::value makeDocAccountLogout();

            static bsoncxx::document::value makeDocAccountRemoveCharacter(const std::string &characterName);

            static bsoncxx::document::value makeDocNewCharacter(const std::string &accountName, const schema::player::CharacterBaseInfo &newCharacter);

            static bsoncxx::document::value makeDocNewCharacter(const message::LoginCharacterRequestMessage &newCharacter);

            static bsoncxx::document::value makeDocUpdateCharacter(const message::CharacterInfo &characterInfo);
        };

    }

}

#endif