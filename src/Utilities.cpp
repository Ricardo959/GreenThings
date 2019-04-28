#include "Utilities.h"

String utils::macToString(uint8_t macaddress[6]) {
    uint32_t mac = 0;
    uint32_t multiplier = 1;
    for (uint8_t i = 0; i < 6; i++) {
        mac += multiplier * macaddress[i];
        multiplier *= 100;
    }
    return String(mac);
}

String utils::publishJSON(String mac, String sensors) {
    String json = "{";
    json.concat("\"mac\":");
    json.concat(mac);
    json.concat(",\"sensors\":");
    json.concat(sensors);
    json.concat("}");
    return json;
}