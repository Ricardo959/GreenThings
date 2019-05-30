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

    dht = new DHT(DHT_PIN, DHT_TYPE);
    servo = new Servo();
    servo->attach(VALVE_PIN);

    valve_value = 0;
    cover_state = false;
    shade_state = false;
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

    dht = new DHT(DHT_PIN, DHT_TYPE);
    servo = new Servo();
    servo->attach(VALVE_PIN);

    valve_value = 0;
    cover_state = false;
    shade_state = false;
}

String Controller::getSensorData()
{
    dht->begin();

    DynamicJsonDocument doc(1024);
    doc["mac"] = id;

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

void Controller::actuate(String rules)
{
    Serial.printf("Deserializing rules: %s\n", rules.c_str());
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, rules);
    if (err)
    {
        Serial.printf("Deserialization failed with code: %s\n", err.c_str());
        return;
    }

    JsonArray actuators = doc.as<JsonArray>();
    Serial.printf("%d actuator rules detected.\n", actuators.size());
    for (uint8_t i = 0; i < actuators.size(); i++)
    {
        for (uint8_t j = 0; j < actuators[i]["activation"].size(); j++)
        {
            JsonObject activation = actuators[i]["activation"][j];
            int sensorValue = 0;
            if (activation["sensor"] == "t") sensorValue = dht11_temperature; // Ativação conforme temperatura
            else if (activation["sensor"] == "h") sensorValue = dht11_humidity; // Ativação conforme humidade
            else if (activation["sensor"] == "p") sensorValue = precipitation_value; // Ativação conforme precipitação

            if(actuators[i]["type"] == "v") // Possui valvula solenoide:
            {
                if (match(sensorValue, activation["max"].as<int>(), activation["min"].as<int>()))
                {
                    if (valve_value != 180)
                    {
                        valve_value = 180;
                        servo->write(valve_value);
                        delay(500);
                    }
                }
                else
                {
                    if (valve_value != 0)
                    {
                        valve_value = 0;
                        servo->write(valve_value);
                        delay(500);
                    }
                }
                Serial.printf("Solenoid valve -> ", valve_value);
            }
            // else if(actuators[i]["type"] == "c")
            // {
            //     ...
            // }
            // ...
        }
    }
}

bool Controller::match(int value, int max, int min)
{
    if (max > min)
    {
        return (min <= value && value <= max);
    }
    else if (max < min)
    {
        return (value <= max && min <= value);
    }
    else
    {
        return (max == value);
    }
}