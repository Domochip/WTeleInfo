#ifndef WirelessTeleInfo_h
#define WirelessTeleInfo_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "src\Utils.h"

#include "LibTeleInfo.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

//Structure of Application Data 1
class AppData1 {

  public:

    typedef struct {
      bool enabled = false;
      bool tls = false;
      char hostname[64 + 1] = {0};
      char apiKey[48 + 1] = {0};
      byte fingerPrint[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    } Jeedom;
    Jeedom jeedom;

    void SetDefaultValues() {

      jeedom.enabled = false;
      jeedom.tls = true;
      jeedom.hostname[0] = 0;
      jeedom.apiKey[0] = 0;
      memset(jeedom.fingerPrint, 0, 20);
    }

    String GetJSON();
    bool SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData);
};


class WebTeleInfo {

  private:
    AppData1* _appData1;

    //for returning Status
    String _request;
    int _requestResult;
    char _ADCO[13] = {0};

    TInfo _tinfo;

    void tinfoUpdatedFrame(ValueList* me);
    String GetLabel(const String &labelName);
    String GetAllLabel();

    String GetStatus();

  public:
    WebTeleInfo();
    void Init(AppData1 &appData1);
    void InitWebServer(AsyncWebServer &server);
    void Run();
};

#endif
