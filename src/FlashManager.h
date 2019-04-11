#ifndef FlashManager_h
#define FlashManager_h

#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 20
#define NULL __null

void setupFlash();

void saveIP(String ip);
void savePort(String port);

String getIP();
String getPort();

#endif