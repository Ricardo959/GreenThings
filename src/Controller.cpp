#include "Controller.h"

Controller::Controller()
{
    this->has_dht11 = false;
    this->has_precipitation_module = false;
    this->has_LDR = false;
    this->has_solenoid_valve = false;
    this->has_cover_actuator = false;
    this->has_shade_actuator = false;
    this->dht = new DHT(DHT_PIN, DHT_TYPE);
}

Controller::Controller(
    bool has_dht11,
    bool has_precipitation_module,
    bool has_LDR,
    bool has_solenoid_valve,
    bool has_cover_actuator,
    bool has_shade_actuator)
{

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

    String json = "[";

    if (has_dht11)
    {
        dht11_temperature = (int)dht->readTemperature();
        dht11_humidity = (int)dht->readHumidity();
        json.concat("{\"type\":\"t\",\"value\":");
        json.concat(dht11_temperature);
        json.concat("},{\"type\":\"h\",\"value\":");
        json.concat(dht11_humidity);
        json.concat("},");
    }

    if (has_precipitation_module)
    {
        precipitation_value = map(analogRead(PRECIP_PIN), 1024, 0, 0, 100);
        if (precipitation_value < 0)
            precipitation_value = 0;
        json.concat("{\"type\":\"p\",\"value\":");
        json.concat(precipitation_value);
        json.concat("},");
    }

    json.remove(json.length() - 1);
    json.concat("]");
    return json;
}