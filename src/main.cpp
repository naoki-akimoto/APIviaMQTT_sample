#include "MQTT_KiiAPI.hpp"

#include <unistd.h>
#include <iostream>

#define SITE ""
#define PORT 1883
#define APPID ""
#define APPKEY ""
#define VENDORTHINGID ""
#define THINGPASSWORD ""

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        SITE, PORT, APPID, APPKEY, VENDORTHINGID, THINGPASSWORD);

    while(getchar() != '\n') {
    }

    delete kiiApi;

    return 0;
}
