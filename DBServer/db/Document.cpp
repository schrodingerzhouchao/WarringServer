#include "Document.h"
#include "GameDataTable/GameDataCharacterAttribute.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

namespace db_server
{
    namespace db
    {
        bsoncxx::document::value MakeDocHelper::makeDocNewAccount(const std::string &accountName, const std::string &password, const std::string &address)
        {
            return make_document(
                kvp("account", accountName),
                kvp("password", password),
                kvp("characters", make_array(make_document(
                                      kvp("character_name", accountName),
                                      kvp("character_level", 0),
                                      kvp("character_gender", true),
                                      kvp("character_occuption", 0),
                                      kvp("character_state", 0)))),
                kvp("last_login_time", bsoncxx::types::b_date(std::chrono::system_clock::now())),
                kvp("last_logout_time", ""),
                kvp("last_login_address", address),
                kvp("on_off", false));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountAndCharacter(const std::string &accountName, const std::string &characterName)
        {
            return make_document(
                kvp("account", accountName),
                kvp("characters.character_name", characterName));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountInsertNewCharacter(const schema::player::CharacterBaseInfo &newCharacter)
        {
            return make_document(
                kvp("$push", make_document(
                                 kvp("characters", make_document(
                                                       kvp("character_name", newCharacter.mName),
                                                       kvp("character_level", newCharacter.mLevel),
                                                       kvp("character_gender", newCharacter.mGender),
                                                       kvp("character_occuption", newCharacter.mOccuption),
                                                       kvp("character_state", newCharacter.mState))))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountInsertNewCharacter(const message::LoginCharacterRequestMessage &newCharacter)
        {
            return make_document(
                kvp("$push", make_document(
                                 kvp("characters", make_document(
                                                       kvp("character_name", newCharacter.CHARACTER_NAME),
                                                       kvp("character_level", 1),
                                                       kvp("character_gender", newCharacter.CHARACTER_GENDER),
                                                       kvp("character_occuption", newCharacter.CHARACTER_OCCUPTION),
                                                       kvp("character_state", newCharacter.CHARACTER_STATE))))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountUpdateCharacter(const message::CharacterBaseInfo &characterBaseInfo)
        {
            return make_document(
                kvp("$set", make_document(
                                kvp("characters.$.character_level", characterBaseInfo.CHARACTER_LEVEL))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountLogout()
        {
            return make_document(
                kvp("$set", make_document(
                                kvp("last_logout_time", bsoncxx::types::b_date(std::chrono::system_clock::now())),
                                kvp("on_off", false))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocAccountRemoveCharacter(const std::string &characterName)
        {
            return make_document(
                kvp("$pull", make_document(
                                 kvp("characters", make_document(
                                                       kvp("character_name", characterName))))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocNewCharacter(const std::string &accountName, const schema::player::CharacterBaseInfo &newCharacter)
        {
            return make_document(
                kvp("account", accountName),
                kvp("base_info", make_document(
                                     kvp("name", newCharacter.mName),
                                     kvp("level", newCharacter.mLevel),
                                     kvp("gender", newCharacter.mGender),
                                     kvp("occuption", newCharacter.mOccuption),
                                     kvp("state", newCharacter.mState))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocNewCharacter(const message::LoginCharacterRequestMessage &newCharacter)
        {
            // TODO check data
            auto location = game::NewerCharacterInit::newerSpawnLocation(newCharacter.CHARACTER_STATE, newCharacter.CHARACTER_OCCUPTION);
            auto attributes = game::NewerCharacterInit::newerAttribute(newCharacter.CHARACTER_STATE, newCharacter.CHARACTER_OCCUPTION);

            return make_document(
                kvp("account", newCharacter.ACCOUNT),
                kvp("base_info", make_document(
                                     kvp("name", newCharacter.CHARACTER_NAME),
                                     kvp("level", 1),
                                     kvp("gender", newCharacter.CHARACTER_GENDER),
                                     kvp("occuption", newCharacter.CHARACTER_OCCUPTION),
                                     kvp("state", newCharacter.CHARACTER_STATE))),
                kvp("attribute", make_document(
                                     kvp(game::field_name::FIELD_EXP, attributes[game::field_name::FIELD_EXP]),
                                     kvp(game::field_name::FIELD_HEALTH, attributes[game::field_name::FIELD_HEALTH]),
                                     kvp(game::field_name::FIELD_MANA, attributes[game::field_name::FIELD_MANA]),
                                     kvp(game::field_name::FIELD_ARMOR, attributes[game::field_name::FIELD_ARMOR]),
                                     kvp(game::field_name::FIELD_STAMINA, attributes[game::field_name::FIELD_STAMINA]),
                                     kvp(game::field_name::FIELD_STRENGTH, attributes[game::field_name::FIELD_STRENGTH]),
                                     kvp(game::field_name::FIELD_AGILITY, attributes[game::field_name::FIELD_AGILITY]),
                                     kvp(game::field_name::FIELD_INTELLECT, attributes[game::field_name::FIELD_INTELLECT]))),
                kvp("location", make_document(
                                    kvp(game::field_name::FIELD_SCENE_ID, std::get<0>(location)),
                                    kvp(game::field_name::FIELD_SCENE_X, std::get<1>(location)),
                                    kvp(game::field_name::FIELD_SCENE_Y, std::get<2>(location)),
                                    kvp(game::field_name::FIELD_SCENE_Z, std::get<3>(location)))));
        }

        bsoncxx::document::value MakeDocHelper::makeDocUpdateCharacter(const message::CharacterInfo &characterInfo)
        {
            return make_document(
                kvp("$set", make_document(
                                kvp("base_info.level", characterInfo.LEVEL),
                                kvp("attribute.exp", characterInfo.EXP),
                                kvp("attribute.health", characterInfo.HEALTH),
                                kvp("attribute.mana", characterInfo.MANA),
                                kvp("attribute.armor", characterInfo.ARMOR),
                                kvp("attribute.stamina", characterInfo.STAMINA),
                                kvp("attribute.strength", characterInfo.STRENGTH),
                                kvp("attribute.agility", characterInfo.AGILITY),
                                kvp("attribute.intellect", characterInfo.INTELLECT),
                                kvp("location.scene_id", characterInfo.SCENE_ID),
                                kvp("location.scene_x", characterInfo.SCENE_X),
                                kvp("location.scene_y", characterInfo.SCENE_Y),
                                kvp("location.scene_z", characterInfo.SCENE_Z))));
        }
    }
}
