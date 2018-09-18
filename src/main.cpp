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

void commandCB(MQTT_KiiAPI &api, picojson::value &v) {
    cout << "Command alived: " << endl;
    if (!v.is<picojson::null>()) {
        cout << v.serialize(true) << endl;

        picojson::array results;
        picojson::object &obj = v.get<picojson::object>();
        string commandID = obj["commandID"].get<string>();
        picojson::array &actions = obj["actions"].get<picojson::array>();
        for (picojson::array::iterator it = actions.begin(); it != actions.end(); it++) {
            picojson::object &action = it->get<picojson::object>();
            string actionName = action.begin()->first;

            picojson::object succeeded;
            succeeded.insert(make_pair("succeeded", picojson::value(true)));

            picojson::object result;
            result.insert(std::make_pair(actionName, picojson::value(succeeded)));
            results.push_back(picojson::value(result));
        }
        picojson::object sendObj;
        sendObj.insert(std::make_pair("actionResults", picojson::value(results)));

        string requestID;
        picojson::value send = picojson::value(sendObj);
        api.sendActionResults(requestID, commandID, send);
        cout << requestID << " requested sendActionResults." << endl;
    }
}

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
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD, commandCB, successCB, failCB);

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
                cout << "\tcommands\tget command list." << endl;
                // cout << "\texecute\t\texecute command." << endl;
                cout << "\tresult\t\tsend results of command." << endl;
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
                /*
                string json =
                    "{"
                    "  \"actions\": ["
                    "    {\"testAction\":{}}"
                    "  ],"
                    "  \"issuer\": \"user:12b097a00022-ac2b-8e11-4d7b-03d36642\","
                    "  \"schema\": \"\","
                    "  \"schemaVersion\":0"
                    "}";
                picojson::value c;
                const string err = picojson::parse(c, json);
                if (err.empty()) {
                    kiiApi->executeCommand(requestID, c);
                    cout << requestID << " requested." << endl;
                } else {
                    cout << err << endl;
                }
                */
            } else if (command == "result") {
                string commandID = "2420dfb0-b7e8-11e8-90f5-22000a66c675";
                string json =
                    "{"
                    "  \"actionResults\": ["
                    "    {\"testAction\":{\"succeeded\":true}}"
                    "  ]"
                    "}";
                picojson::value v;
                const string err = picojson::parse(v, json);
                if (err.empty()) {
                    kiiApi->sendActionResults(requestID, commandID, v);
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
