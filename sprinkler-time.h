#ifndef SPRINKLER_TIME_H
#define SPRINKLER_TIME_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <TimeLib.h>
#include <functional>

#define NTP_PACKET_SIZE 48 // NTP time is in the first 48 bytes of message

class NTPClass
{
private:

  IPAddress timeServer;
  // IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
  // IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
  // IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov

  //const int timeZone = 1;   // Central European Time
  //const int timeZone = -5;  // Eastern Standard Time (USA)
  const int timeZone = -4; // Eastern Daylight Time (USA)
  //const int timeZone = -8;  // Pacific Standard Time (USA)
  //const int timeZone = -7;  // Pacific Daylight Time (USA)

  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  WiFiUDP Udp;

  // send an NTP request to the time server at the given address
  void sendNTPpacket(IPAddress &address)
  {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
  }

public:

  time_t getTime(IPAddress &address)
  {
    while (Udp.parsePacket() > 0)
      ; // discard any previously received packets
    Serial.println("[TIMEREVR] Transmit NTP Request");
    sendNTPpacket(address);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE)
      {
        Serial.println("[TIMEREVR] Receive NTP Response");
        Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 = (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
    }
    Serial.println("[TIMEREVR] No NTP Response :-(");
    return 0; // return 0 if unable to get the time
  }

  time_t getTime()
  {
    return getTime(timeServer);
  }

  NTPClass()
  : timeServer(216,239,35,0) // time1.google.com
  {
    Udp.begin(8888);
  }
};

extern NTPClass NTP = NTPClass();

#endif
