#ifndef SPRINKLER_DEVICE_H
#define SPRINKLER_DEVICE_H

#include <FS.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "sprinkler.h"

class SprinklerDevice
{
private:
  char device_name[50];
  bool shouldSaveConfig;

protected:
  unsigned int _RELAY;
  unsigned int _LED;
  unsigned int _BTN;

public:
  SprinklerDevice(int relay, int led, int btn) : 
  _RELAY(relay), 
  _LED(led), 
  _BTN(btn), 
   shouldSaveConfig(false), 
   device_name{'S', 'p', 'r', 'i', 'n', 'k', 'l', 'e', 'r'}
  {
  }

  void requestSave()
  {
    shouldSaveConfig = true;
  }

  const char *getDeviceName() const
  {
    return device_name;
  }

  char *getDeviceName(char *str, size_t len)
  {
    if (device_name)
    {
      strncpy(str, device_name, len);
    }
    return str;
  }

  bool setDeviceName(const char *value)
  {
    if (value && sizeof(value) > 1 && strcmp(device_name, value) != 0)
    {
      strncpy(device_name, value, 49);

      return true;
    }

    return false;
  }

  void setup()
  {
    pinMode(_LED, OUTPUT);

    pinMode(_BTN, INPUT_PULLUP);

    pinMode(_RELAY, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(_BTN), []() {
      if (Sprinkler.isWatering())
      {
        Sprinkler.stop();
      }
      else
      {
        Sprinkler.start();
      }
    },
                    CHANGE);

    load();

    if (shouldSaveConfig)
    {
      save();
    }
  }

  void load()
  {
    //read configuration from FS json
    Serial.println("[SETTINGS] mounting FS...");

    if (SPIFFS.begin())
    {
      Serial.println("[SETTINGS] mounted file system");
      if (SPIFFS.exists("/config.json"))
      {
        //file exists, reading and loading
        Serial.println("[SETTINGS] reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile)
        {
          Serial.println("[SETTINGS] opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file..0

          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonBuffer jsonBuffer;
          JsonObject &json = jsonBuffer.parseObject(buf.get());
          json.printTo(Serial);
          if (json.success())
          {
            Serial.println("[SETTINGS] parsed json");
            strcpy(device_name, json["relay_name"]);
            Serial.print("[SETTINGS] device_name ");
            Serial.println(device_name);
          }
          else
          {
            Serial.println("[SETTINGS] failed to load json config");
          }
        }
      }
    }
    else
    {
      Serial.println("[SETTINGS] failed to mount FS");
    }
  }

  void turnOn()
  {
    digitalWrite(_LED, LOW);
    digitalWrite(_RELAY, HIGH);
  }

  void turnOff()
  {
    digitalWrite(_LED, HIGH);
    digitalWrite(_RELAY, LOW);
  }

  virtual void reset()
  {
    Serial.println("[MAIN] Factory reset requested.");

    WiFi.disconnect(true);
    SPIFFS.format();
    system_restart();

    delay(5000);
  }

  virtual void restart()
  {
    Serial.println("[MAIN] Restarting...");
    system_restart();
  }

  void save()
  {
    Serial.println("[MAIN] saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["relay_name"] = device_name;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("[MAIN] failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
};

#endif
