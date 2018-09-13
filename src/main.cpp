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

void registerSuccessCB(picojson::value &v) {
    cout << "register succeeded:" << endl << v.serialize(true) << endl;
}

void registerFailCB(picojson::value &v) {
    cout << "register failed:" << endl << v.serialize(true) << endl;
}

void stateSuccessCB(picojson::value &v) {
    cout << "state succeeded:" << endl << v.serialize(true) << endl;
}

void stateFailCB(picojson::value &v) {
    cout << "state failed:" << endl << v.serialize(true) << endl;
}

void commandsSuccessCB(picojson::value &v) {
    cout << "commands succeeded:" << endl << v.serialize(true) << endl;
}

void commandsFailCB(picojson::value &v) {
    cout << "commands failed:" << endl << v.serialize(true) << endl;
}

void executeSuccessCB(picojson::value &v) {
    cout << "execute succeeded:" << endl << v.serialize(true) << endl;
}

void executeFailCB(picojson::value &v) {
    cout << "execute failed:" << endl << v.serialize(true) << endl;
}

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD);

    cout << "Please wait for ready." <<endl;
    if (kiiApi->waitForReady()) {
        while(true) {
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
                    kiiApi->registerState(state, registerSuccessCB, registerFailCB);
                } else {
                    cout << err << endl;
                }
            } else if (command == "state") {
                kiiApi->getState(stateSuccessCB, stateFailCB);
            } else if (command == "commands") {
                kiiApi->getCommandList(commandsSuccessCB, commandsFailCB);
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
                    kiiApi->executeCommand(c, executeSuccessCB, executeFailCB);
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
