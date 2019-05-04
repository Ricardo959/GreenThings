#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h> // Increase the MQTT_MAX_PACKET_SIZE to 1024 in PubSubClient.h
#include <ArduinoJson.h>
#include "WiFi.h"
#include "Flash.h"
#include "Utilities.h"
#include "Controller.h"

#define UUID PROGMEM "ESTUFA INTELIGENTE"
#define USER PROGMEM "greenthing"
#define PASS PROGMEM "xyzlmnop"
#define HTMLHEADER PROGMEM "<style>h1,h3{color:#ffffff}body{background-color:#339933}button{background-color:#6bdb87}</style>"

RTC_DATA_ATTR uint8_t SLEEP_TIME = 60; // 60 segundos.
bool newServer = true; // If false, updates once; if true, updates always
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
		String message;
		for (unsigned int i = 0; i < length; i++)
		{
			message += (char)payload[i];
		}

		Serial.printf("Response recieved: %s\n", message.c_str());
		hasResponse = true;

		if (message != "null" && message != "error")
		{
			newServer = false;
			StaticJsonDocument<1024> doc;
			deserializeJson(doc, message);

			JsonArray actuators = doc["actuators"].as<JsonArray>();
			if (actuators.size() > 0)
			{
				String rules;
				serializeJson(actuators, rules);
				Serial.printf("Saving rules: %s\n", rules.c_str());
				flash::saveRules(rules);
				return;
			}
			Serial.println("No rules found");
			return;
		}
		Serial.println("Response is empty");
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
		delay(500);
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
		Serial.println("Requesting server data ...");
		DynamicJsonDocument doc(1024);
		doc["mac"] = MAC;
		String json;
		serializeJson(doc, json);
		bool subscribed = pubSubClient.subscribe("/device/response");
		bool published = pubSubClient.publish("/device/get", json.c_str(), true); // Retained = true
		if (subscribed && published)
		{
			Serial.println("Mac address sent, waiting 5 seconds for response ...");
			for (uint8_t i = 0; i < 50; i++) // Aguarda 5 segundos pela resposta:
			{
				delay(100);
				pubSubClient.loop();
				if (!newServer) break;
			}
			if (hasResponse)
			{
				if (newServer)
				{
					String registerData = "{\"mac\": \"1012109628\", \"sensors\": [ { \"type\": \"h\" }, { \"type\": \"t\" }, { \"type\": \"p\" } ], \"actuators\": [ { \"type\": \"v\" } ] }";
					Serial.printf("Sending registration request: %s\n", registerData.c_str());
					if (pubSubClient.publish("/device/new", registerData.c_str()), true)
					{
						Serial.println("Registration sent.");
					}
					else
					{
						Serial.println("Request failed.");
						reset();
					}
				}
			}
			else
			{
				Serial.println("No response received.");
				reset();
			}
		}
		else
		{
			Serial.println("Connection to server failed.");
			reset();
		}
	}

	// Leitura dos sensores:
	Serial.println("Reading sensor values ...");
	Controller *controller = new Controller(MAC);
	controller->has_dht11 = true;
	controller->has_precipitation_module = true;
	controller->has_solenoid_valve = true;

	// Enviando dados dos sensores:
	String sensorData = controller->getSensorData();
	Serial.printf("Sending sensor data: %s\n", sensorData.c_str());
	if (pubSubClient.publish("/sensor/value", sensorData.c_str()), true)
	{
		Serial.println("Data sent.");
	}
	else
	{
		Serial.printf("Publish failed.");
	}

	// Atuando nos atuadores:
	Serial.println("Starting actuations ...");
	controller->actuate(flash::getRules());

	// Desconectando do Broker:
	pubSubClient.disconnect();
	deepSleep();
}

// Não usado.
void loop() {}
