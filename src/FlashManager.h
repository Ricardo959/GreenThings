#ifndef FlashManager_h
#define FlashManager_h

#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define EEPROM_SIZE 21

void setupFlash();

void saveIP(String ip);
void savePort(String port);

String getIP();
String getPort();

#endif