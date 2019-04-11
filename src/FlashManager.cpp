#include "FlashManager.h"

void setupFlash() {
    EEPROM.begin(EEPROM_SIZE);
}

void saveIP(String ip) {
    unsigned int i = 0;
    for( ; i < 15 && i < ip.length(); i++) {
        EEPROM.write(i, ip.charAt(i));
    }
    EEPROM.write(i, 'n');
    EEPROM.commit();
}

void savePort(String port) {
    unsigned int i = 0;
    for( ; i < 4 && i < port.length(); i++) {
        EEPROM.write(i, port.charAt(i + 16));
    }
    EEPROM.write(i + 16, 'n');
    EEPROM.commit();
}

String getIP() {
    String ip = "";
    char c = NULL;
    for(unsigned int i = 0; i < 15; i++) {
        c = EEPROM.read(i);
        if (c == 'n') {
            return ip;
        } else {
            ip += c;
        }
    }
}

String getPort() {
    String port = "";
    char c = NULL;
    for(unsigned int i = 0; i < 4; i++) {
        c = EEPROM.read(i + 16);
        if (c == 'n') {
            return port;
        } else {
            port += c;
        }
    }
}