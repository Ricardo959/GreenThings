#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "WiFi.h"
#include "Flash.h"
#include "Utilities.h"
#include "Controller.h"

#define UUID PROGMEM "ESTUFA INTELIGENTE"
#define USER PROGMEM "greenthing"
#define PASS PROGMEM "xyzlmnop"
#define HTMLHEADER PROGMEM "<style>h1,h3{color:#ffffff}body{background-color:#339933}button{background-color:#6bdb87}</style>"

RTC_DATA_ATTR uint8_t SLEEP_TIME = 60; // 60 segundos.
bool newServer = 0;
uint8_t macaddress[6];

WiFiManager* wifiManager;

WiFiManagerParameter mqtt_ip("mqqt_ip", "Broker IP", "", 16);
WiFiManagerParameter mqtt_port("mqtt_port", "Broker Port", "", 5);

void deepSleep() {
  // Inicialização do sono profundo:
  Serial.println("Starting deep sleep ...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000); // recebe o tempo em microsegundos.
  delay(100);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void saveConfigCallback() {
  // Executado apenas quando novos dados foram inseridos:
  newServer = true;
  flash::saveIP(mqtt_ip.getValue());
  flash::savePort(mqtt_port.getValue());
}

void setup() {
  // Inicializações básicas:
  Serial.begin(115200);
  flash::setup();
  
  // Configuração do WifiManager:
  wifiManager = new WiFiManager();
  wifiManager->setCustomHeadElement(HTMLHEADER);
  wifiManager->addParameter(&mqtt_ip);
  wifiManager->addParameter(&mqtt_port);
  wifiManager->setSaveConfigCallback(saveConfigCallback);
  wifiManager->autoConnect(UUID, PASS);

  esp_wifi_get_mac(ESP_IF_WIFI_STA, macaddress);
  const String MAC = utils::macToString(macaddress);

  Serial.printf("Connected to WiFi! MAC address: %s\n", MAC.c_str());
  Serial.printf("Broker configured on: %s, ", flash::getIP().c_str());
  Serial.printf("Port: %s\n", flash::getPort().c_str());

  // Conectando ao Broker:
  Serial.println("Connecting to Broker ...");
  WiFiClient wifiClient;
  PubSubClient pubSubClient(wifiClient);
  pubSubClient.setServer(flash::getIP().c_str(), flash::getPort().toInt());
  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    pubSubClient.connect(MAC.c_str(), USER, PASS);
    if (pubSubClient.connected()) break;
    Serial.printf("Connection failed. Attempt: %d/3\n", attempt);
    delay(1000);
  }

  if (!pubSubClient.connected()) {
    // Caso sem conexão ao Broker:
    Serial.println("Connection failed! Cannot proceed.");
    deepSleep();
  } else {
    Serial.println("Connected to Broker!");
  }

  // Configuração da primeira conexão:
  if (newServer) {
    Serial.println("New connection detected! Requesting server data ...");

    newServer = false;
  }

  // Leitura dos sensores:
  Serial.println("Reading sensor values ...");
  Controller* controller = new Controller();
  controller->has_dht11 = true;
  controller->has_precipitation_module = true;

  // Enviando dados dos sensores:
  String sensorData = controller->getSensorData();
  Serial.printf("Sending sensor data: %s\n", sensorData.c_str());
  if (pubSubClient.publish("/sensor/value", utils::publishJSON(MAC, sensorData).c_str()), true) {
    Serial.println("Data sent.");
  } else {
    Serial.printf("Publish failed.");
  }
  delay(1000);

  // Desconectando do Broker:
  pubSubClient.disconnect();

  deepSleep();
}

// Não usado.
void loop() {}
