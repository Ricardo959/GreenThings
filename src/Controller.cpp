#include "Controller.h"

Controller::Controller(String id)
{
    this->id = id;
    this->has_dht11 = false;
    this->has_precipitation_module = false;
    this->has_LDR = false;
    this->has_solenoid_valve = false;
    this->has_cover_actuator = false;
    this->has_shade_actuator = false;
    this->dht = new DHT(DHT_PIN, DHT_TYPE);
}

Controller::Controller(
    String id,
    bool has_dht11,
    bool has_precipitation_module,
    bool has_LDR,
    bool has_solenoid_valve,
    bool has_cover_actuator,
    bool has_shade_actuator)
{
    this->id = id;
    this->has_dht11 = has_dht11;
    this->has_precipitation_module = has_precipitation_module;
    this->has_LDR = has_LDR;
    this->has_solenoid_valve = has_solenoid_valve;
    this->has_cover_actuator = has_cover_actuator;
    this->has_shade_actuator = has_shade_actuator;
    this->dht = new DHT(DHT_PIN, DHT_TYPE);
}

String Controller::getSensorData()
{
    dht->begin();

    DynamicJsonDocument doc(1024);
    doc["mac"] = this->id;

    JsonArray sensors = doc.createNestedArray("sensors");

    if (has_dht11)
    {
        dht11_temperature = (int)dht->readTemperature();
        dht11_humidity = (int)dht->readHumidity();

        JsonObject hSensor = sensors.createNestedObject();
        hSensor["type"] = "t";
        hSensor["value"] = dht11_temperature;

        JsonObject tSensor = sensors.createNestedObject();
        tSensor["type"] = "h";
        tSensor["value"] = dht11_humidity;
    }

    if (has_precipitation_module)
    {
        precipitation_value = map(analogRead(PRECIP_PIN), 1024, 0, 0, 100);
        if (precipitation_value < 0) precipitation_value = 0;

        JsonObject sensor = sensors.createNestedObject();
        sensor["type"] = "p";
        sensor["value"] = precipitation_value;
    }

    String json;
    serializeJson(doc, json);
    return json;
}