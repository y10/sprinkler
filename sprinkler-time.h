#ifndef SPRINKLER_TIME_H
#define SPRINKLER_TIME_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <TimeLib.h>
#include <Time.h>

#define NTP_TIMEZONE  -5
#define NTP_SERVER1   "pool.ntp.org"
#define NTP_SERVER2   "time.google.com"
#define NTP_SERVER3   "time-a.timefreq.bldrdoc.gov"

class NtpClient
{
public:

  void setup()
  {
    configTime(60*60*NTP_TIMEZONE, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    Serial.print("[NTP] Connecting server");
    int tryCount = 0;
    while (tryCount++ < 20) // wait for 5 seconds
    {
      delay(250);
      Serial.print(".");
      if(time(nullptr) > 100000) 
      {
        break;
      }
    }
    Serial.println(".");

    setSyncProvider([]()
    {
      time_t t = time(nullptr);
      Serial.print("[TIME] ");
      Serial.println((String)day(t) + " " + (String)monthShortStr(month(t)) + " " + (String)year(t) + " " + (String)hour(t) + ":" + (String)minute(t));

      return t;
    });
  }
};

extern NtpClient NTP = NtpClient();

#endif
