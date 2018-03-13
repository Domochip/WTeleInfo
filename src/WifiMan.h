#ifndef WifiMan_h
#define WifiMan_h

#include <ESP8266WiFi.h>

#include "..\Main.h"

class WifiMan {

  private:

    WiFiEventHandler _wifiHandler1, _wifiHandler2;
    int _apChannel = 2;
    char _apSsid[64];
    uint16_t _retryPeriod = 300;
    bool _retryStation = false;

    uint32_t _ip = 0;
    uint32_t _gw = 0;
    uint32_t _mask = 0;
    uint32_t _dns1 = 0;
    uint32_t _dns2 = 0;

  public:
    bool ConfigIP(uint32_t ip, uint32_t gw, uint32_t mask, uint32_t dns1 = 0, uint32_t dns2 = 0);
    bool Init(char* ssid, char* password, char* hostname, char* apSSID, char* apPassword, uint16_t retryPeriod = 300);
    void Run();
};

#endif

