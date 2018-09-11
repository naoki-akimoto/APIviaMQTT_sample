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

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD);

    if (kiiApi->waitForStandby()) {
        while(true) {
            string command;
            cout << ">>";
            cin >> command;
            if (command == "exit") {
                break;
            }
        }
    } else {
      cout << "Please check error." << endl;
    }

    delete kiiApi;

    return 0;
}
