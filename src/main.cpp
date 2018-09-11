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
    cout << "register succeeded:" << v << endl;
}

void registerFailCB(picojson::value &v) {
    cout << "register failed:" << v << endl;
}

void stateSuccessCB(picojson::value &v) {
    cout << "state succeeded:" << v << endl;
}

void stateFailCB(picojson::value &v) {
    cout << "state failed:" << v << endl;
}

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD);

    if (kiiApi->waitForStandby()) {
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
