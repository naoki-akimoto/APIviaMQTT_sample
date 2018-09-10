#include "MQTT_KiiAPI.hpp"

#include <iostream>
#include <string>

#include "picojson.h"

using namespace std;

#define KEEP_ALIVE 60

MQTT_KiiAPI::MQTT_KiiAPI(
        const char *host,
        int port,
        const char *appId,
        const char *appKey,
        const char *vendorThingID,
        const char *thingPassword)
    : mosquittopp("anonymous")
{
    string username;
    string password;

    this->appId = string(appId);
    this->appKey = string(appKey);
    this->vendorThingID = string(vendorThingID);
    this->thingPassword = string(thingPassword);
    this->connectAPIBroker = false;

    username = "type=oauth2&client_id=" + this->appId;
    password = "client_secret=" + this->appKey;

    mosqpp::lib_init();
    username_pw_set(username.c_str(), password.c_str());
    connect_async(host, port, KEEP_ALIVE);
    loop_start();
}

MQTT_KiiAPI::~MQTT_KiiAPI()
{
    disconnect();
    loop_stop();
    mosqpp::lib_cleanup();
}

void MQTT_KiiAPI::on_connect(int rc)
{
    if (rc == 0) {
        if (!this->connectAPIBroker) {
            string topic;
            string payload;

            topic = "p/anonymous/thing-if/apps/" + this->appId + "/onboardings";
            payload = string("POST\n") +
                "Content-Type: application/vnd.kii.OnboardingWithVendorThingIDByThing+json\n" +
                "\n" +
                "{" +
                "\"vendorThingID\":\"" + this->vendorThingID + "\"," +
                "\"thingPassword\":\"" + this->thingPassword + "\"" +
                "}";
            publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
        } else {
            cout << "succeed to connect API broker." << endl;
        }
    } else {
        cout << "Failed to connect default broker. (" << rc << ")" << endl;
    }
}

void MQTT_KiiAPI::on_disconnect(int rc)
{
    if (rc == 0) {
        if (!this->connectAPIBroker) {
            this->connectAPIBroker = true;

            APIBrokerInfo &info = this->apiBrokerInfo;
            reinitialise(info.clientID.c_str(), true);
            username_pw_set(info.username.c_str(), info.password.c_str());
            connect_async(info.host.c_str(), info.port, KEEP_ALIVE);
        }
    }
}

void MQTT_KiiAPI::on_message(const struct mosquitto_message *msg)
{
    string topic = "p/anonymous/thing-if/apps/" + this->appId + "/onboardings";
    if (topic != msg->topic) {
        cout << "'" << msg->topic << "' is not for onboardings." << endl;
        return;
    }
    if (msg->payloadlen > 0) {
        string body = string((const char*)msg->payload);
        if (body.find("200\r\n") == 0) {
            string json = body.substr(body.find("\r\n\r\n") + 4);
            picojson::value v;
            const string err = picojson::parse(v, json);
            if (err.empty()) {
                picojson::object &obj = v.get<picojson::object>();
                picojson::object &endPoint = obj["mqttEndpoint"].get<picojson::object>();
                APIBrokerInfo &info = this->apiBrokerInfo;
                info.accessToken = obj["accessToken"].get<string>();
                info.thingID = obj["thingID"].get<string>();
                info.clientID = endPoint["mqttTopic"].get<string>();
                info.host = endPoint["host"].get<string>();
                info.port = endPoint["portTCP"].get<double>();
                info.username = endPoint["username"].get<string>();
                info.password = endPoint["password"].get<string>();

                disconnect();
            } else {
                cout << err << endl;
            }
        }
    }
}

