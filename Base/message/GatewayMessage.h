#ifndef __GATEWAY_SERVER_MESSAGE_H__
#define __GATEWAY_SERVER_MESSAGE_H__

#include "CustomMessages.h"

namespace message
{
    namespace code
    {
        // xx110 ~ xx199
        namespace gateway
        {
            const size_t GATEWAY_ENTER_GAME_REQUEST_CODE = 110; // request enter game scene
            const size_t GATEWAY_EXIT_GAME_REQUEST_CODE = 120;  // request exit game scene

            // const size_t GATEWAY_GROUP_INVITE_REQUEST_CODE = ;
            // const size_t GATEWAY_GROUP_QUIT_REQUEST_CODE = ;

            // TODO more

            // const size_t GATEWAY_CHAT_PUBLIC_CHANNEL_CODE = ;
            // const size_t GATEWAY_CHAT_PRIVATE_CHANNEL_CODE = ;

            const size_t GATEWAY_ENTER_GAME_RESULT_SUCCESS = 10110; // reply enter game success
            const size_t GATEWAY_EXIT_GAME_RESULT_SUCCESS = 10120;  // reply exit game scuuess

            const size_t GATEWAY_ENTER_GAME_ERROR = 30110; // reply exit game error
            const size_t GATEWAY_EXIT_GAME_ERROR = 30120;  // reply exit game error
        }
    }
}

namespace message
{
#if 0
    struct CharacterInfo : public BaseMessage
    {
        // TODO
    };
#endif


    struct EnterGameRequestMessage : public TimestampMessage
    {
        size_t REQUEST_CODE;

        std::string ACCOUNT;
        std::string CHARACTER;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual EnterGameRequestMessage &getRef() override { return *this; }
    };

    struct EnterGameResponseMessage : public TimestampMessage
    {
        size_t RESPONSE_CODE;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual EnterGameResponseMessage &getRef() override { return *this; }
    };

    struct ChatRequestMessage : public TimestampMessage
    {
        size_t REQUEST_CODE;

        std::string ACCOUNT;
        std::string CHARACTER;

        std::string CHANNEL;

        std::string NOTE;

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual ChatRequestMessage &getRef() override { return *this; }
    };

}

namespace message
{
    namespace message_carrier
    {
        

        struct GatewayMessage : public BaseMessage
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