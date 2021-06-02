#include <DNSServer.h>
#include <ESP8266WiFi.h> 
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> 
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <fauxmoESP.h>
#include "sprinkler.h"
#include "sprinkler-device-wemos.h"
#include "sprinkler-http.h"
#include "sprinkler-wss.h"
#include "sprinkler-time.h"

Ticker ticker;
fauxmoESP fauxmo;
AsyncWebServer httpServer(80);
AsyncWebSocket webSocket("/ws");
AsyncWiFiManager wifiManager;
SprinklerHttp httpSprinkler;
SprinklerWss wssSprinkler;

void setup()
{
  Serial.begin(115200);

  Serial.print("\n[MAIN] Reset reason: ");
  Serial.println(ESP.getResetReason());

  Device.setup();

  ticker.attach(0.6, tick);
  setupDevice();
  setupWifi();
  setupTime();
  setupOTA();
  setupAlexa();
  setupHttp();
  ticker.detach();

  Serial.println("[MAIN] System started.");
}

void loop()
{
  ArduinoOTA.handle();
  fauxmo.handle();
  Alarm.delay(0);
}

void setupDevice()
{
  Sprinkler.setup(Device);
}

void setupWifi()
{
  wifiManager.setFriendlyName(Device.dispname().c_str());
  WiFi.setSleepMode(WIFI_NONE_SLEEP);  
  WiFi.hostname(Device.hostname().c_str());
  
  wm_status_t status = wifiManager.autoConnect(Device.hostname().c_str());
  switch (status)
  {
  case WM_FIRST_TIME_CONNECTED:
      Device.dispname(wifiManager.getFriendlyName().c_str());
      Device.hostname(wifiManager.getDeviceName().c_str());
      Device.save();
      ESP.reset();
      break;
  case WM_CONNECT_FAILED:
      ESP.reset();
      return;
  }
}

void setupOTA()
{
  Serial.println("[MAIN] Setup OTA");
  // OTA
  // An important note: make sure that your project setting of Flash size is at least double of size of the compiled program. Otherwise OTA fails on out-of-memory.
  ArduinoOTA.onStart([]() {
    Serial.println("[MAIN] OTA: Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[MAIN] OTA: End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[MAIN] OTA progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    char errormsg[100];
    sprintf(errormsg, "OTA: Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      strcpy(errormsg + strlen(errormsg), "Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      strcpy(errormsg + strlen(errormsg), "Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      strcpy(errormsg + strlen(errormsg), "Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      strcpy(errormsg + strlen(errormsg), "Receive Failed");
    else if (error == OTA_END_ERROR)
      strcpy(errormsg + strlen(errormsg), "End Failed");
    Serial.println(errormsg);
  });
  ArduinoOTA.setHostname(Device.hostname().c_str());
  ArduinoOTA.begin();
}

void setupAlexa()
{
  // Setup Alexa devices
  if (Device.dispname().length() > 0)
  {
    fauxmo.addDevice(Device.dispname().c_str());
    Serial.print("[MAIN] Added alexa device: ");
    Serial.println(Device.dispname());
  }

  fauxmo.onSet([&](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Set Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    state ? Sprinkler.start() : Sprinkler.stop();
  });

  fauxmo.onGet([&](unsigned char device_id, const char *device_name, bool &state, unsigned char &value) {
    state = Sprinkler.isWatering();
    Serial.printf("[MAIN] Get Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
  });
}

void setupTime()
{
  // sync time
  Serial.println("[MAIN] Setup time.");

  NTP.setup();
}

void setupHttp()
{
  // Setup Web UI
  Serial.println("[MAIN] Setup http server.");
  httpSprinkler.setup(httpServer);
  wssSprinkler.setup(webSocket);
  httpServer.addHandler(&webSocket);
  httpServer.begin();
}

void tick()
{
  //toggle state
  int state = digitalRead(LED_PIN); // get the current state of GPIO pin
  digitalWrite(LED_PIN, !state);    // set pin to the opposite state
}
