#ifndef WifiMan_h
#define WifiMan_h

#include "..\Main.h" //for DEFAULT_AP_SSID and ...PSK

#include "data\statusw.html.gz.h"
#include "data\configw.html.gz.h"

const char predefPassword[] PROGMEM = "ewcXoCt4HHjZUvY0";

class WifiMan : public Application
{

private:
  //Configuration Properties
  char ssid[32 + 1] = {0};
  char password[64 + 1] = {0};
  char hostname[24 + 1] = {0};
  uint32_t ip = 0;
  uint32_t gw = 0;
  uint32_t mask = 0;
  uint32_t dns1 = 0;
  uint32_t dns2 = 0;

  //Run properties
  WiFiEventHandler _wifiHandler1, _wifiHandler2;
  int _apChannel = 2;
  char _apSsid[64];
  uint16_t _retryPeriod = 300;
  bool _retryStation = false;

  void SetConfigDefaultValues();
  void ParseConfigJSON(DynamicJsonDocument &doc);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool forSaveFile);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  const uint8_t* GetHTMLContent(WebPageForPlaceHolder wp);
  size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun();

public:
  WifiMan(char appId, String appName) : Application(appId, appName) {}
};

#endif
