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

  //if Home Automation upload not enabled then return
  if (!ha.enabled)
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

  String completeURI;

  //compose URI to request
  switch (ha.enabled)
  {
  case 1:
    completeURI = completeURI + F("http") + (ha.tls ? F("s") : F("")) + F("://") + ha.hostname + F("/plugins/teleinfo/core/php/jeeTeleinfo.php?api=") + ha.jeedom.apiKey;
    completeURI = completeURI + F("&ADCO=") + _ADCO + _request;
    break;
  }
  //if tls is enabled or not, we need to provide certificate fingerPrint
  if (!ha.tls)
    http.begin(completeURI);
  else
  {
    char fpStr[41];
    http.begin(completeURI, Utils::FingerPrintA2S(fpStr, ha.fingerPrint));
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
  ha.enabled = 0;
  ha.tls = false;
  ha.hostname[0] = 0;
  memset(ha.fingerPrint, 0, 20);

  ha.jeedom.apiKey[0] = 0;
}

void WebTeleInfo::ParseConfigJSON(JsonObject &root)
{
  //Retrocompatibility block to be removed after v3.1.2 --
  if (root["je"].success())
    ha.enabled = root["je"] ? 1 : 0;
  if (root["jt"].success())
    ha.tls = root["jt"];
  if (root["jh"].success())
    strlcpy(ha.hostname, root["jh"], sizeof(ha.hostname));
  if (root["jfp"].success())
    Utils::FingerPrintS2A(ha.fingerPrint, root["jfp"]);
  // --

  if (root[F("hae")].success())
    ha.enabled = root[F("hae")];
  if (root[F("hatls")].success())
    ha.tls = root[F("hatls")];
  if (root[F("hah")].success())
    strlcpy(ha.hostname, root["hah"], sizeof(ha.hostname));
  if (root["hafp"].success())
    Utils::FingerPrintS2A(ha.fingerPrint, root["hafp"]);

  if (root["ja"].success())
    strlcpy(ha.jeedom.apiKey, root["ja"], sizeof(ha.jeedom.apiKey));
}

bool WebTeleInfo::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam(F("hae"), true))
    ha.enabled = request->getParam(F("hae"), true)->value().toInt();

  //if an home Automation system is enabled then get common param
  if (ha.enabled)
  {
    if (request->hasParam(F("hatls"), true))
      ha.tls = (request->getParam(F("hatls"), true)->value() == F("on"));
    else
      ha.tls = false;
    if (request->hasParam(F("hah"), true) && request->getParam(F("hah"), true)->value().length() < sizeof(ha.hostname))
      strcpy(ha.hostname, request->getParam(F("hah"), true)->value().c_str());
    if (request->hasParam(F("hafp"), true))
      Utils::FingerPrintS2A(ha.fingerPrint, request->getParam(F("hafp"), true)->value().c_str());
  }

  //Now get specific param
  switch (ha.enabled)
  {
  case 1: //Jeedom
    char tempApiKey[48 + 1];
    //put apiKey into temporary one for predefpassword
    if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempApiKey))
      strcpy(tempApiKey, request->getParam(F("ja"), true)->value().c_str());
    //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempApiKey, appDataPredefPassword))
      strcpy(ha.jeedom.apiKey, tempApiKey);
    if (!ha.hostname[0] || !ha.jeedom.apiKey[0])
      ha.enabled = 0;
    break;
  }

  return true;
}
String WebTeleInfo::GenerateConfigJSON(bool forSaveFile)
{
  char fpStr[60];

  String gc('{');

  gc = gc + F("\"hae\":") + ha.enabled;
  gc = gc + F(",\"hatls\":") + ha.tls;
  gc = gc + F(",\"hah\":\"") + ha.hostname + '"';
  gc = gc + F(",\"hafp\":\"") + Utils::FingerPrintA2S(fpStr, ha.fingerPrint, forSaveFile ? 0 : ':') + '"';
  if (forSaveFile)
  {
    if (ha.enabled == 1)
      gc = gc + F(",\"ja\":\"") + ha.jeedom.apiKey + '"';
  }
  else
    gc = gc + F(",\"ja\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)

  gc = gc + '}';

  return gc;
}
String WebTeleInfo::GenerateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"a\":\"") + _ADCO + '"';
  if (ha.enabled)
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
