#ifndef __DBSERVER_DB_MONGODB_SCHEMA_H__
#define __DBSERVER_DB_MONGODB_SCHEMA_H__

#include <string>
#include <vector>

namespace schema
{
    namespace item
    {
        struct Item
        {
            int mID;
            int mCurrentStack;
            bool mBound;
        };

        struct Equipment : public Item
        {
            int mCurrentAbrasion;
        };
    }

    namespace skill
    {
        struct Skill
        {
            int mID;             
        };
    }

    namespace player
    {
        struct CharacterBaseInfo
        {
            std::string mName;
            int mLevel;
            bool mGender;
            int mOccuption;
            int mState;
        };

        struct Account
        {
            std::string mAccount;
            std::string mPassword;
            std::vector<CharacterBaseInfo> mCharacters;
            std::string mLastLoginTime;
            std::string mLastLogoffTime;
            std::string mLastLoginAddress;
        };

        struct CharacterBaseAttribute
        {
            int mHealth;
            int mStamina;
            int mMana;
            int mExperience;

            // TODO more attributes
        };

        struct Character
        {
            std::string mAccount;
            CharacterBaseInfo mCharacterBaseInfo;
            
            CharacterBaseAttribute mBaseAttribute;

            std::vector<item::Equipment *> mEquipped;
            std::vector<item::Item *> mBackpack;
            std::vector<item::Item *> mInventory;
        };
    }

}

#endif
