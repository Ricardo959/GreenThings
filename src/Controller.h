#ifndef Controller_h
#define Controller_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Servo.h>

#define DHT_TYPE DHT11
#define DHT_PIN 4
#define PRECIP_PIN 13
#define VALVE_PIN 0

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

	String id;

	Controller(String id);

	Controller(
		String id,
		bool has_dht11,
		bool has_precipitation_module,
		bool has_LDR,
		bool has_solenoid_valve,
		bool has_cover_actuator,
		bool has_shade_actuator);

	String getSensorData();

	void actuate(String rules);

  private:

	DHT* dht;
	Servo* servo;

	int dht11_temperature;
	int dht11_humidity;
	int precipitation_value;
	int valve_value;
	bool cover_state;
	bool shade_state;

	bool match(int value, int max, int min);
};

#endif