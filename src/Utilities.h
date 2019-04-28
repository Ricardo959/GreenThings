#ifndef Utilities_h
#define Utilities_h

#include <Arduino.h>

namespace utils {
    String macToString(uint8_t macaddress[6]);
    String publishJSON(String mac, String data);
};

#endif
