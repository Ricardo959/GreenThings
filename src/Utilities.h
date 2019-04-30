#ifndef Utilities_h
#define Utilities_h

#include <Arduino.h>
#include <string.h>

namespace utils {
    String macToString(uint8_t macaddress[6]);
    String publishJSON(String mac, String data);

    uint8_t* stringToIP(String ip);

    String getValue(String data, char separator, int index);
};

#endif
