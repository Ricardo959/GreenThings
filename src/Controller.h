#ifndef Controller_h
#define Controller_h

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHT_TYPE DHT11
#define DHT_PIN 4
#define PRECIP_PIN 13

class Controller
{
  public:
	// Sensores:
	bool has_dht11;
	bool has_precipitation_module;
	bool has_LDR;
	// Atuadores:
	bool has_solenoid_valve;
	bool has_cover_actuator;
	bool has_shade_actuator;

	Controller();

	Controller(
		bool has_dht11,
		bool has_precipitation_module,
		bool has_LDR,
		bool has_solenoid_valve,
		bool has_cover_actuator,
		bool has_shade_actuator);

	String getSensorData();

  private:
	DHT *dht;

	int dht11_temperature;
	int dht11_humidity;
	int precipitation_value;
	int LDR_value;
};

#endif