#include <Arduino.h>
#include <WiFiManager.h>
#include "FlashManager.h"

#define UUID PROGMEM "Estufa Inteligente"
#define PASS PROGMEM "xyzlmnop"
#define STYL PROGMEM "<style>body{background-color:#a8e063}button{background-color:#56ab2f}</style>"

WiFiManager* wifiManager;
String mqttIP;
String mqttPort;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupFlash();
  
  wifiManager = new WiFiManager();
  wifiManager->setCustomHeadElement(STYL);

  WiFiManagerParameter mqtt_text("<h1>MQTT</h1>");
  WiFiManagerParameter mqtt_ip("ip", "mqtt server IP", "", 15);
  WiFiManagerParameter mqtt_port("ip", "mqtt port", "1883", 15);

  wifiManager->addParameter(&mqtt_text);
  wifiManager->addParameter(&mqtt_ip);
  wifiManager->addParameter(&mqtt_port);

  wifiManager->autoConnect(UUID, PASS);
  mqttIP = mqtt_ip.getValue();
  mqttPort = mqtt_port.getValue();

  delete wifiManager;
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Connected!");
  Serial.printf("Broker configured on: %s port: %s\n", mqttIP.c_str(), mqttPort.c_str());
  delay(5000);
}
