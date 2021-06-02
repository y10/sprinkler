#ifndef SPRINKLER_OTAUPDATE_H
#define SPRINKLER_OTAUPDATE_H

#include <ESPAsyncTCP.h>
#include "includes\URL.h"
#include "sprinkler.h"

#define Log Serial

const char OTA_REQUEST_TEMPLATE[] PROGMEM =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "User-Agent: SmartSwitch\r\n"
    "Connection: close\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 0\r\n\r\n\r\n";

unsigned long _ota_size = 0;
bool _ota_connected = false;
Url* _ota_url = nullptr;
AsyncClient* _ota_client = nullptr;
Ticker _ota_ticker;

void _otaClientOnDisconnect(void *s, AsyncClient *c) {

    Log.print("\n");

    if (Update.end(true)){
        Log.printf("[OTA] Success: %u bytes\n", _ota_size);
        _ota_ticker.once_ms(100, []{
            ESP.restart();
        });
    } else {
        Update.printError(Log);
    }

    Log.println("[OTA] Disconnected");

    _ota_connected = false;
    _ota_url = nullptr;
    _ota_client = nullptr;

}

void _otaClientOnTimeout(void *s, AsyncClient *c, uint32_t time) {
    Log.println("[OTA] Timeout");
    _ota_connected = false;
    _ota_url = nullptr;
    c->close(true);
}

void _otaClientOnData(void * arg, AsyncClient * c, void * data, size_t len) {

    char * p = (char *) data;

    if (_ota_size == 0) {

        Update.runAsync(true);

        uint32_t space = ESP.getFreeSketchSpace();

        if (!Update.begin((space - 0x1000) & 0xFFFFF000)) {
            Update.printError(Log);
            c->close(true);
            return;
        }

        p = strstr((char *)data, "\r\n\r\n") + 4;
        len = len - (p - (char *) data);
    }

    if (!Update.hasError()) {
        if (Update.write((uint8_t *) p, len) != len) {
            c->close(true);
            return;
        }
    }
    else
    {
        Update.printError(Log);
    }

    _ota_size += len;
    
    delay(0);
}

void _otaClientOnConnect(void *arg, AsyncClient *client) {

    // Disabling EEPROM rotation to prevent writing to EEPROM after the upgrade
    //eepromRotate(false);

    Log.printf("[OTA] Downloading %s\n", _ota_url->path.c_str());
    char buffer[strlen_P(OTA_REQUEST_TEMPLATE) + _ota_url->path.length() + _ota_url->host.length()];
    snprintf_P(buffer, sizeof(buffer), OTA_REQUEST_TEMPLATE, _ota_url->path.c_str(), _ota_url->host.c_str());
    client->write(buffer);
}

class SprinklerOTA
{
    public:        

        void update(const String& url) {

            if (_ota_connected) {
                Log.print(PSTR("[OTA] Already connected\n"));
                return;
            }

            _ota_size = 0;

            if (_ota_url) _ota_url = nullptr;

            _ota_url = new Url(url);
            
            // we only support HTTP
            if ((!_ota_url->protocol.equals("http")) && (!_ota_url->protocol.equals("https"))) {
                Log.print(PSTR("[OTA] Incorrect URL specified\n"));
                _ota_url = nullptr;
                return;
            }

            if (!_ota_client) 
            {
                _ota_client = new AsyncClient();
                _ota_client->onDisconnect(_otaClientOnDisconnect, nullptr);
                _ota_client->onTimeout(_otaClientOnTimeout, nullptr);
                _ota_client->onData(_otaClientOnData, nullptr);
                _ota_client->onConnect(_otaClientOnConnect, nullptr);
            }

            #if ASYNC_TCP_SSL_ENABLED
                _ota_connected = _ota_client->connect(_ota_url->host.c_str(), _ota_url->port, 443 == _ota_url->port);
            #else
                _ota_connected = _ota_client->connect(_ota_url->host.c_str(), _ota_url->port);
            #endif

            if (!_ota_connected) {
                Log.print("[OTA] Connection failed\n");
                _ota_url = nullptr;
                _ota_client->close(true);
            }
        }        

};

extern SprinklerOTA OTA = SprinklerOTA();

#endif