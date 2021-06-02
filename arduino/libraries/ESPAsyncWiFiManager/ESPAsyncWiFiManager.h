/**************************************************************
 * ESPAsyncWiFiManager is a library for the ESP8266/Arduino platform
 * (https://github.com/esp8266/Arduino) to enable easy 
 * configuration and reconfiguration of WiFi credentials and 
 * store them in EEPROM.
 * inspired by http://www.esp8266.com/viewtopic.php?f=29&t=2520
 * https://github.com/chriscook8/esp-arduino-apboot 
 * Built by AlexT https://github.com/tzapu
 * Licensed under MIT license
 **************************************************************/

#ifndef ESPAsyncWiFiManager_H
#define ESPAsyncWiFiManager_H

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <memory>

typedef enum {
    WM_CONNECT_FAILED   = 0,
    WM_FIRST_TIME_CONNECTED = 1,
    WM_CONNECTED  = 2
} wm_status_t;

class AsyncWiFiManager
{
public:
    typedef std::function<AsyncWebServerResponse*(AsyncWebServerRequest *request)> OnHandleRequest;

    AsyncWiFiManager();
        
    wm_status_t autoConnect();
    wm_status_t autoConnect(char const *apName);
    wm_status_t autoConnect(char const *apName, char const *apPasswd);

    void    onHandleRootRequest(OnHandleRequest handler){ onRootRequestHandler = handler; }
    void    onHandlePostRequest(OnHandleRequest handler){ onPostRequestHandler = handler; }
    void    onHandleInfoRequest(OnHandleRequest handler){ onInfoRequestHandler = handler; }

    String  getSSID();
    String  getPassword();

    void    resetSettings();
    //for convenience
    String  urldecode(const char*);

    //sets timeout before webserver loop ends and exits even if there has been no setup. 
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds
    void    setTimeout(unsigned long seconds);
    void    setDebugOutput(boolean enable);

    //sets a custom ip /gateway /subnet configuration
    void    setAPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    void    setAPCallback( void (*func)(void) );    
 
private:
    std::unique_ptr<DNSServer> dnsServer;
    std::unique_ptr<AsyncWebServer> server;

    const int WM_DONE = 0;
    const int WM_WAIT = 10;

    void    begin();
    void    begin(char const *apName);
    void    begin(char const *apName, char const *apPasswd);
    
    int         _eepromStart;
    const char* _apPasswd = NULL;
    String      _host = "no-net";
    String      _ssid = "";
    String      _pass = "";
    unsigned long timeout = 0;
    unsigned long start = 0;
    IPAddress   _ip;
    IPAddress   _gw;
    IPAddress   _sn;
    
    String getEEPROMString(int start, int len);
    void setEEPROMString(int start, int len, String string);

    bool keepLooping = true;
    int status = WL_IDLE_STATUS;
    void connectWifi(String ssid, String pass);

    void handleRoot(AsyncWebServerRequest *request);
    void handleHostInfo(AsyncWebServerRequest *request);
    void handleWifiScan(AsyncWebServerRequest *request);
    void handleRootPost(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    void handle204(AsyncWebServerRequest *request);
    boolean captivePortal(AsyncWebServerRequest *request);
    
    // DNS server
    const byte DNS_PORT = 53;

    //helpers
    int getRSSIasQuality(int RSSI);
    boolean isIp(String str);
    String toStringIp(IPAddress ip);

    boolean connect;
    boolean debug = false;

    OnHandleRequest onRootRequestHandler;
    OnHandleRequest onPostRequestHandler;
    OnHandleRequest onInfoRequestHandler;

    template <typename Generic>
    void DEBUG_PRINT(Generic text);
};

#endif
