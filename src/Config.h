#ifndef Config_h
#define Config_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "..\Main.h"

const char predefPassword[] PROGMEM = "ewcXoCt4HHjZUvY0";

class SystemData {
  public:
    char ssid[32 + 1] = {0};
    char password[64 + 1] = {0};
    char hostname[24 + 1] = {0};
    uint32_t ip = 0;
    uint32_t gw = 0;
    uint32_t mask = 0;
    uint32_t dns1 = 0;
    uint32_t dns2 = 0;

    void SetDefaultValues() {
      ssid[0] = 0;
      password[0] = 0;
      hostname[0] = 0;
      ip = 0;
      gw = 0;
      mask = 0;
      dns1 = 0;
      dns2 = 0;
    }

    String GetJSON();

    bool SetFromParameters(AsyncWebServerRequest* request, SystemData &tempSystemData);
};

class Config {
  public:

    SystemData systemData;
    AppData1 appData1;

    void SetDefaultValues() {
      systemData.SetDefaultValues();
      appData1.SetDefaultValues();
    }

    bool Save();
    bool Load();
    void InitWebServer(AsyncWebServer &server, bool &shouldReboot);
  private :
    String GetJSON();
    bool SetFromParameters(AsyncWebServerRequest* request);
    uint16_t crc; ///!\ crc should always stay in last position
};

#endif

