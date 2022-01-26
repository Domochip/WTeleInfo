#ifndef WirelessTeleInfo_h
#define WirelessTeleInfo_h

#include "Main.h"
#include "base\Utils.h"
#include "base\MQTTMan.h"
#include "base\Application.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include "LibTeleInfo.h"

class WebTeleInfo : public Application
{
private:
#define HA_HTTP_GENERIC 0
#define HA_HTTP_JEEDOM_TELEINFO 1

  typedef struct
  {
    byte type = HA_HTTP_GENERIC;
    bool tls = false;
    byte fingerPrint[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct
    {
      char uriPattern[150 + 1] = {0};
    } generic;
    struct
    {
      char apiKey[48 + 1] = {0};
    } jeedom;
  } HTTP;

#define HA_MQTT_GENERIC1 0
#define HA_MQTT_GENERIC2 1

  typedef struct
  {
    byte type = HA_MQTT_GENERIC1;
    uint32_t port = 1883;
    char username[128 + 1] = {0};
    char password[150 + 1] = {0};
    struct
    {
      char baseTopic[64 + 1] = {0};
    } generic;
  } MQTT;

#define HA_PROTO_DISABLED 0
#define HA_PROTO_HTTP 1
#define HA_PROTO_MQTT 2

  typedef struct
  {
    byte protocol = HA_PROTO_DISABLED;
    char hostname[64 + 1] = {0};
    uint16_t uploadPeriod = 60;
    HTTP http;
    MQTT mqtt;
  } HomeAutomation;

  _Mode_e _tinfoMode = TINFO_MODE_HISTORIQUE;

  HomeAutomation _ha;

  //for returning Status
  String _httpJeedomRequest;
  int _haSendResult = 0;
  char _ADCO[13] = {0};
  WiFiClient _wifiClient;
  WiFiClientSecure _wifiClientSecure;

  TInfo _tinfo;
  bool _needPublish = false;
  Ticker _publishTicker;

  MQTTMan _mqttMan;

  void tinfoUpdatedFrame(ValueList *me);
  String getLabel(const String &labelName);
  String getAllLabel();
  void mqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection);
  void mqttCallback(char *topic, uint8_t *payload, unsigned int length);
  void publishTick(bool publishAll);

  void setConfigDefaultValues();
  void parseConfigJSON(DynamicJsonDocument &doc);
  bool parseConfigWebRequest(AsyncWebServerRequest *request);
  String generateConfigJSON(bool forSaveFile);
  String generateStatusJSON();
  bool appInit(bool reInit);
  const uint8_t *getHTMLContent(WebPageForPlaceHolder wp);
  size_t getHTMLContentSize(WebPageForPlaceHolder wp);
  void appInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void appRun();

public:
  WebTeleInfo(char appId, String appName);
};

#endif
