#ifndef SPRINKLER_TIME_H
#define SPRINKLER_TIME_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Time.h>
#include <TimeLib.h>

#define NTP_TIMEZONE -5
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define NTP_SERVER3 "time.google.com"


class NtpClient {
 public:
  void setup() {
    Serial.print("Built: ");
    Serial.println(builtDate(&builtDateTime));
    setSyncProvider(0);
    configTime(60 * 60 * NTP_TIMEZONE, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    Serial.println("Connecting...");
    if (!sync()) {
      setTime(builtDateTime);
      Serial.println("Failed.");
    }
  }

 private:
  const char* builtDate(time_t* dt) const {
    if (dt) {
      struct tm t;
      if (strptime(__DATE__ " " __TIME__ " GMT", "%b %d %Y %H:%M:%S GMT", &t)) {
        *dt = mktime(&t);
      }
    }
    return __DATE__ " " __TIME__ " GMT";
  }

  time_t sync() {
    int tryCount = 0;
    while (tryCount++ < 20)  // wait for 5 seconds
    {
      delay(250);
      Serial.print(".");
      time_t t = time(nullptr);
      if (t > builtDateTime) {
        Serial.println();
        setTime(t);
        Serial.println((String)day(t) + " " + (String)monthShortStr(month(t)) + " " + (String)year(t) + " " + (String)hour(t) + ":" + (String)minute(t));
        return t;
      }
    }
    Serial.println(".");
    timer.once(300, [&] {
      Serial.print("Reconecting...");
      sync();
    });

    return (time_t)0;
  }

  time_t builtDateTime;
  Ticker timer;
};

extern NtpClient NTP = NtpClient();

#endif
