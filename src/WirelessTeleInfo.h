#ifndef WirelessTeleInfo_h
#define WirelessTeleInfo_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "base\Utils.h"
#include "base\Application.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "LibTeleInfo.h"

#define TELEINFO_PIN D5

class WebTeleInfo : public Application
{
private:
#define HA_HTTP_GENERIC 0
#define HA_HTTP_JEEDOM_TELEINFO 1

  typedef struct
  {
    byte type = HA_HTTP_GENERIC;
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

#define HA_MQTT_GENERIC 0

  typedef struct
  {
    byte type = HA_MQTT_GENERIC;
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
    bool tls = false;
    char hostname[64 + 1] = {0};
    uint16_t uploadPeriod = 60;
    HTTP http;
    MQTT mqtt;
  } HomeAutomation;

  HomeAutomation _ha;

  //for returning Status
  String _httpJeedomRequest;
  int _haSendResult = 0;
  char _ADCO[13] = {0};

  TInfo _tinfo;
  bool _needPublish = false;
  Ticker _publishTicker;
  WiFiClient _wifiMqttClient;
  WiFiClientSecure _wifiMqttClientSecure;
  PubSubClient _mqttClient;
  bool _needMqttReconnect = false;
  Ticker _mqttReconnectTicker;

  void tinfoUpdatedFrame(ValueList *me);
  String GetLabel(const String &labelName);
  String GetAllLabel();
  bool MqttConnect();
  void PublishTick(bool publishAll);

  void SetConfigDefaultValues();
  void ParseConfigJSON(DynamicJsonDocument &doc);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool forSaveFile);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  const uint8_t *GetHTMLContent(WebPageForPlaceHolder wp);
  size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun();

public:
  WebTeleInfo(char appId, String appName);
};

#endif
