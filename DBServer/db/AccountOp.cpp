#include "AccountOp.h"
#include "Document.h"

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
        void AccountOp::initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName)
        {
            mDBName = dbName;
            mAccountCollectionName = accountCollName;
            mCharacterCollectionName = characterCollName;

            assert(!mDBName.empty());
            assert(!mAccountCollectionName.empty());
            assert(!mCharacterCollectionName.empty());
        }

        bool AccountOp::createIndex()
        {
            auto client = mPool.acquire();

            try
            {
                mongocxx::options::index indexOpts{};
                indexOpts.unique(true);
                client->database(mDBName).collection(mAccountCollectionName).create_index(make_document(kvp("account", 1)), indexOpts);
                client->database(mDBName).collection(mAccountCollectionName).create_index(make_document(kvp("characters.character_name", 1)), indexOpts);
            }
            catch (const mongocxx::exception &e)
            {
                LOG_WARN << e.what();

                return false;
            }
            return true;
        }

        int AccountOp::addNewAccount(const std::string &accountName, const std::string &password, const std::string &address)
        {
            auto client = mPool.acquire();
            if (!client)
                return -1;

            try
            {
                // set accountName as the characters default placeholder for characters.character_name unique index, see makeDocNewAccount()
                auto result = client->database(mDBName).collection(mAccountCollectionName).insert_one(MakeDocHelper::makeDocNewAccount(accountName, password, address));

                return result->result().inserted_count();
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                // duplicate key error
                if (e.code().value() == 11000)
                {
                    return 0;
                }
                return -1;
            }
        }

        int AccountOp::addNewCharacter(const std::string &accountName, const schema::player::CharacterBaseInfo &newCharacter)
        {
            auto client = mPool.acquire();
            if ((!client) || (accountName == newCharacter.mName))
                return -1;

            auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(MakeDocHelper::makeDocAccountAndCharacter(accountName, newCharacter.mName));

            if (findResult)
            {
                return 0;
            }

            mongocxx::write_concern wcMajority{};
            wcMajority.acknowledge_level(mongocxx::write_concern::level::k_majority);

            mongocxx::read_concern rcLocal{};
            rcLocal.acknowledge_level(mongocxx::read_concern::level::k_local);

            mongocxx::read_preference rpPrimary{};
            rpPrimary.mode(mongocxx::read_preference::read_mode::k_primary);

            mongocxx::client_session::with_transaction_cb callback = [&](mongocxx::client_session *session)
            {
                client->database(mDBName).collection(mAccountCollectionName).update_one(*session, make_document(kvp("account", accountName)), MakeDocHelper::makeDocAccountInsertNewCharacter(newCharacter));

                client->database(mDBName).collection(mCharacterCollectionName).insert_one(*session, MakeDocHelper::makeDocNewCharacter(accountName, newCharacter));
            };

            auto session = client->start_session();

            try
            {
                mongocxx::options::transaction opts;
                opts.write_concern(wcMajority);
                opts.read_concern(rcLocal);
                opts.read_preference(rpPrimary);
                session.with_transaction(callback, opts);

                return 1;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                // duplicate key error
                if (e.code().value() == 11000)
                {
                    return 0;
                }

                return -1;
            }
        }

        int AccountOp::addNewCharacter(const message::LoginCharacterRequestMessage &newCharacter)
        {
            auto client = mPool.acquire();

            auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(MakeDocHelper::makeDocAccountAndCharacter(newCharacter.ACCOUNT, newCharacter.CHARACTER_NAME));

            if (findResult)
            {
                return 0;
            }

            mongocxx::write_concern wcMajority{};
            wcMajority.acknowledge_level(mongocxx::write_concern::level::k_majority);

            mongocxx::read_concern rcLocal{};
            rcLocal.acknowledge_level(mongocxx::read_concern::level::k_local);

            mongocxx::read_preference rpPrimary{};
            rpPrimary.mode(mongocxx::read_preference::read_mode::k_primary);

            mongocxx::client_session::with_transaction_cb callback = [&](mongocxx::client_session *session)
            {
                client->database(mDBName).collection(mAccountCollectionName).update_one(*session, make_document(kvp("account", newCharacter.ACCOUNT)), MakeDocHelper::makeDocAccountInsertNewCharacter(newCharacter));

                client->database(mDBName).collection(mCharacterCollectionName).insert_one(*session, MakeDocHelper::makeDocNewCharacter(newCharacter));
            };

            auto session = client->start_session();

            try
            {
                mongocxx::options::transaction opts;
                opts.write_concern(wcMajority);
                opts.read_concern(rcLocal);
                opts.read_preference(rpPrimary);
                session.with_transaction(callback, opts);

                return 1;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                // duplicate key error
                if (e.code().value() == 11000)
                {
                    return 0;
                }

                return -1;
            }
        }

        int AccountOp::removeCharacter(const std::string &accountName, const std::string &characterName)
        {
            auto client = mPool.acquire();
            if ((!client) || (accountName == characterName))
                return -1;

            mongocxx::write_concern wcMajority{};
            wcMajority.acknowledge_level(mongocxx::write_concern::level::k_majority);

            mongocxx::read_concern rcLocal{};
            rcLocal.acknowledge_level(mongocxx::read_concern::level::k_local);

            mongocxx::read_preference rpPrimary{};
            rpPrimary.mode(mongocxx::read_preference::read_mode::k_primary);

            int removeResultCode = 0;

            mongocxx::client_session::with_transaction_cb callback = [&](mongocxx::client_session *session)
            {
                auto updateResult = client->database(mDBName).collection(mAccountCollectionName).update_one(make_document(kvp("account", accountName)), MakeDocHelper::makeDocAccountRemoveCharacter(characterName));
                if (updateResult->modified_count() == 1)
                {
                    auto removeResult = client->database(mDBName).collection(mCharacterCollectionName).delete_one(make_document(kvp("account", accountName), kvp("base_info.name", characterName)));
                    removeResultCode = removeResult->deleted_count();
                }
            };

            auto session = client->start_session();

            try
            {
                mongocxx::options::transaction opts;
                opts.write_concern(wcMajority);
                opts.read_concern(rcLocal);
                opts.read_preference(rpPrimary);
                session.with_transaction(callback, opts);

                return removeResultCode;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();

                return -1;
            }
        }

        int AccountOp::updateCharacter(const message::CharacterInfo &characterInfo, bool last)
        {
            auto client = mPool.acquire();

            auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(MakeDocHelper::makeDocAccountAndCharacter(characterInfo.ACCOUNT, characterInfo.NAME));

            if (!findResult)
            {
                return 0;
            }
            message ::CharacterBaseInfo characterBaseInfo;
            characterBaseInfo.CHARACTER_NAME = characterInfo.NAME;
            characterBaseInfo.CHARACTER_LEVEL = characterInfo.LEVEL;
            characterBaseInfo.CHARACTER_GENDER = characterInfo.GENDER;
            characterBaseInfo.CHARACTER_OCCUPTION = characterInfo.OCCUPTION;
            characterBaseInfo.CHARACTER_STATE = characterInfo.STATE;

            mongocxx::write_concern wcMajority{};
            wcMajority.acknowledge_level(mongocxx::write_concern::level::k_majority);

            mongocxx::read_concern rcLocal{};
            rcLocal.acknowledge_level(mongocxx::read_concern::level::k_local);

            mongocxx::read_preference rpPrimary{};
            rpPrimary.mode(mongocxx::read_preference::read_mode::k_primary);

            mongocxx::client_session::with_transaction_cb callback = [&](mongocxx::client_session *session)
            {
                if (last)
                {
                    client->database(mDBName).collection(mAccountCollectionName).update_one(*session, make_document(kvp("account", characterInfo.ACCOUNT)), MakeDocHelper::makeDocAccountLogout());
                }

                client->database(mDBName).collection(mAccountCollectionName).update_one(*session, make_document(kvp("account", characterInfo.ACCOUNT), kvp("characters.character_name", characterInfo.NAME)), MakeDocHelper::makeDocAccountUpdateCharacter(characterBaseInfo));

                client->database(mDBName).collection(mCharacterCollectionName).update_one(*session, make_document(kvp("account", characterInfo.ACCOUNT), kvp("base_info.name", characterInfo.NAME)), MakeDocHelper::makeDocUpdateCharacter(characterInfo));
            };

            auto session = client->start_session();

            try
            {
                mongocxx::options::transaction opts;
                opts.write_concern(wcMajority);
                opts.read_concern(rcLocal);
                opts.read_preference(rpPrimary);
                session.with_transaction(callback, opts);

                return 1;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();

                return -1;
            }
        }

        int AccountOp::getCharacterNum(const std::string &accountName)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return -1;
            }

            mongocxx::pipeline stages;

            stages.match(make_document(kvp("account", accountName)))
                .project(make_document(kvp("character num", make_document(kvp("$size", "$characters")))));
            try
            {
                auto result = client->database(mDBName).collection(mAccountCollectionName).aggregate(stages);
                if (result.begin() != result.end())
                {
                    bsoncxx::document::element characterNum = (*result.begin())["character num"];
                    // skip the characters default placeholder
                    return characterNum.get_int32().value - 1;
                }
                return 0;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();

                return -1;
            }
        }

        std::optional<std::vector<std::string>> AccountOp::getAllCharacterName(const std::string &accountName)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return std::nullopt;
            }

            std::vector<std::string> res = {};

            mongocxx::pipeline stages;

            stages.match(make_document(kvp("account", accountName)))
                .project(make_document(kvp("character member", make_document(kvp("$slice", make_array("$characters", 1, 100)))))); // TODO magic num 100

            try
            {
                auto result = client->database(mDBName).collection(mAccountCollectionName).aggregate(stages);

                if (result.begin() != result.end())
                {
                    bsoncxx::document::element characters = (*result.begin())["character member"];

                    auto characterArray = characters.get_array().value;

                    for (auto &character : characterArray)
                    {
                        std::string x = std::string(character["character_name"].get_string().value);
                        res.push_back(x);
                    }
                }

                return std::make_optional<std::vector<std::string>>(res);
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        std::optional<std::vector<schema::player::CharacterBaseInfo>> AccountOp::getAllCharacterBaseInfo(const std::string &accountName)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return std::nullopt;
            }

            std::vector<schema::player::CharacterBaseInfo> res = {};

            mongocxx::pipeline stages;

            stages.match(make_document(kvp("account", accountName)))
                .project(make_document(kvp("character member", make_document(kvp("$slice", make_array("$characters", 1, 100)))))); // TODO magic num 100

            try
            {
                auto result = client->database(mDBName).collection(mAccountCollectionName).aggregate(stages);

                if (result.begin() != result.end())
                {
                    bsoncxx::document::element characters = (*result.begin())["character member"];

                    auto characterArray = characters.get_array().value;

                    for (auto &character : characterArray)
                    {
                        schema::player::CharacterBaseInfo baseInfo;
                        baseInfo.mName = std::string(character["character_name"].get_string().value);
                        baseInfo.mLevel = character["character_level"].get_int32().value;
                        baseInfo.mOccuption = character["character_occuption"].get_int32().value;
                        baseInfo.mState = character["character_state"].get_int32().value;

                        res.push_back(baseInfo);
                    }
                }

                return std::make_optional<std::vector<schema::player::CharacterBaseInfo>>(res);
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        std::optional<std::vector<message::CharacterBaseInfo>> AccountOp::getAllCharacterBaseInfoFill2Message(const std::string &accountName)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return std::nullopt;
            }

            std::vector<message::CharacterBaseInfo> res = {};

            mongocxx::pipeline stages;

            stages.match(make_document(kvp("account", accountName)))
                .project(make_document(kvp("character member", make_document(kvp("$slice", make_array("$characters", 1, 100)))))); // TODO magic num 100

            try
            {
                auto result = client->database(mDBName).collection(mAccountCollectionName).aggregate(stages);
                if (result.begin() != result.end())
                {
                    bsoncxx::document::element characters = (*result.begin())["character member"];

                    auto characterArray = characters.get_array().value;

                    for (auto &character : characterArray)
                    {
                        message::CharacterBaseInfo baseInfo;
                        baseInfo.CHARACTER_NAME = std::string(character["character_name"].get_string().value);
                        baseInfo.CHARACTER_LEVEL = character["character_level"].get_int32().value;
                        baseInfo.CHARACTER_GENDER = character["character_gender"].get_bool().value;
                        baseInfo.CHARACTER_OCCUPTION = character["character_occuption"].get_int32().value;
                        baseInfo.CHARACTER_STATE = character["character_state"].get_int32().value;

                        res.push_back(baseInfo);
                    }
                }

                return std::make_optional<std::vector<message::CharacterBaseInfo>>(res);
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        std::optional<message::CharacterBaseInfo> AccountOp::getOneCharacterBaseInfoFill2Message(const std::string &accountName,
                                                                                                 const std::string &characterName)
        {
            message::CharacterBaseInfo baseInfo;

            auto client = mPool.acquire();
            if (!client)
            {
                return std::nullopt;
            }

            mongocxx::options::find opts{};
            opts.projection(make_document(
                kvp("characters", make_document(
                                      kvp("$elemMatch", make_document(
                                                            kvp("character_name", characterName)))))));

            try
            {
                auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(make_document(kvp("account", accountName)), opts);
                if (findResult)
                {
                    auto array = findResult.value()["characters"].get_array().value;

                    baseInfo.CHARACTER_NAME = characterName;
                    baseInfo.CHARACTER_LEVEL = (*array.begin())["character_level"].get_int32().value;
                    baseInfo.CHARACTER_GENDER = (*array.begin())["character_gender"].get_bool().value;
                    baseInfo.CHARACTER_OCCUPTION = (*array.begin())["character_occuption"].get_int32().value;
                    baseInfo.CHARACTER_STATE = (*array.begin())["character_state"].get_int32().value;

                    return std::make_optional<message::CharacterBaseInfo>(baseInfo);
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return std::nullopt;
            }
        }

        int AccountOp::matchAccount(const std::string &accountName, const std::string &password)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return -1;
            }
            try
            {
                auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(make_document(kvp("account", accountName), kvp("password", password)));

                if (findResult)
                    return 1;

                return 0;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        int AccountOp::matchAccountAndUpdateLoginState(const std::string &accountName, const std::string &password, bool tryLogin)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return -1;
            }

            try
            {
                auto findResult =
                    client->database(mDBName)
                        .collection(mAccountCollectionName)
                        .find_one_and_update(
                            make_document(
                                kvp("account", accountName),
                                kvp("password", password)),
                            make_document(
                                kvp("$set",
                                    make_document(
                                        kvp("on_off", tryLogin))),
                                kvp("$set", make_document(
                                                kvp("last_login_time", bsoncxx::types::b_date(std::chrono::system_clock::now()))))));

                if (findResult)
                {
                    bool oldState = (*findResult)["on_off"].get_bool().value;

                    // std::cout << "old State:" << std::boolalpha << oldState << std::endl;

                    // old state "on" means account is already on, return 2 for AccountAlreadyLogin error
                    return oldState ? 2 : 1;
                }

                return 0;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

        int AccountOp::matchAccountAndCharacter(const std::string &accountName, const std::string &characterName)
        {
            auto client = mPool.acquire();
            if (!client)
            {
                return -1;
            }
            try
            {
                auto findResult = client->database(mDBName).collection(mAccountCollectionName).find_one(make_document(kvp("account", accountName), kvp("characters.character_name", characterName)));

                if (findResult)
                    return 1;

                return 0;
            }
            catch (const mongocxx::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }
    }
}