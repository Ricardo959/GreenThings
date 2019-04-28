#include "Flash.h"

void flash::setup() {
    EEPROM.begin(EEPROM_SIZE);
}

void flash::saveIP(String data) {
    EEPROM.writeString(0, data);
    EEPROM.commit();
}

String flash::getIP() {
    return EEPROM.readString(0);
}

void flash::savePort(String data) {
    EEPROM.writeString(16, data);
    EEPROM.commit();
}

String flash::getPort() {
    return EEPROM.readString(16);
}