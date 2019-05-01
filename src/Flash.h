#ifndef Flash_h
#define Flash_h

#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1046

namespace flash
{
    void setup();
    void saveIP(String ip);
    void savePort(String port);
    void saveRules(String port);
    String getIP();
    String getPort();
    String getRules();
}; // namespace flash

#endif