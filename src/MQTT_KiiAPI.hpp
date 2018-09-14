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

class MQTT_KiiAPI;

typedef void (*CB_command_t)(MQTT_KiiAPI &api, picojson::value &command);
typedef void (*CB_success_t)(std::string &requestID, picojson::value &result);
typedef void (*CB_fail_t)(std::string &requestID, picojson::value &result);

class MQTT_KiiAPI : public mosqpp::mosquittopp
{
    private:
        std::string appId;
        std::string appKey;
        std::string vendorThingID;
        std::string thingPassword;
        APIBrokerInfo apiBrokerInfo;
        Status status;
        CB_command_t commandCB;
        CB_success_t successCB;
        CB_fail_t failCB;

        void on_connect(int rc);
        void on_disconnect(int rc);
        void on_message(const struct mosquitto_message *msg);
        void parseAPIBrokerInfo(std::string &msg);
        void parseCommand(std::string &msg);
        void parseAPIResponse(std::string &msg);

    public:
        MQTT_KiiAPI(
                const char *host,
                int port,
                const char *appId,
                const char *appKey,
                const char *vendorThingID,
                const char *thingPassword,
                CB_command_t cb_command,
                CB_success_t cb_success,
                CB_fail_t cb_fail);
        ~MQTT_KiiAPI();

        bool waitForReady();

        void registerState(std::string &requestID, picojson::value &state);
        void getState(std::string &requestID);
        void getCommandList(std::string &requestID);
        void executeCommand(std::string &requestID, picojson::value &command);
        void sendActionResults(std::string &requestID, std::string &commandID, picojson::value &results);
};

#endif//MQTT_KIIAPI_HPP
