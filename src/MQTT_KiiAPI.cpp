#include "MQTT_KiiAPI.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>

using namespace std;

#define KEEP_ALIVE 60

MQTT_KiiAPI::MQTT_KiiAPI(
        const char *host,
        int port,
        const char *appId,
        const char *appKey,
        const char *vendorThingID,
        const char *thingPassword,
        CB_command_t cb_command,
        CB_success_t cb_success,
        CB_fail_t cb_fail)
    : mosquittopp("anonymous")
{
    string username;
    string password;

    this->appId = string(appId);
    this->appKey = string(appKey);
    this->vendorThingID = string(vendorThingID);
    this->thingPassword = string(thingPassword);
    this->status = CONNECT_DEFAULT;
    this->commandCB = cb_command;
    this->successCB = cb_success;
    this->failCB = cb_fail;

    username = "type=oauth2&client_id=" + this->appId;
    password = "client_secret=" + this->appKey;

    mosqpp::lib_init();
    username_pw_set(username.c_str(), password.c_str());
    connect_async(host, port, KEEP_ALIVE);
    loop_start();
}

MQTT_KiiAPI::~MQTT_KiiAPI()
{
    if (this->status == STAND_BY_OK) {
        unsubscribe(NULL, this->apiBrokerInfo.clientID.c_str());
    }
    disconnect();
    loop_stop();
    mosqpp::lib_cleanup();
}

void MQTT_KiiAPI::on_connect(int rc)
{
    if (rc == 0) {
        if (this->status == CONNECT_DEFAULT) {
            string topic;
            string payload;

            topic = "p/anonymous/thing-if/apps/" + this->appId + "/onboardings";
            payload =
                "POST\n"
                "Content-Type: application/vnd.kii.OnboardingWithVendorThingIDByThing+json\n"
                "\n"
                "{"
                "  \"vendorThingID\":\"" + this->vendorThingID + "\","
                "  \"thingPassword\":\"" + this->thingPassword + "\""
                "}";
            publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
        } else {
            this->status = STAND_BY_OK;
            subscribe(NULL, this->apiBrokerInfo.clientID.c_str(), 0);
        }
    } else {
        this->status = ERROR;
        cout << "Failed to connect default broker. (" << rc << ")" << endl;
    }
}

void MQTT_KiiAPI::on_disconnect(int rc)
{
    if (rc == 0) {
        if (this->status == CONNECT_DEFAULT) {
            this->status = CONNECT_API;

            APIBrokerInfo &info = this->apiBrokerInfo;
            reinitialise(info.clientID.c_str(), true);
            username_pw_set(info.username.c_str(), info.password.c_str());
            connect_async(info.host.c_str(), info.port, KEEP_ALIVE);
        }
    }
}

void MQTT_KiiAPI::on_message(const struct mosquitto_message *msg)
{
    if (this->status == CONNECT_DEFAULT) {
        string topic = "p/anonymous/thing-if/apps/" + this->appId + "/onboardings";
        if (topic != msg->topic) {
            this->status = ERROR;
            cout << "'" << msg->topic << "' is not for onboardings." << endl;
            return;
        }
        if (msg->payloadlen > 0) {
            string body = string((const char*)msg->payload);
            parseAPIBrokerInfo(body);
        }
    } else {
        if (msg->payloadlen > 0) {
            string body = string((const char*)msg->payload);
            if (msg->topic == this->apiBrokerInfo.clientID) {
                // Received command.
                parseCommand(body);
            } else {
                // API publish response.
                parseAPIResponse(body);
            }
        }
    }
}

void MQTT_KiiAPI::parseAPIBrokerInfo(string &msg)
{
    if (msg.find("200\r\n") == 0) {
        string json = msg.substr(msg.find("\r\n\r\n") + 4);
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
            this->status = ERROR;
            cout << err << endl;
        }
    } else {
        this->status = ERROR;
        cout << "Failed to get API Broker infomation." << endl << msg << endl;
    }
}

void MQTT_KiiAPI::parseCommand(string &msg)
{
    picojson::value v;
    string err = picojson::parse(v, msg);
    if (err.empty()) {
        this->commandCB(*this, v);
    }
}

void MQTT_KiiAPI::parseAPIResponse(string &msg)
{
    // parse response status.
    istringstream iss(msg);
    int responseStatus;
    iss >> responseStatus;

    // parse X-Kii-RequestID.
    int idIndex = msg.find("X-Kii-RequestID:") + 16;
    string requestID = msg.substr(idIndex, msg.find("\r\n", idIndex) - idIndex);

    // parse response body json.
    picojson::value v;
    string json;
    string err;
    int bodyIndex = msg.find("\r\n\r\n");
    if (bodyIndex > 0 && msg.length() > bodyIndex + 4) {
        json = msg.substr(bodyIndex + 4);
        err = picojson::parse(v, json);
    }
    if (err.empty()) {
        if (responseStatus >= 200 && responseStatus < 300) {
            if (this->successCB != NULL) {
                this->successCB(requestID, v);
            }
        } else {
            if (this->failCB != NULL) {
                this->failCB(requestID, v);
            }
        }
    } else {
        cout << err << endl << json << endl;
    }
}

bool MQTT_KiiAPI::waitForReady()
{
    while(this->status != ERROR && this->status != STAND_BY_OK) {
        this_thread::sleep_for(chrono::microseconds(100));
    }
    return this->status == STAND_BY_OK;
}

void MQTT_KiiAPI::registerState(string &requestID, picojson::value &state)
{
    APIBrokerInfo &info = this->apiBrokerInfo;
    string topic;
    string payload;

    requestID = "registerState";
    topic = "p/" + info.clientID + "/thing-if/apps/" + this->appId + "/targets/thing:" + info.thingID + "/states";
    payload =
        "PUT\n"
        "Authorization: Bearer " + info.accessToken + "\n"
        "X-Kii-RequestID: " + requestID + "\n"
        "Content-Type: application/json\n"
        "\n" +
        state.serialize();
    publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
}

void MQTT_KiiAPI::getState(string &requestID)
{
    APIBrokerInfo &info = this->apiBrokerInfo;
    string topic;
    string payload;

    requestID = "getState";
    topic = "p/" + info.clientID + "/thing-if/apps/" + this->appId + "/targets/thing:" + info.thingID + "/states";
    payload =
        "GET\n"
        "Authorization: Bearer " + info.accessToken + "\n"
        "X-Kii-RequestID: " + requestID + "\n"
        "\n";
    publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
}

void MQTT_KiiAPI::getCommandList(string &requestID)
{
    APIBrokerInfo &info = this->apiBrokerInfo;
    string topic;
    string payload;

    requestID = "getCommandList";
    topic = "p/" + info.clientID + "/thing-if/apps/" + this->appId + "/targets/thing:" + info.thingID + "/commands";
    payload =
        "GET\n"
        "Authorization: Bearer " + info.accessToken + "\n"
        "X-Kii-RequestID: " + requestID + "\n"
        "\n";
    publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
}

void MQTT_KiiAPI::executeCommand(string &requestID, picojson::value &command)
{
    APIBrokerInfo &info = this->apiBrokerInfo;
    string topic;
    string payload;

    requestID = "executeCommand";
    topic = "p/" + info.clientID + "/thing-if/apps/" + this->appId + "/targets/thing:" + info.thingID + "/commands";
    payload =
        "PUT\n"
        "Authorization: Bearer " + info.accessToken + "\n"
        "X-Kii-RequestID: " + requestID + "\n"
        "Content-Type: application/json\n"
        "\n" +
        command.serialize();
    publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
}

void MQTT_KiiAPI::sendActionResults(std::string &requestID, std::string &commandID, picojson::value &results)
{
    APIBrokerInfo &info = this->apiBrokerInfo;
    string topic;
    string payload;

    requestID = "sendActionResults";
    topic = "p/" + info.clientID + "/thing-if/apps/" + this->appId + "/targets/thing:" + info.thingID + "/commands/" + commandID + "/action-results";
    payload =
        "PUT\n"
        "Authorization: Bearer " + info.accessToken + "\n"
        "X-Kii-RequestID: " + requestID + "\n"
        "Content-Type: application/json\n"
        "\n" +
        results.serialize();
    publish(NULL, topic.c_str(), payload.size(), payload.c_str(), 1, false);
}

