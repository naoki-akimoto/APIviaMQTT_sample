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
            } else if (command == "register") {
                kiiApi->registerState(registerSuccessCB, registerFailCB);
            } else if (command == "state") {
                kiiApi->getState(stateSuccessCB, stateFailCB);
            }
        }
    } else {
      cout << "Please check error." << endl;
    }

    delete kiiApi;

    return 0;
}
