#include "FlashManager.h"

void setupFlash() {
    EEPROM.begin(EEPROM_SIZE);
}

void saveIP(String data) {
    EEPROM.writeString(0, data);
    EEPROM.commit();
}

String getIP() {
    return EEPROM.readString(0);
}

void savePort(String data) {
    EEPROM.writeString(16, data);
    EEPROM.commit();
}

String getPort() {
    return EEPROM.readString(16);
}