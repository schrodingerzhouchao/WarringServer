#include "CharacterOp.h"
#include "Document.h"

#include "GameDataTable/GameDataCode.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/client_session.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/instance.hpp>

#include <cassert>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

namespace db_server
{
    namespace db
    {
        void CharacterOp::initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName)
        {
            mDBName = dbName;
            mAccountCollectionName = accountCollName;
            mCharacterCollectionName = characterCollName;

            assert(!mDBName.empty());
            assert(!mAccountCollectionName.empty());
            assert(!mCharacterCollectionName.empty());
        }

        bool CharacterOp::createIndex()
        {
            auto client = mPool.acquire();

            try
            {
                client->database(mDBName).collection(mCharacterCollectionName).create_index(make_document(kvp("account", 1), kvp("base_info.name", 1)));
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();

                return false;
            }
            return true;
        }

        std::optional<message::CharacterInfo> CharacterOp::getOneCharacterInfoFill2Message(const std::string &accountName, const std::string &characterName)
        {
            message::CharacterInfo characterInfo;

            auto client = mPool.acquire();
            if (!client)
            {
                return std::nullopt;
            }

            try
            {
                auto findResult = client->database(mDBName).collection(mCharacterCollectionName).find_one(make_document(kvp("account", accountName), kvp("base_info.name", characterName)));
                if (findResult)
                {
                    characterInfo.ACCOUNT = accountName;
                    characterInfo.NAME = characterName;

                    game::field_name::FIELD_LOCATION;

                    characterInfo.LEVEL = findResult.value()[game::field_name::FIELD_CHARACTER_BASE_INFO][game::field_name::FIELD_CHARACTER_LEVEL].get_int32().value;
                    characterInfo.GENDER = findResult.value()[game::field_name::FIELD_CHARACTER_BASE_INFO][game::field_name::FIELD_CHARACTER_GENDER].get_bool().value;
                    characterInfo.OCCUPTION = findResult.value()[game::field_name::FIELD_CHARACTER_BASE_INFO][game::field_name::FIELD_CHARACTER_OCCUPTION].get_int32().value;
                    characterInfo.STATE = findResult.value()[game::field_name::FIELD_CHARACTER_BASE_INFO][game::field_name::FIELD_CHARACTER_STATE].get_int32().value;
                    characterInfo.EXP = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_EXP].get_int32().value;
                    characterInfo.HEALTH = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_HEALTH].get_int32().value;
                    characterInfo.MANA = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_MANA].get_int32().value;
                    characterInfo.ARMOR = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_ARMOR].get_int32().value;
                    characterInfo.STAMINA = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_STAMINA].get_int32().value;
                    characterInfo.STRENGTH = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_STRENGTH].get_int32().value;
                    characterInfo.AGILITY = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_AGILITY].get_int32().value;
                    characterInfo.INTELLECT = findResult.value()[game::field_name::FIELD_ATTRIBUTE][game::field_name::FIELD_INTELLECT].get_int32().value;
                    characterInfo.SCENE_ID = findResult.value()[game::field_name::FIELD_LOCATION][game::field_name::FIELD_SCENE_ID].get_int32().value;
                    characterInfo.SCENE_X = findResult.value()[game::field_name::FIELD_LOCATION][game::field_name::FIELD_SCENE_X].get_double().value;
                    characterInfo.SCENE_Y = findResult.value()[game::field_name::FIELD_LOCATION][game::field_name::FIELD_SCENE_Y].get_double().value;
                    characterInfo.SCENE_Z = findResult.value()[game::field_name::FIELD_LOCATION][game::field_name::FIELD_SCENE_Z].get_double().value;

                    return std::make_optional<message::CharacterInfo>(characterInfo);
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        std::string CharacterOp::getOneCharacterInfoFill2Data(const std::string &accountName, const std::string &characterName)
        {
            auto data = getOneCharacterInfoFill2Message(accountName, characterName);
            if (data.has_value())
            {
                data.value().count();
                return data.value().serialize();
            }

            return {};
        }
    }
}