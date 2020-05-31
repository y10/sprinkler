#ifndef SPRINKLER_DEVICE_H
#define SPRINKLER_DEVICE_H

#include <EEPROM.h>
#include "sprinkler.h"

#define EEPROM_SIZE 1024

struct SprinklerConfig {
  uint8_t version;
  char disp_name[50];
  char upds_addr[50];
};

class SprinklerDevice
{
private:
  std::function<void()> onSetup;

  String host_name;
  String disp_name;
  String safe_name;
  String upds_addr;

public:
  SprinklerDevice(std::function<void(void)> onSetupCallback) : onSetup(onSetupCallback)
  {
    disp_name = "Sprinkler";
    safe_name = "sprinkler";
    host_name = "sprinkler-" + String(ESP.getChipId(), HEX);
    upds_addr = "http://ota.voights.net/sprinkler.bin";
  }

   const String hostname() const {
    return host_name;
  }

  const String dispname() const {
    return disp_name;
  }

  const String safename() const{
    return safe_name;
  }

  bool dispname(const char* name){
    bool changed = false;
    if (strlen(name) > 0)
    {
      if (!disp_name.equals(name))
      {
        disp_name = name;
        changed = true;
      }
      if (!safe_name.equals(disp_name))
      {
        safe_name = name;
        safe_name.replace(" ", "_");
        safe_name.toLowerCase();
        changed = true;
      }
    }

    return changed;
  }

  const String updsaddr() const {
    return upds_addr;
  }

  const String updsaddr(const char* addr){
    upds_addr = addr;
    
    if (upds_addr.indexOf("://") == -1)
    {
      upds_addr = "http://" + upds_addr;
    }

    return upds_addr;
  }

  void setup()
  {
    onSetup();
    load();
  }

  void load()
  {
    Serial.println("[EEPROM] reading...");
    EEPROM.begin(EEPROM_SIZE);
    SprinklerConfig config;
    EEPROM.get(0, config);
    if(config.version == 1) 
    {
        dispname(config.disp_name);
        updsaddr(config.upds_addr);
    }
    else
    {
      Serial.println("[EEPROM] not found.");
    }
  }

  void save()
  {
    Serial.println("[EEPROM] saving");
    SprinklerConfig config = {1, 0, 0};
    strcpy(config.disp_name, disp_name.c_str());
    strcpy(config.upds_addr, upds_addr.c_str());
    EEPROM.put(0, config);
    EEPROM.commit();
  }

  void turnOn()
  {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);
  }

  void turnOff()
  {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, LOW);
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
};

#endif
