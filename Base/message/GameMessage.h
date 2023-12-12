#ifndef __GAME_SERVER_MESSAGE_H__
#define __GAME_SERVER_MESSAGE_H__

#include "CustomMessages.h"

namespace message
{
    namespace code
    {
        //
        namespace game
        {
            const size_t GAME_DS_REGISTER_CODE = 210;                // ds register
            const size_t GAME_DS_QUIT_CODE = 220;                    // ds quit
            const size_t GAME_DS_UPDATE_CODE = 230;                  // ds update ds's info
            const size_t GAME_LOAD_CHARACTER_CODE = 240;             // load one character's info
            const size_t GAME_UPLOAD_CHARACTER_CODE = 250;           // upload character's info
            const size_t GAME_LAST_UPLOAD_CHARACTER_CODE = 251;      // upload character's info when logout
                                                                     //
            const size_t GAME_DS_REGISTER_SUCCESS = 10210;           // reply ds register success
            const size_t GAME_DS_QUIT_SUCCESS = 10220;               // reply ds quit scuuess
            const size_t GAME_DS_UPDATE_SUCCESS = 10230;             // reply ds update success
            const size_t GAME_LOAD_CHARACTER_SUCCESS = 10240;        // reply load one character's info success
            const size_t GAME_UPLOAD_CHARACTER_SUCCESS = 10250;      // reply upload character success
            const size_t GAME_LAST_UPLOAD_CHARACTER_SUCCESS = 10251; // reply last upload character success
                                                                     //
            const size_t GAME_DS_REGISTER_ERROR = 30210;             // reply ds register error
            const size_t GAME_DS_QUIT_ERROR = 30220;                 // reply ds quit error
            const size_t GAME_DS_UPDATE_ERROR = 30230;               // reply ds update error
            const size_t GAME_LOAD_CHARACTER_ERROR = 30240;          // reply load character error

            const size_t GAME_DS_UNKNOWN_ERROR = 30999;
        }
    }
}

namespace message
{
    // load or upload character's all info
    struct DSLoadOrUploadCharacterMessage : public TimestampMessage
    {
        size_t CODE;

        int SCENE_ID;

        std::string ACCOUNT;

        std::string CHARACTER_NAME;

        std::string CHARACTER_DATA;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual DSLoadOrUploadCharacterMessage &getRef() override { return *this; }
    };

#if 0
    // upload character's part info
    struct DSUploadCharacterPartMessage : public TimestampMessage
    {
        size_t CODE;

        std::string ACCOUNT;

        std::string CHARACETR_NAME;

        std::vector<int> FIELD_CODES;
        std::vector<double> VALUES;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual DSUploadCharacterPartMessage &getRef() override { return *this; }
    };
#endif

    struct DSMessage : public TimestampMessage
    {
        size_t CODE;

        int DS_PORT;

        int SCENE_ID;

        int LOAD;

        std::string SCENE_DATA;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual DSMessage &getRef() override { return *this; }
    };

    struct CharacterInfo : public BaseMessage
    {
        std::string ACCOUNT;

        std::string NAME;
        int LEVEL;
        bool GENDER;
        int OCCUPTION;
        int STATE;

        int EXP;
        int HEALTH;
        int MANA;
        int ARMOR;
        int STAMINA;
        int STRENGTH;
        int AGILITY;
        int INTELLECT;

        /*
        TODO more attributes
        */

        /*
        TODO more owned items
        */

        /*
        TODO more skills
        */

        /*
        TODO more quest processes
        */

        int SCENE_ID;
        double SCENE_X;
        double SCENE_Y;
        double SCENE_Z;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual CharacterInfo &getRef() override { return *this; }
    };

#if 0
    struct DSInfo :public BaseMessage
    {
        
    };
#endif
}

namespace message
{
    namespace message_carrier
    {

        struct GameMessage : public BaseMessage
        {
            std::string CLIENT_ID;

            std::string CLIENT_ADDRESS;

            void FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const override;

            void FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector) override;

        private:
            const size_t TAG_NUM = 6;
        };

    }
}

#endif