#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "WirelessTeleInfo.h"


//Return JSON of AppData1 content
String AppData1::GetJSON() {

  char fpStr[60];
  String gc;

  //there is a predefined special password (mean to keep already saved one)
  gc = gc + F("\"je\":\"") + (jeedom.enabled ? F("on") : F("off")) + F("\",\"jt\":\"") + (jeedom.tls ? F("on") : F("off")) + F("\",\"jh\":\"") + jeedom.hostname + F("\",\"ja\":\"") + (__FlashStringHelper*)appDataPredefPassword + F("\",\"jfp\":\"") + Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint, ':') + '"';

  return gc;
}

//Parse HTTP Request into an AppData1 structure
bool AppData1::SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData1) {

  if (request->hasParam(F("je"), true)) tempAppData1.jeedom.enabled = (request->getParam(F("je"), true)->value() == F("on"));
  if (request->hasParam(F("jt"), true)) tempAppData1.jeedom.tls = (request->getParam(F("jt"), true)->value() == F("on"));
  if (request->hasParam(F("jh"), true) && request->getParam(F("jh"), true)->value().length() < sizeof(tempAppData1.jeedom.hostname)) strcpy(tempAppData1.jeedom.hostname, request->getParam(F("jh"), true)->value().c_str());
  if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempAppData1.jeedom.apiKey)) strcpy(tempAppData1.jeedom.apiKey, request->getParam(F("ja"), true)->value().c_str());
  if (!tempAppData1.jeedom.hostname[0] || !tempAppData1.jeedom.apiKey[0]) tempAppData1.jeedom.enabled = false;
  if (request->hasParam(F("jfp"), true)) Utils::FingerPrintS2A(tempAppData1.jeedom.fingerPrint, request->getParam(F("jfp"), true)->value().c_str());

  //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
  if (!strcmp_P(tempAppData1.jeedom.apiKey, appDataPredefPassword)) strcpy(tempAppData1.jeedom.apiKey, jeedom.apiKey);

  return true;
}




//------------------------------------------
// TeleInfo CallBack function (when value change occured)
void WebTeleInfo::tinfoUpdatedFrame(ValueList* me) {

  //if pointer is null
  if (!me) return;

  if (!_ADCO[0]) {
    //Find ADCO
    ValueList* adcoLookup = me;
    while (adcoLookup->next) {
      adcoLookup = adcoLookup->next;
      if (!strcmp_P(adcoLookup->name, PSTR("ADCO"))) strcpy(_ADCO, adcoLookup->value);
    }
  }

  //if Jeedom upload not enabled then return
  if (!_appData1->jeedom.enabled) return;

  //if we didn't find ADCO then we can't send stuff so return
  if (!_ADCO[0]) return;


  //initialize fullPath to send
  _request = "";

  while (me->next) {
    me = me->next;

    // Add only new or updated values (And name is not ADCO (it will be added))
    if ((me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED)) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("MOTDETAT")) != 0) {
      _request += String("&") + me->name + '=' + me->value;
    }
  }

  //do we need to send datas
  if (_request == "") return;

  //create HTTP request
  HTTPClient http;

  //compose URI to request
  String completeURI;
  completeURI = completeURI + F("http") + (_appData1->jeedom.tls ? F("s") : F("")) + F("://") + _appData1->jeedom.hostname + F("/plugins/teleinfo/core/php/jeeTeleinfo.php?api=") + _appData1->jeedom.apiKey;
  completeURI = completeURI + F("&ADCO=") + _ADCO + _request;

  //if tls is enabled or not, we need to provide certificate fingerPrint
  if (!_appData1->jeedom.tls) http.begin(completeURI);
  else {
    char fpStr[41];
    http.begin(completeURI, Utils::FingerPrintA2S(fpStr, _appData1->jeedom.fingerPrint));
  }

  //run request
  _requestResult = http.GET();
  http.end();

  //LOG
  Serial.println(F("Sent"));
}

//------------------------------------------
//Parse GET request to get a Counter Label
String WebTeleInfo::GetLabel(const String &labelName) {
  //name : 9char max
  //value : 12char max
  char value[12];
  if (!_tinfo.valueGet((char*)labelName.c_str(), value)) return String();
  else return String(F("{\"")) + labelName + F("\":\"") + value + F("\"}");
}
//------------------------------------------
//Parse GET request to get ALL Counter Label
String WebTeleInfo::GetAllLabel() {
  String galvJSON('{');

  ValueList* vl = _tinfo.getList();
  bool first = true;

  while (vl->next) {
    vl = vl->next;

    if (!first) galvJSON += ',';
    else first = false;
    galvJSON = galvJSON + '\"' + vl->name + F("\":\"") + vl->value + F("\"\r\n");
  }

  galvJSON += '}';

  return galvJSON;
}

//------------------------------------------
//return WebTeleInfo Status in JSON
String WebTeleInfo::GetStatus() {

  String statusJSON('{');
  statusJSON = statusJSON + F("\"a\":\"") + _ADCO + F("\"");
  if (_appData1->jeedom.enabled) statusJSON = statusJSON + F(", \"lr\":\"") + _request + F("\",\"lrr\":") + _requestResult;
  statusJSON += '}';

  return statusJSON;
}

//------------------------------------------
//WebTeleInfo Constructor
WebTeleInfo::WebTeleInfo() {
  // Init and configure TeleInfo
  _ADCO[0] = 0;
  _tinfo.init();
  _tinfo.attachUpdatedFrame([this](ValueList * vl) {
    this->tinfoUpdatedFrame(vl);
  });
}
//------------------------------------------
//Function to initiate WebTeleInfo with Config
void WebTeleInfo::Init(AppData1 &appData1) {

  Serial.print(F("Start WebTeleInfo"));

  _appData1 = &appData1;

  //If a Pin is defined to control TeleInfo signal arrival
#ifdef TELEINFO_CONTROL_PIN
  pinMode(TELEINFO_CONTROL_PIN, OUTPUT);
  digitalWrite(TELEINFO_CONTROL_PIN, LOW);
#endif

  //try to consume Serial Data until end of TeleInfo frame
  char c = 0;
  while (c != 0x03) {
    if (!Serial.available()) delay(100);
    if (Serial.available()) c = Serial.read() & 0x7f;
    else break;
  }
  if (c != 0x03) Serial.println(F(" : FAILED"));
  else Serial.println(F(" : OK"));
}

//------------------------------------------
void WebTeleInfo::InitWebServer(AsyncWebServer &server) {

  server.on("/gs1", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetStatus());
  });


  server.on("/getLabel", HTTP_GET, [this](AsyncWebServerRequest * request) {
    if (!request->hasParam(F("name"))) {
      request->send(400, F("text/html"), F("Missing parameter"));
      return;
    }
    if (request->getParam(F("name"))->value().length() == 0) {
      request->send(400, F("text/html"), F("Incorrect label name"));
      return;
    }
    String labelResponse = GetLabel(request->getParam(F("name"))->value());
    if (!labelResponse.length()) request->send(500, F("text/html"), F("Label cannot be found"));
    else request->send(200, F("text/json"), labelResponse);
  });

  server.on("/getAllLabel", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetAllLabel());
  });
}

//------------------------------------------
//Run for timer
void WebTeleInfo::Run() {

  if (Serial.available()) _tinfo.process(Serial.read() & 0x7f);
}
