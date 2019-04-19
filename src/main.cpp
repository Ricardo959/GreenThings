#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "FlashManager.h"
#include "WiFi.h"

#define UUID PROGMEM "ESTUFA INTELIGENTE"
#define USER PROGMEM "admin"
#define PASS PROGMEM "xyzlmnop"
#define CLIENT PROGMEM "espclient"
#define HTMLHEADER PROGMEM "<style>h1,h3{color:#ffffff}body{background-color:#339933}button{background-color:#6bdb87}</style>"

RTC_DATA_ATTR bool newServer = 0;
RTC_DATA_ATTR uint8_t SLEEP_TIME = 30; // 30 segundos.

uint8_t macaddress[6];

WiFiManager* wifiManager;

WiFiManagerParameter mqtt_ip("mqqt_ip", "IP do servidor", "", 16);
WiFiManagerParameter mqtt_port("mqtt_port", "Porta do Broker", "", 5);

void deepSleep() {
  // Inicialização do sono profundo:
  Serial.println("Starting deep sleep ...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000); // recebe o tempo em microsegundos.
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void saveConfigCallback() {
  // Executado apenas quando novos dados foram inseridos:
  newServer = true;
  saveIP(mqtt_ip.getValue());
  savePort(mqtt_port.getValue());
}

void setup() {
  // Inicializações básicas:
  Serial.begin(115200);
  setupFlash();
  esp_wifi_get_mac(ESP_IF_WIFI_STA, macaddress);
  
  // Configuração do WifiManager:
  wifiManager = new WiFiManager();
  wifiManager->setCustomHeadElement(HTMLHEADER);
  wifiManager->addParameter(&mqtt_ip);
  wifiManager->addParameter(&mqtt_port);
  wifiManager->setSaveConfigCallback(saveConfigCallback);
  wifiManager->autoConnect(UUID, PASS);

  Serial.printf("Broker configured on: %s, ", getIP().c_str());
  Serial.printf("Port: %s\n", getPort().c_str());

  // Leitura dos sensores:
  Serial.println("Reading sensor values ...");

  // Conectando ao Broker:
  Serial.println("Connecting to Broker ...");
  WiFiClient wifiClient;
  PubSubClient pubSubClient(wifiClient);
  //client.setServer(getIP().c_str(), getPort().toInt());
  pubSubClient.setServer({192, 168, 0, 104}, 1883);
  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    pubSubClient.connect(CLIENT, USER, PASS);
    if (pubSubClient.connected()) break;
    Serial.printf("Connection failed. Attempt: %d/3\n", attempt);
    delay(1000);
  }

  if (!pubSubClient.connected()) {
    // Caso sem conexão ao Broker:
    Serial.println("Connection failed! Cannot proceed.");
    deepSleep();
  } else {
    Serial.println("Connected!");
  }

  // Configuração da primeira conexão:
  if (newServer) {
    Serial.println("New connection detected! Requesting server data ...");
    newServer = false;
  }

  // Enviando dados dos sensores:
  Serial.println("Sending sensor data ...");
  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    if (pubSubClient.publish("/history/add", "{\"mac\":\"89:71\",\"type\":2,\"value\":20}"), true) {
      Serial.println("Data sent!");
      break;
    }
    Serial.printf("Publish failed. Attempt: %d/3\n", attempt);
    delay(1000);
  }

  // Desconectando do Broker:
  pubSubClient.disconnect();

  deepSleep();
}

// Não usado.
void loop() {}
