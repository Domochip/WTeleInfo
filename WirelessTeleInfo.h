#ifndef WirelessTeleInfo_h
#define WirelessTeleInfo_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "src\Utils.h"
#include "src\Base.h"

#include <ESP8266HTTPClient.h>
#include "LibTeleInfo.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

class WebTeleInfo : public Application {

  private:
    typedef struct {
      bool enabled = false;
      bool tls = false;
      char hostname[64 + 1] = {0};
      char apiKey[48 + 1] = {0};
      byte fingerPrint[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    } Jeedom;
    Jeedom jeedom;

    //for returning Status
    String _request;
    int _requestResult;
    char _ADCO[13] = {0};

    TInfo _tinfo;

    void tinfoUpdatedFrame(ValueList* me);
    String GetLabel(const String &labelName);
    String GetAllLabel();


    void SetConfigDefaultValues();
    void ParseConfigJSON(JsonObject &root);
    bool ParseConfigWebRequest(AsyncWebServerRequest *request);
    String GenerateConfigJSON(bool forSaveFile);
    String GenerateStatusJSON();
    bool AppInit(bool reInit);
    void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
    void AppRun();
  public:
    WebTeleInfo(char appId, String appName);
};

#endif
