#ifndef __MESSAGE_CUSTOM_MESSAGE_H__
#define __MESSAGE_CUSTOM_MESSAGE_H__

#include "BaseMessage.h"
#include <chrono>

namespace message
{
    // as heartbeat message or inherited communication message
    struct TimestampMessage : public BaseMessage
    {
        time_t GET_MILLISECONDS_SINCE_EPOCH() const { return MILLISECONDS_SINCE_EPOCH; }

        virtual void count() override;

        virtual std::string serialize() override;

        virtual void deserialize(const char *data) override;

        virtual TimestampMessage &getRef() override { return *this; }

    protected:
        static time_t getMillisecondsSinceEpoch();

    protected:
        time_t MILLISECONDS_SINCE_EPOCH = getMillisecondsSinceEpoch();
    };

    struct HeartBeatPacket : public TimestampMessage
    {
        virtual HeartBeatPacket &getRef() override { return *this; }
    };

    namespace message_carrier
    {
        const std::string TYPE_TAG = "CARRIER_TYPE";
        const std::string SERVICE_TAG = "CARRIER_SERVICE";
        const std::string NEXT_SERVICE_TAG = "CARRIER_NEXT_SERVICE";
        const std::string MESSAGE_TAG = "CARRIER_MESSAGE";
        const std::string CLIENT_ID_TAG = "CARRIER_CLIENT_ID_TAG";
        const std::string CLIENT_ADDRESS_TAG = "CARRIER_CLIENT_ADDRESS_TAG";

        struct BaseMessage
        {
            std::string TYPE;

            std::string SERVICE;

            std::string NEXT_SERVICE;

            std::string MESSAGE;

            virtual void FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const;

            virtual void FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector);

        private:
            const size_t TAG_NUM = 4;
        };

    }

    namespace test
    {
        struct TestVecNormal : public BaseMessage
        {
            std::vector<int> INTS;

            std::vector<double> DOUBLES;

            virtual void count() override;

            virtual std::string serialize() override;

            virtual void deserialize(const char *data) override;

            virtual TestVecNormal &getRef() override { return *this; }
        };

        struct TestStringsMessage : public BaseMessage
        {
            bool BOOL = true;

            std::vector<size_t> INTS;

            std::vector<std::string> STRINGS;

            virtual void count() override;

            virtual std::string serialize() override;

            virtual void deserialize(const char *data) override;

            virtual TestStringsMessage &getRef() override { return *this; }
        };

        // used for pack various of messages
        struct MessagesPacker : public TimestampMessage
        {
            std::vector<std::string> MESSAGES;

            virtual void count() override;

            virtual std::string serialize() override;

            virtual void deserialize(const char *data) override;

            virtual MessagesPacker &getRef() override { return *this; }
        };

        struct CLoginMessage : public TimestampMessage
        {
            size_t requestCode = 0;

            std::string account = {};
            std::string password = {};

            virtual void count() override;

            virtual std::string serialize() override;

            virtual void deserialize(const char *data) override;

            virtual CLoginMessage &getRef() override { return *this; }
        };

        struct SLoginMessage : public TimestampMessage
        {
            size_t requestResult = 0;

            std::string token = {};

            virtual void count() override;

            virtual std::string serialize() override;

            virtual void deserialize(const char *data) override;

            virtual SLoginMessage &getRef() override { return *this; }
        };
    }
}

#endif
