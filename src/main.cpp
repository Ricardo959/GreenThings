#include <Arduino.h>
#include <WiFiManager.h>

#define UUID "GreenThing"
#define PASS "xyzlmnop"

WiFiManager* wifiManager;

void setup() {
  // put your setup code here, to run once:
  wifiManager = new WiFiManager();
  wifiManager->autoConnect(UUID, PASS);
}

void loop() {
  // put your main code here, to run repeatedly:
}