#include "MQTT_KiiAPI.hpp"

#include <unistd.h>
#include <iostream>

int main() {
    MQTT_KiiAPI *kiiApi;

    kiiApi = new MQTT_KiiAPI(
        "mqtt-us.kii.com",
        1883,
        "ldmaynmm98zp",
        "75473e7bea444d42afedae4c4ecec64f",
        "testVendor",
        "testVendorPass");


   std::cin.good();

   delete kiiApi;
}
