#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson 
#include <fauxmoESP.h>
#include "sprinkler.h"
#include "sprinkler-http.h"
#include "sprinkler-wss.h"
#include "sprinkler-time.h"

/************ Defines ******************/

#define RELAY_1 13
#define RELAY_2 12
#define LED 16
#define BTN1 2
#define BTN2 0

/************ Global State ******************/
DNSServer dns;
AsyncWebServer httpServer(80);
AsyncWebSocket webSocket("/ws");
AsyncWiFiManager wifiManager(&httpServer, &dns);
fauxmoESP fauxmo;
Ticker ticker;
SprinklerHttp httpSprinkler;
SprinklerWss wssSprinkler;

char relay1_name[50] = "Sprinkler";

//flag for saving data
bool shouldSaveConfig = false;

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

  Serial.print("\n[MAIN] Reset reason: ");
  Serial.println(ESP.getResetReason());

  //set led pin as output
  pinMode(LED, OUTPUT);

  //attach reset handler
  pinMode(BTN1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN1), reset, CHANGE);

  // set relay as outputs
  pinMode(RELAY_1, OUTPUT);

  Sprinkler.onTurnOn(turnOn);
  Sprinkler.onTurnOff(turnOff);

  ticker.attach(0.6, tick);
  setupSPIFFS();
  setupWifi();
  setupOTA();
   //save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfig();  
    //end save
  }
  setupAlexa();
  setupTime();  
  setupHttp();
  ticker.detach();
  
  Serial.println("[MAIN] System started.");
}

void loop() {
  ArduinoOTA.handle();
  fauxmo.handle();
  Alarm.delay(0);
}

void reset() {
  Serial.println("[MAIN] Factory reset requested.");
  
  wifiManager.resetSettings();
  SPIFFS.format();
  system_restart();

  delay(5000);
}

void turnOn() {
  digitalWrite(RELAY_1, HIGH);
}

void turnOff() {
  digitalWrite(RELAY_1, LOW);
}

void setupSPIFFS()  {

  //read configuration from FS json
  Serial.println("[MAIN] Mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("[MAIN] mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("[MAIN] reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("[MAIN] opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file..0

        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\n[MAIN] parsed json");

          strcpy(relay1_name, json["relay1_name"]);

        } else {
          Serial.println("\n[MAIN] failed to load json config");
        }
      }
    }
  } else {
    Serial.println("[MAIN] failed to mount FS");
  }

  //end read
}

void setupWifi() {

   //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
   wifiManager.setAPCallback(configModeCallback);
   //set config save notify callback
   wifiManager.setSaveConfigCallback(saveConfigCallback);
 
   // The extra parameters to be configured (can be either global or just in the setup)
   // After connecting, parameter.getValue() will get you the configured value
   // id/name placeholder/prompt default length
   AsyncWiFiManagerParameter  custom_relay1_name("relay1_name", "Name", relay1_name, sizeof(relay1_name) - 1);
 
   //add all your parameters here
   wifiManager.addParameter(&custom_relay1_name);
 
   wifiManager.setConfigPortalTimeout(300); // wait 5 minutes for Wifi config and then return

   String hostname("Sprinkler-");
   hostname += String(ESP.getChipId(), HEX);
   hostname += "-ED";
  
   if (!wifiManager.autoConnect(hostname.c_str())) {
     Serial.println("[MAIN] failed to connect and hit timeout");
     ESP.reset();
   }
 
   //if you get here you have connected to the WiFi
   Serial.println("[MAIN] connected to Wifi");
 
  //read updated parameters
  strcpy(relay1_name, custom_relay1_name.getValue());  
}

void setupOTA() {
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
    if (error == OTA_AUTH_ERROR) strcpy(errormsg + strlen(errormsg), "Auth Failed");
    else if (error == OTA_BEGIN_ERROR) strcpy(errormsg + strlen(errormsg), "Begin Failed");
    else if (error == OTA_CONNECT_ERROR) strcpy(errormsg + strlen(errormsg), "Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) strcpy(errormsg + strlen(errormsg), "Receive Failed");
    else if (error == OTA_END_ERROR) strcpy(errormsg + strlen(errormsg), "End Failed");
    Serial.println(errormsg);
  });
  ArduinoOTA.begin();
}

void saveConfig()
{
  Serial.println("[MAIN] saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["relay1_name"] = relay1_name;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("[MAIN] failed to open config file for writing");
  }

  json.printTo(Serial);
  Serial.println();
  json.printTo(configFile);
  configFile.close();
}

void setupAlexa()
{
  // Setup Alexa devices
  if ((sizeof(relay1_name) - 1) > 0)  {
    fauxmo.addDevice(relay1_name);
    Serial.print("[MAIN] Added alexa device: ");
    Serial.println(relay1_name);
  }

  fauxmo.onSet([](unsigned char device_id, const char *device_name, bool state) {
    Serial.printf("[MAIN] Set Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    state ? Sprinkler.start() : Sprinkler.stop();
  });

  fauxmo.onGet([](unsigned char device_id, const char *device_name) {
    bool state = Sprinkler.isWatering();
    Serial.printf("[MAIN] Get Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    return state;
  });
}

void setupTime()
{
  // sync time
  Serial.println("[MAIN] Setup time synchronization");
  setSyncProvider([](){return NTP.getTime();});
  setSyncInterval(3600);  
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
  int state = digitalRead(LED);  // get the current state of GPIO pin
  digitalWrite(LED, !state);     // set pin to the opposite state
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("[MAIN] Should save config");
  shouldSaveConfig = true;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("[MAIN] Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}
