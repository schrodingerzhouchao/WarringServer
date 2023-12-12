#include "Database.h"

namespace db_server
{
    namespace db
    {
        void Database::initDBCollName(const std::string &dbName, const std::string &accountCollName, const std::string &characterCollName)
        {
            mDBName = dbName;
            mAccountCollectionName = accountCollName;
            mCharacterCollectionName = characterCollName;
            mAccountOp.initDBCollName(mDBName, mAccountCollectionName, mCharacterCollectionName);
            mCharacterOp.initDBCollName(mDBName, mAccountCollectionName, mCharacterCollectionName);
        }

        void Database::check()
        {
            assert(!mDBName.empty());
            assert(!mAccountCollectionName.empty());
            assert(!mCharacterCollectionName.empty());

            assert(mPool.acquire());
        }

        bool Database::createIndex()
        {
            return mAccountOp.createIndex() && mCharacterOp.createIndex();
        }

        int Database::addNewAccount(const std::string &accountName, const std::string &password, const std::string &address)
        {
            return mAccountOp.addNewAccount(accountName, password, address);
        }

        int Database::addNewCharacter(const std::string &accountName, const schema::player::CharacterBaseInfo &newCharacter)
        {
            return mAccountOp.addNewCharacter(accountName, newCharacter);
        }

        int Database::addNewCharacter(const message::LoginCharacterRequestMessage &newCharacter)
        {
            return mAccountOp.addNewCharacter(newCharacter);
        }

        int Database::removeCharacter(const std::string &accountName, const std::string &characterName)
        {
            return mAccountOp.removeCharacter(accountName, characterName);
        }
        int Database::updateCharacter(const message::CharacterInfo &characterInfo, bool last)
        {
            mAccountOp.updateCharacter(characterInfo, last);
        }

        int Database::getCharacterNum(const std::string &accountName)
        {
            return mAccountOp.getCharacterNum(accountName);
        }

        std::optional<std::vector<std::string>> Database::getAllCharacterName(const std::string &accountName)
        {
            return mAccountOp.getAllCharacterName(accountName);
        }

        std::optional<std::vector<schema::player::CharacterBaseInfo>> Database::getAllCharacterBaseInfo(const std::string &accountName)
        {
            return mAccountOp.getAllCharacterBaseInfo(accountName);
        }

        std::optional<std::vector<message::CharacterBaseInfo>> Database::getAllCharacterBaseInfoFill2Message(const std::string &accountName)
        {
            return mAccountOp.getAllCharacterBaseInfoFill2Message(accountName);
        }

        std::optional<message::CharacterBaseInfo> Database::getOneCharacterBaseInfoFill2Message(const std::string &accountName, const std::string &characterName)
        {
            return mAccountOp.getOneCharacterBaseInfoFill2Message(accountName, characterName);
        }

        std::optional<message::CharacterInfo> Database::getOneCharacterInfoFill2Message(const std::string &accountName, const std::string &characterName)
        {
            return mCharacterOp.getOneCharacterInfoFill2Message(accountName, characterName);
        }

        std::string Database::getOneCharacterInfoFill2Data(const std::string &accountName, const std::string &characterName)
        {
            return mCharacterOp.getOneCharacterInfoFill2Data(accountName, characterName);
        }

        int Database::matchAccount(const std::string &accountName, const std::string &password)
        {
            return mAccountOp.matchAccount(accountName, password);
        }

        int Database::matchAccountAndUpdateLoginState(const std::string &accountName, const std::string &password, bool tryLogin)
        {
            return mAccountOp.matchAccountAndUpdateLoginState(accountName, password, tryLogin);
        }

        int Database::matchAccountAndCharacter(const std::string &accountName, const std::string &characterName)
        {
            return mAccountOp.matchAccountAndCharacter(accountName, characterName);
        }

    }
}