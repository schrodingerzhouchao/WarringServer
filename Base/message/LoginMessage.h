#ifndef __LOGIN_SERVER_MESSAGE_H__
#define __LOGIN_SERVER_MESSAGE_H__

#include "CustomMessages.h"

namespace message
{
    namespace code
    {
        const size_t NONE = 0;

        // xxx10 ~ xxx99
        namespace login
        {
            const size_t LOGIN_REGISTER_REQUEST_CODE = 10; // request register
            const size_t LOGIN_LOGIN_REQUEST_CODE = 20;    // request login
            const size_t LOGIN_OFFLINE_REQUEST_CODE = 30;  // request off line
            const size_t LOGIN_LOGOUT_REQUEST_CODE = 40;   // request logout

            const size_t LOGIN_CREATE_CHARACTER_CODE = 50;     // request create character
            const size_t LOGIN_DELETE_CHARACTER_CODE = 60;     // request delete character
            const size_t LOGIN_CHARACTER_ENTER_GAME_CODE = 70; // reuqest character enter game world

            const size_t LOGIN_REGISTER_RESULT_SUCCESS = 10010; // reply register success
            const size_t LOGIN_LOGIN_RESULT_SUCCESS = 10020;    // reply login scuuess

            const size_t LOGIN_CREATE_CHARACTER_RESULT_SUCCESS = 10050; // reply create character success
            const size_t LOGIN_DELETE_CHARACTER_RESULT_SUCCESS = 10060; // reply delete character success
            const size_t LOGIN_CHARACTER_ENTER_SUCCESS = 10070;         // reply character enter game world success
            const size_t LOGIN_CHARACTER_ENTERING = 10071;              // reply character enter game world success

            const size_t LOGIN_REGISTER_ERROR_ACCOUNT_EXISTED = 30011;        // reply register error with account is already existed
            const size_t LOGIN_LOGIN_ERROR_ACCOUNT_NO_EXISTED = 30021;        // reply login error with account is not existed
            const size_t LOGIN_LOGIN_ERROR_ACCOUNT_PASSWORD_MISMATCH = 30022; // reply login error with account is not match with password
            const size_t LOGIN_LOGIN_ERROR_ACCOUNT_ALREADY_ON = 30023;        // reply login error with account is already on

            const size_t LOGIN_CREATE_CHARACTER_ERROR_EXISTED = 30051;    // reply create character error with character is already existed
            const size_t LOGIN_DELETE_CHARACTER_ERROR_NO_EXISTED = 30061; // reply delete character error with character is not existed
            const size_t LOGIN_CHARACTER_ERROR_ACCOUNT_MISMATCH = 30071;  // reply character is not match with account
            const size_t LOGIN_CHARACTER_ERROR_ENTER_GAME = 30072;        // reply character enter game world error

            const size_t LOGIN_ERROR_NO_SUCH_SERVICE = 30098; // reply error with no such service
            const size_t LOGIN_ERROR_FROM_DB = 30099;         // reply error with unknown db case

            const std::string MESSAGE_TYPE_LOGIN = "LOGIN_MESSAGE_TYPE";
            const std::string MESSAGE_TYPE_CHARACTER_BASE_INFO = "CHARACTER_BASE_INFO_MESSAGE_TYPE";

            const std::string LOGIN_TOKEN_NONE = "";

            static std::string CODE_MESSAGE(size_t code);
        }
    }
}

namespace message
{
    struct CharacterBaseInfo : public BaseMessage
    {
        std::string CHARACTER_NAME;
        int CHARACTER_LEVEL;
        bool CHARACTER_GENDER;
        int CHARACTER_OCCUPTION;
        int CHARACTER_STATE;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual CharacterBaseInfo &getRef() override { return *this; }
    };

    struct LoginRequestMessage : public TimestampMessage
    {
        size_t REQUEST_CODE = code::NONE;

        std::string ACCOUNT;
        std::string PASSWORD;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual LoginRequestMessage &getRef() override { return *this; }
    };

    struct LoginResponseMessage : public TimestampMessage
    {
        size_t RESPONSE_CODE = code::NONE;

        std::string TOKEN;

        std::string GAME_GATEWAY_IP;

        int GAME_GATEWAY_PORT;

        std::string GAME_DS_IP;

        int GAME_DS_PORT;

        std::vector<std::string> CHARACTER_BASE_INFO_MESSAGES;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual LoginResponseMessage &getRef() override { return *this; }
    };

    struct LoginCharacterRequestMessage : public LoginRequestMessage
    {
        std::string CHARACTER_NAME;

        bool CHARACTER_GENDER;

        int CHARACTER_OCCUPTION;

        int CHARACTER_STATE;

        void initAccount(const LoginRequestMessage &src, const std::string &msgName);

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual LoginCharacterRequestMessage &getRef() override { return *this; }
    };

}

namespace message
{
    namespace message_carrier
    {
        

        struct LoginMessage : public BaseMessage
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