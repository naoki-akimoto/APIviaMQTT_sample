#ifndef MQTT_KIIAPI_HPP
#define MQTT_KIIAPI_HPP

#include <mosquittopp.h>
#include <string>

#include "APIBrokerInfo.h"

enum Status {
    ERROR = -1,
    CONNECT_DEFAULT,
    CONNECT_API,
    STAND_BY_OK,
};

class MQTT_KiiAPI : public mosqpp::mosquittopp
{
    private:
        std::string appId;
        std::string appKey;
        std::string vendorThingID;
        std::string thingPassword;
        APIBrokerInfo apiBrokerInfo;
        Status status;

        void on_connect(int rc);
        void on_disconnect(int rc);
        void on_message(const struct mosquitto_message *msg);

    public:
        MQTT_KiiAPI(
                const char *host,
                int port,
                const char *appId,
                const char *appKey,
                const char *vendorThingID,
                const char *thingPassword);
        ~MQTT_KiiAPI();

        bool waitForStandby();
};

#endif//MQTT_KIIAPI_HPP
