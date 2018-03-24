#include "WirelessTeleInfo.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
// TeleInfo CallBack function (when value change occured)
void WebTeleInfo::tinfoUpdatedFrame(ValueList *me)
{

  //if pointer is null
  if (!me)
    return;

  if (!_ADCO[0])
  {
    //Find ADCO
    ValueList *adcoLookup = me;
    while (adcoLookup->next)
    {
      adcoLookup = adcoLookup->next;
      if (!strcmp_P(adcoLookup->name, PSTR("ADCO")))
        strcpy(_ADCO, adcoLookup->value);
    }
  }

  //if Jeedom upload not enabled then return
  if (!jeedom.enabled)
    return;

  //if we didn't find ADCO then we can't send stuff so return
  if (!_ADCO[0])
    return;

  //initialize fullPath to send
  _request = "";

  while (me->next)
  {
    me = me->next;

    // Add only new or updated values (And name is not ADCO (it will be added))
    if ((me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED)) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("MOTDETAT")) != 0)
    {
      _request += String("&") + me->name + '=' + me->value;
    }
  }

  //do we need to send datas
  if (_request == "")
    return;

  //create HTTP request
  HTTPClient http;

  //compose URI to request
  String completeURI;
  completeURI = completeURI + F("http") + (jeedom.tls ? F("s") : F("")) + F("://") + jeedom.hostname + F("/plugins/teleinfo/core/php/jeeTeleinfo.php?api=") + jeedom.apiKey;
  completeURI = completeURI + F("&ADCO=") + _ADCO + _request;

  //if tls is enabled or not, we need to provide certificate fingerPrint
  if (!jeedom.tls)
    http.begin(completeURI);
  else
  {
    char fpStr[41];
    http.begin(completeURI, Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint));
  }

  //run request
  _requestResult = http.GET();
  http.end();

  //LOG
  Serial.println(F("Sent"));
}

//------------------------------------------
//Parse GET request to get a Counter Label
String WebTeleInfo::GetLabel(const String &labelName)
{
  //name : 9char max
  //value : 12char max
  char value[12];
  if (!_tinfo.valueGet((char *)labelName.c_str(), value))
    return String();
  else
    return String(F("{\"")) + labelName + F("\":\"") + value + F("\"}");
}
//------------------------------------------
//Parse GET request to get ALL Counter Label
String WebTeleInfo::GetAllLabel()
{
  String galvJSON('{');

  ValueList *vl = _tinfo.getList();
  bool first = true;

  while (vl->next)
  {
    vl = vl->next;

    if (!first)
      galvJSON += ',';
    else
      first = false;
    galvJSON = galvJSON + '\"' + vl->name + F("\":\"") + vl->value + F("\"\r\n");
  }

  galvJSON += '}';

  return galvJSON;
}


void WebTeleInfo::SetConfigDefaultValues()
{
  jeedom.enabled = false;
  jeedom.tls = true;
  jeedom.hostname[0] = 0;
  jeedom.apiKey[0] = 0;
  memset(jeedom.fingerPrint, 0, 20);
}

void WebTeleInfo::ParseConfigJSON(JsonObject &root)
{
  if (root["je"].success())
    jeedom.enabled = root["je"];
  if (root["jt"].success())
    jeedom.tls = root["jt"];
  if (root["jh"].success())
    strlcpy(jeedom.hostname, root["jh"], sizeof(jeedom.hostname));
  if (root["ja"].success())
    strlcpy(jeedom.apiKey, root["ja"], sizeof(jeedom.apiKey));
  if (root["jfp"].success())
    Utils::FingerPrintS2A(jeedom.fingerPrint, root["jfp"]);
}

bool WebTeleInfo::ParseConfigWebRequest(AsyncWebServerRequest *request)
{

  char tempApiKey[48 + 1];

  if (request->hasParam(F("je"), true))
    jeedom.enabled = (request->getParam(F("je"), true)->value() == F("on"));
  else
    jeedom.enabled = false;
  if (request->hasParam(F("jt"), true))
    jeedom.tls = (request->getParam(F("jt"), true)->value() == F("on"));
  else
    jeedom.tls = false;
  if (request->hasParam(F("jh"), true) && request->getParam(F("jh"), true)->value().length() < sizeof(jeedom.hostname))
    strcpy(jeedom.hostname, request->getParam(F("jh"), true)->value().c_str());
  //put apiKey into temporary one for predefpassword
  if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempApiKey))
    strcpy(tempApiKey, request->getParam(F("ja"), true)->value().c_str());
  if (request->hasParam(F("jfp"), true))
    Utils::FingerPrintS2A(jeedom.fingerPrint, request->getParam(F("jfp"), true)->value().c_str());

  //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
  if (strcmp_P(tempApiKey, appDataPredefPassword))
    strcpy(jeedom.apiKey, tempApiKey);

  if (!jeedom.hostname[0] || !jeedom.apiKey[0])
    jeedom.enabled = false;

  return true;
}
String WebTeleInfo::GenerateConfigJSON(bool forSaveFile)
{
  char fpStr[60];

  String gc('{');

  gc = gc + F("\"je\":") + (jeedom.enabled ? true : false);
  gc = gc + F(",\"jt\":") + (jeedom.tls ? true : false);
  gc = gc + F(",\"jh\":\"") + jeedom.hostname + '"';
  if (forSaveFile)
  {
    gc = gc + F(",\"ja\":\"") + jeedom.apiKey + '"';
    Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint);
  }
  else
  {
    //there is a predefined special password (mean to keep already saved one)
    gc = gc + F(",\"ja\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"';
    Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint, ':');
  }
  gc = gc + F(",\"jfp\":\"") + fpStr + '"';

  gc = gc + '}';

  return gc;
}
String WebTeleInfo::GenerateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"a\":\"") + _ADCO + '"';
  if (jeedom.enabled)
  {
    gs = gs + F(",\"lr\":\"") + _request + '"';
    gs = gs + F(",\"lrr\":") + _requestResult;
  }

  gs = gs + '}';

  return gs;
}
bool WebTeleInfo::AppInit(bool reInit = false)
{
  if (reInit)
    return true;

//If a Pin is defined to control TeleInfo signal arrival
#ifdef TELEINFO_CONTROL_PIN
  pinMode(TELEINFO_CONTROL_PIN, OUTPUT);
  digitalWrite(TELEINFO_CONTROL_PIN, LOW);
#endif

  //try to consume Serial Data until end of TeleInfo frame
  char c = 0;
  while (c != 0x03)
  {
    if (!Serial.available())
      delay(100);
    if (Serial.available())
      c = Serial.read() & 0x7f;
    else
      break;
  }
  return c == 0x03;
}
void WebTeleInfo::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{

  server.on("/getLabel", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->hasParam(F("name")))
    {
      request->send(400, F("text/html"), F("Missing parameter"));
      return;
    }
    if (request->getParam(F("name"))->value().length() == 0)
    {
      request->send(400, F("text/html"), F("Incorrect label name"));
      return;
    }
    String labelResponse = GetLabel(request->getParam(F("name"))->value());
    if (!labelResponse.length())
      request->send(500, F("text/html"), F("Label cannot be found"));
    else
      request->send(200, F("text/json"), labelResponse);
  });

  server.on("/getAllLabel", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, F("text/json"), GetAllLabel());
  });
}

//------------------------------------------
//Run for timer
void WebTeleInfo::AppRun()
{
  if (Serial.available())
    _tinfo.process(Serial.read() & 0x7f);
}

//------------------------------------------
//WebTeleInfo Constructor
WebTeleInfo::WebTeleInfo(char appId, String appName) : Application(appId, appName)
{
  // Init and configure TeleInfo
  _ADCO[0] = 0;
  _tinfo.init();
  _tinfo.attachUpdatedFrame([this](ValueList *vl) {
    this->tinfoUpdatedFrame(vl);
  });
}
