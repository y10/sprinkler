#ifndef SPRINKLER_DEVICE_H
#define SPRINKLER_DEVICE_H

#include <EEPROM.h>
#include <ESP8266WiFi.h>

#include "schedule.h"
#include "sprinkler.h"

#define EEPROM_SIZE 1024

struct SchedulerConfig {
  bool enabled;
  unsigned int hour;
  unsigned int minute;
  unsigned int duration;
};

struct SprinklerConfig {
  uint8_t version;
  char full_name[50];
  char host_name[50];
  char disp_name[50];
  SchedulerConfig scheduler[8];
};

class SprinklerDevice {
 private:
  std::function<void()> onSetup;
  uint8_t led_pin;
  uint8_t rel_pin;

  String host_name;
  String disp_name;
  String upds_addr;
  String full_name;

  uint8_t revision;

 public:
  SprinklerDevice(std::function<void(void)> onSetupCallback, uint8_t led, uint8_t rel)
      : onSetup(onSetupCallback), led_pin(led), rel_pin(rel), revision(1) {
    disp_name = "Sprinkler";
    host_name = "sprinkler-" + String(ESP.getChipId(), HEX);
    full_name = "sprinkler-v" + (String)SKETCH_VERSION_MAJOR + "." + (String)SKETCH_VERSION_MINOR + "." + (String)SKETCH_VERSION_RELEASE + "_" + String(ESP.getChipId(), HEX);
    upds_addr = "http://ota.voights.net/sprinkler.bin";
  }

  const String hostname() const {
    return host_name;
  }

  bool hostname(const char *name) {
    bool changed = false;
    if (strlen(name) > 0) {
      if (!host_name.equals(name)) {
        host_name = name;
        changed = true;
      }
    }

    return changed;
  }

  const String dispname() const {
    return disp_name;
  }

  bool dispname(const char *name) {
    bool changed = false;
    if (strlen(name) > 0) {
      if (!disp_name.equals(name)) {
        disp_name = name;
        changed = true;
      }
    }

    return changed;
  }

  const String updsaddr() const {
    return upds_addr;
  }

  const String updsaddr(const char *addr) {
    upds_addr = addr;

    if (upds_addr.indexOf("://") == -1) {
      upds_addr = "http://" + upds_addr;
    }

    return upds_addr;
  }

  void setup() {
    onSetup();
  }

  void load() {
    Serial.println("[EEPROM] reading...");
    EEPROM.begin(EEPROM_SIZE);
    SprinklerConfig config;
    EEPROM.get(0, config);
    if (full_name.equals(config.full_name)) {
      Serial.print("[EEPROM] Display Name: ");
      dispname(config.disp_name);
      Serial.println(disp_name);
      Serial.print("[EEPROM] Host Name: ");
      hostname(config.host_name);
      Serial.println(host_name);
      revision = config.version;

      Schedule.setDuration(config.scheduler[0].duration);
      Schedule.setHour(config.scheduler[0].hour);
      Schedule.setMinute(config.scheduler[0].minute);
      if (config.scheduler[0].enabled) Schedule.enable();

      for (int day = (int)dowSunday; day <= (int)dowSaturday; day++) {
        ScheduleClass &skd = Schedule.get((timeDayOfWeek_t)day);
        skd.setDuration(config.scheduler[day].duration);
        skd.setHour(config.scheduler[day].hour);
        skd.setMinute(config.scheduler[day].minute);

        if (config.scheduler[day].enabled) skd.enable();
      }
    } else {
      Serial.println("[EEPROM] not found.");
    }
  }

  void save() {
    Serial.println("[EEPROM] saving");
    SprinklerConfig config = {
        /*version*/   ++revision,
        /*full_name*/ {0},
        /*host_name*/ {0},
        /*disp_name*/ {0},
        /*scheduler*/ {
           /*everyday*/ {Schedule.isEnabled(),     Schedule.getHour(), Schedule.getMinute(),     Schedule.getDuration()},
            /*sun*/ {Schedule.Sun.isEnabled(), Schedule.Sun.getHour(), Schedule.Sun.getMinute(), Schedule.Sun.getDuration()},
            /*mon*/ {Schedule.Mon.isEnabled(), Schedule.Mon.getHour(), Schedule.Mon.getMinute(), Schedule.Mon.getDuration()},
            /*tue*/ {Schedule.Tue.isEnabled(), Schedule.Tue.getHour(), Schedule.Tue.getMinute(), Schedule.Tue.getDuration()},
            /*wed*/ {Schedule.Wed.isEnabled(), Schedule.Wed.getHour(), Schedule.Wed.getMinute(), Schedule.Wed.getDuration()},
            /*thu*/ {Schedule.Thu.isEnabled(), Schedule.Thu.getHour(), Schedule.Thu.getMinute(), Schedule.Thu.getDuration()},
            /*fri*/ {Schedule.Fri.isEnabled(), Schedule.Fri.getHour(), Schedule.Fri.getMinute(), Schedule.Fri.getDuration()},
            /*sat*/ {Schedule.Sat.isEnabled(), Schedule.Sat.getHour(), Schedule.Sat.getMinute(), Schedule.Sat.getDuration()}
      }
    };
    strcpy(config.full_name, full_name.c_str());
    strcpy(config.host_name, host_name.c_str());
    strcpy(config.disp_name, disp_name.c_str());
    EEPROM.put(0, config);
    EEPROM.commit();
  }

  void turnOn() {
    digitalWrite(led_pin, LOW);
    digitalWrite(rel_pin, HIGH);
  }

  void turnOff() {
    digitalWrite(led_pin, HIGH);
    digitalWrite(rel_pin, LOW);
  }

  virtual void reset() {
    Serial.println("[MAIN] Factory reset requested.");
    Serial.println("[EEPROM] clear");
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();

    WiFi.disconnect(true);
    ESP.restart();
  }

  virtual void restart() {
    Serial.println("[MAIN] Restarting...");
    ESP.restart();
  }

  String toJSON() {
    return (String) "{" +
           "\r\n  \"disp_name\": \"" + disp_name + "\"" +
           "\r\n ,\"host_name\": \"" + host_name + "\"" +
           "\r\n ,\"upds_addr\": \"" + upds_addr + "\"" +
           "\r\n}";
  }
};

#endif
