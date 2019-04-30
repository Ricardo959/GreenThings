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

uint8_t* utils::stringToIP(String ip) {
    uint8_t ipArray[4];
    for (uint8_t i = 0; i <  4; i++) {
        ipArray[i] = utils::getValue(ip, '.', i).toInt();
    }
    return ipArray;
}

String utils::getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}