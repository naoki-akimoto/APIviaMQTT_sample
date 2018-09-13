#include "MQTT_KiiAPI.hpp"

#include <unistd.h>
#include <iostream>

#define SITE "mqtt-us.kii.com"
#define PORT 1883
#define APPID ""
#define APPKEY ""
#define VENDORTHINGID ""
#define THINGPASSWORD ""

using namespace std;

void successCB(string &requestID, picojson::value &v) {
    cout << "Request: " << requestID << " succeeded." << endl;
    if (!v.is<picojson::null>()) {
        cout << v.serialize(true) << endl;
    }
}

void failCB(string &requestID, picojson::value &v) {
    cout << "Request: " << requestID << " failed." << endl;
    if (!v.is<picojson::null>()) {
        cout << v.serialize(true) << endl;
    }
}

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD, successCB, failCB);

    cout << "Please wait for ready." <<endl;
    if (kiiApi->waitForReady()) {
        while(true) {
            string requestID;
            string command;
            cout << ">> ";
            getline(cin, command);
            if (command == "exit") {
                break;
            } else if (command == "help") {
                cout << "command list" << endl;
                cout << "\texit\t\texit this application." << endl;
                cout << "\thelp\t\tshow this help." << endl;
                cout << "\tresiter\t\tregister state." << endl;
                cout << "\tstate\t\tget state." << endl;
                cout << "\tcommands\t\tget command list." << endl;
                cout << "\texecute\t\texecute command." << endl;
            } else if (command == "register") {
                string json =
                    "{"
                    "  \"power\":true,"
                    "  \"presetTemperature\":25,"
                    "  \"fanspeed\":5,"
                    "  \"currentTemperature\":28,"
                    "  \"currentHumidity\":65"
                    "}";
                picojson::value state;
                const string err = picojson::parse(state, json);
                if (err.empty()) {
                    kiiApi->registerState(requestID, state);
                    cout << requestID << " requested." << endl;
                } else {
                    cout << err << endl;
                }
            } else if (command == "state") {
                kiiApi->getState(requestID);
                cout << requestID << " requested." << endl;
            } else if (command == "commands") {
                kiiApi->getCommandList(requestID);
                cout << requestID << " requested." << endl;
            } else if (command == "execute") {
                string json =
                    "{"
                    "  \"actions\": ["
                    "    {\"turnPower\":{\"power\":true}},"
                    "    {\"setBrightness\":{\"brightness\":3000}}"
                    "  ],"
                    "  \"issuer\": \"user:XXXXXXXXXXXX\","
                    "  \"schema\": \"SmartLight\","
                    "  \"schemaVersion\":1,"
                    "  \"title\": \"Turn and set brightness\","
                    "  \"description\": \"Turn light on and set to full brightness\","
                    "  \"metadata\": {"
                    "    \"brightness\":\"full brightness\""
                    "  }"
                    "}";
                picojson::value c;
                const string err = picojson::parse(c, json);
                if (err.empty()) {
                    kiiApi->executeCommand(requestID, c);
                    cout << requestID << " requested." << endl;
                } else {
                    cout << err << endl;
                }
            }
        }
    } else {
      cout << "Please check error." << endl;
    }

    delete kiiApi;

    return 0;
}
