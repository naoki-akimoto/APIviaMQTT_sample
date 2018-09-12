#ifndef MQTT_KIIAPI_HPP
#define MQTT_KIIAPI_HPP

#include <mosquittopp.h>
#include <string>
#include <map>
#include <utility>

#include "picojson.h"

#include "APIBrokerInfo.h"

enum Status {
    ERROR = -1,
    CONNECT_DEFAULT,
    CONNECT_API,
    STAND_BY_OK,
};

typedef void (*CB_success_t)(picojson::value &result);
typedef void (*CB_fail_t)(picojson::value &result);

class MQTT_KiiAPI : public mosqpp::mosquittopp
{
    private:
        std::string appId;
        std::string appKey;
        std::string vendorThingID;
        std::string thingPassword;
        APIBrokerInfo apiBrokerInfo;
        Status status;
        std::map<std::string, std::pair<CB_success_t, CB_fail_t> > cbMap;

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

        bool waitForReady();

        void registerState(CB_success_t cb_success, CB_fail_t cb_fail);
        void getState(CB_success_t cb_success, CB_fail_t cb_fail);
};

#endif//MQTT_KIIAPI_HPP
