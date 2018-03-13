#include <Arduino.h>
#include <EEPROM.h>

#include "Config.h"

uint16_t crc16(const uint8_t* data_p, uint16_t length) {
  uint8_t x;
  uint16_t crc = 0xFFFF;

  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
  }
  return crc;
}


String SystemData::GetJSON() {

  String gc = F("\"s\":\"");
  //there is a predefined special password (mean to keep already saved one)
  gc = gc + ssid + F("\",\"p\":\"") + (__FlashStringHelper*)predefPassword + F("\",\"h\":\"") + hostname + '"';
  gc = gc + F(",\"staticip\":\"") + (ip ? F("on") : F("off")) + '"';
  if (ip) gc = gc + F(",\"ip\":\"") + IPAddress(ip).toString() + '"';
  gc = gc + F(",\"gw\":\"") + IPAddress(gw).toString() + '"';
  gc = gc + F(",\"mask\":\"") + IPAddress(mask).toString() + '"';
  if (dns1) gc = gc + F(",\"dns1\":\"") + IPAddress(dns1).toString() + '"';
  if (dns2) gc = gc + F(",\"dns2\":\"") + IPAddress(dns2).toString() + '"';

  return gc;
}

bool SystemData::SetFromParameters(AsyncWebServerRequest* request, SystemData &tempSystemData) {

  //basic control
  if (!request->hasParam(F("s"), true)) {
    request->send(400, F("text/html"), F("SSID missing"));
    return false;
  }

  if (request->hasParam(F("s"), true) && request->getParam(F("s"), true)->value().length() < sizeof(tempSystemData.ssid)) strcpy(tempSystemData.ssid, request->getParam(F("s"), true)->value().c_str());

  if (request->hasParam(F("p"), true) && request->getParam(F("p"), true)->value().length() < sizeof(tempSystemData.password)) strcpy(tempSystemData.password, request->getParam(F("p"), true)->value().c_str());
  if (request->hasParam(F("h"), true) && request->getParam(F("h"), true)->value().length() < sizeof(tempSystemData.hostname)) strcpy(tempSystemData.hostname, request->getParam(F("h"), true)->value().c_str());

  IPAddress ipParser;
  if (request->hasParam(F("ip"), true) && ipParser.fromString(request->getParam(F("ip"), true)->value())) tempSystemData.ip = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("gw"), true) && ipParser.fromString(request->getParam(F("gw"), true)->value())) tempSystemData.gw = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("mask"), true) && ipParser.fromString(request->getParam(F("mask"), true)->value())) tempSystemData.mask = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("dns1"), true) && ipParser.fromString(request->getParam(F("dns1"), true)->value())) tempSystemData.dns1 = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("dns2"), true) && ipParser.fromString(request->getParam(F("dns2"), true)->value())) tempSystemData.dns2 = static_cast<uint32_t>(ipParser);

  //check for previous password ssid (there is a predefined special password that mean to keep already saved one)
  if (!strcmp_P(tempSystemData.password, predefPassword)) strcpy(tempSystemData.password, password);

  return true;
}







bool Config::Save() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  // Init pointer
  uint8_t * p = (uint8_t *) this ;

  // Init CRC
  this->crc = crc16(p, (uint8_t*)&this->crc - (uint8_t*)this);

  //For each byte of Config object
  for (uint16_t i = 0; i < sizeof(Config); ++i) EEPROM.write(i, *(p + i));

#ifdef ESP8266
  EEPROM.end();
#endif

  return Load();
}


bool Config::Load() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  //tmpConfig will be used to load EEPROM datas
  Config tmpConfig;

  //create pointer tmpConfig
  uint8_t * p = (uint8_t *) &tmpConfig ;

  // For size of Config, read bytes
  for (uint16_t i = 0; i < sizeof(Config); ++i) *(p + i) = EEPROM.read(i);

#ifdef ESP8266
  EEPROM.end();
#endif


  // Check CRC
  if (crc16(p, (uint8_t*)&tmpConfig.crc - (uint8_t*)&tmpConfig) == tmpConfig.crc) {
    *this = tmpConfig;
    return true;
  }

  return false;
}

void Config::InitWebServer(AsyncWebServer &server, bool &shouldReboot) {

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t*)confightmlgz, sizeof(confightmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/gc", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetJSON());
  });

  server.on("/sc", HTTP_POST, [this, &shouldReboot](AsyncWebServerRequest * request) {
    shouldReboot = SetFromParameters(request);
  });

  server.on("/wnl", HTTP_GET, [this](AsyncWebServerRequest * request) {

    int8_t n = WiFi.scanComplete();
    if (n == -2) {
      request->send(200, F("text/json"), F("{\"r\":-2,\"wnl\":[]}"));
      WiFi.scanNetworks(true);
    }
    else if (n == -1) {
      request->send(200, F("text/json"), F("{\"r\":-1,\"wnl\":[]}"));
    }
    else {
      String networksJSON(F("{\"r\":"));
      networksJSON = networksJSON + n + F(",\"wnl\":[");
      for (byte i = 0; i < n; i++) {
        networksJSON = networksJSON + '"' + WiFi.SSID(i) + '"';
        if (i != (n - 1)) networksJSON += ',';
      }
      networksJSON += F("]}");
      request->send(200, F("text/json"), networksJSON);
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) WiFi.scanNetworks(true);
    }
  });
}

String Config::GetJSON() {

  String gc('{');
  gc = gc + systemData.GetJSON();
  gc = gc + ',' + appData1.GetJSON();
  gc += '}';

  return gc;
}

bool Config::SetFromParameters(AsyncWebServerRequest* request) {

  //temp config
  Config tempConfig;

  if(!systemData.SetFromParameters(request, tempConfig.systemData)) return false;
  if(!appData1.SetFromParameters(request, tempConfig.appData1)) return false;


  //then save
  bool result = tempConfig.Save();

  //Send client answer
  if (result) request->send(200);
  else request->send(500, F("text/html"), F("Configuration hasn't been saved"));

  return result;
}
