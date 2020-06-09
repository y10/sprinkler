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

    int tryCount = 0;
    while (tryCount++ < 6)
    {
      delay(250);

      if(time(nullptr) > 100000) 
      {
        break;
      }
    }
    
    time_t t = time(nullptr);
    if (t)
    {
      setTime(t);
      Serial.print("[TIME] ");
      Serial.println((String)day() + " " + (String)monthShortStr(month()) + " " + (String)year() + " " + (String)hour() + ":" + (String)minute());
    }
  }
};

extern NtpClient NTP = NtpClient();

#endif
