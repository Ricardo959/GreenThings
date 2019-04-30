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
bool newServer = false;
bool hasResponse = false;
uint8_t macaddress[6];

WiFiManager *wifiManager;

WiFiManagerParameter mqtt_ip("mqqt_ip", "Broker IP", "", 16);
WiFiManagerParameter mqtt_port("mqtt_port", "Broker Port", "", 5);

void saveConfigCallback()
{
	// Quando novos dados foram inseridos:
	newServer = true;
	flash::saveIP(mqtt_ip.getValue());
	flash::savePort(mqtt_port.getValue());
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
	// Quando uma mensagem chega no broker:
	if (String(topic) == "/device/response")
	{
		char message[length];
		for (unsigned int i = 0; i < length; i++)
		{
			message[i] = (char)payload[i];
		}

		Serial.printf("Response recieved: %s\n", message);
		hasResponse = true;

		if (strcmp(message, "null") + strcmp(message, "error") > 0)
		{
			newServer = false;
			
			// TODO: Salvar Regras
		}
	}
}

void deepSleep()
{
	// Inicialização do sono profundo:
	Serial.println("Starting deep sleep ...");
	esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000); // recebe o tempo em microsegundos.
	delay(100);
	Serial.flush();
	esp_deep_sleep_start();
}

void reset()
{
	Serial.printf("Restarting ...");
	wifiManager->resetSettings();
	delay(100);
	ESP.restart();
}

void setup()
{
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
	pubSubClient.setCallback(mqttCallback);
	for (uint8_t attempt = 1; attempt <= 3; attempt++)
	{
		pubSubClient.connect(MAC.c_str(), USER, PASS);
		if (pubSubClient.connected())
			break;
		Serial.printf("Connection failed. Attempt: %d/3\n", attempt);
		delay(1000);
	}
	if (!pubSubClient.connected()) // Caso sem conexão ao Broker:
	{
		Serial.println("Connection failed! Cannot proceed.");
		deepSleep();
	}
	else
	{
		Serial.println("Connected to Broker!");
	}

	if (newServer) // Configuração da primeira conexão ao servidor:
	{
		Serial.println("New connection detected! Requesting server data ...");
		String json = "{\"mac\":";
		json.concat(MAC);
		json.concat("}");
		bool subscribed = pubSubClient.subscribe("/device/response", 2);		  // QoS = 2
		bool published = pubSubClient.publish("/device/get", json.c_str(), true); // Retained = true
		if (subscribed && published)
		{
			Serial.println("Mac address sent, waiting for response ...");
			for (uint8_t i = 0; i < 50; i++) // Espera 5 segundos pela resposta:
			{
				pubSubClient.loop();
				if (!newServer)
					break;
				delay(100);
			}
			if (hasResponse)
			{
				if (newServer)
				{
					String registerData = "\"sensors\": [ { \"type\": \"h\" }, { \"type\": \"t\" }, { \"type\": \"p\" } ], \"actuators\": [ { \"type\": \"v\" } ]";
					Serial.println("Sending registration request ...");
					if (pubSubClient.publish("/device/new", utils::publishJSON(MAC, registerData).c_str()), true)
					{
						Serial.println("Registration sent.");
					}
					else
					{
						Serial.printf("Registration failed.");
						reset();
					}
				}
			}
			else
			{
				Serial.printf("No response received.");
				reset();
			}
		}
		else
		{
			Serial.printf("Connection to server failed.");
			reset();
		}
	}

	// Leitura dos sensores:
	Serial.println("Reading sensor values ...");
	Controller *controller = new Controller();
	controller->has_dht11 = true;
	controller->has_precipitation_module = true;

	// Enviando dados dos sensores:
	String sensorData = controller->getSensorData();
	Serial.printf("Sending sensor data: %s\n", sensorData.c_str());
	if (pubSubClient.publish("/sensor/value", utils::publishJSON(MAC, sensorData).c_str()), true)
	{
		Serial.println("Data sent.");
	}
	else
	{
		Serial.printf("Publish failed.");
	}
	delay(1000);

	// Desconectando do Broker:
	pubSubClient.disconnect();
	deepSleep();
}

// Não usado.
void loop() {}
