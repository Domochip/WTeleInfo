#include "WifiMan.h"

void WifiMan::SetConfigDefaultValues()
{
  ssid[0] = 0;
  password[0] = 0;
  hostname[0] = 0;
  ip = 0;
  gw = 0;
  mask = 0;
  dns1 = 0;
  dns2 = 0;
}

void WifiMan::ParseConfigJSON(DynamicJsonDocument &doc)
{
  if (!doc["s"].isNull())
    strlcpy(ssid, doc["s"], sizeof(ssid));

  if (!doc["p"].isNull())
    strlcpy(password, doc["p"], sizeof(password));

  if (!doc["h"].isNull())
    strlcpy(hostname, doc["h"], sizeof(hostname));

  if (!doc["ip"].isNull())
    ip = doc["ip"];
  if (!doc["gw"].isNull())
    gw = doc["gw"];
  if (!doc["mask"].isNull())
    mask = doc["mask"];
  if (!doc["dns1"].isNull())
    dns1 = doc["dns1"];
  if (!doc["dns2"].isNull())
    dns2 = doc["dns2"];
}

bool WifiMan::ParseConfigWebRequest(AsyncWebServerRequest *request)
{

  //basic control
  if (!request->hasParam(F("s"), true))
  {
    request->send(400, F("text/html"), F("SSID missing"));
    return false;
  }

  char tempPassword[64 + 1] = {0};

  if (request->hasParam(F("s"), true) && request->getParam(F("s"), true)->value().length() < sizeof(ssid))
    strcpy(ssid, request->getParam(F("s"), true)->value().c_str());

  if (request->hasParam(F("p"), true) && request->getParam(F("p"), true)->value().length() < sizeof(tempPassword))
    strcpy(tempPassword, request->getParam(F("p"), true)->value().c_str());
  if (request->hasParam(F("h"), true) && request->getParam(F("h"), true)->value().length() < sizeof(hostname))
    strcpy(hostname, request->getParam(F("h"), true)->value().c_str());

  IPAddress ipParser;
  if (request->hasParam(F("ip"), true))
  {
    if (ipParser.fromString(request->getParam(F("ip"), true)->value()))
      ip = static_cast<uint32_t>(ipParser);
    else
      ip = 0;
  }
  if (request->hasParam(F("gw"), true))
  {
    if (ipParser.fromString(request->getParam(F("gw"), true)->value()))
      gw = static_cast<uint32_t>(ipParser);
    else
      gw = 0;
  }
  if (request->hasParam(F("mask"), true))
  {
    if (ipParser.fromString(request->getParam(F("mask"), true)->value()))
      mask = static_cast<uint32_t>(ipParser);
    else
      mask = 0;
  }
  if (request->hasParam(F("dns1"), true))
  {
    if (ipParser.fromString(request->getParam(F("dns1"), true)->value()))
      dns1 = static_cast<uint32_t>(ipParser);
    else
      dns1 = 0;
  }
  if (request->hasParam(F("dns2"), true))
  {
    if (ipParser.fromString(request->getParam(F("dns2"), true)->value()))
      dns2 = static_cast<uint32_t>(ipParser);
    else
      dns2 = 0;
  }

  //check for previous password ssid (there is a predefined special password that mean to keep already saved one)
  if (strcmp_P(tempPassword, predefPassword))
    strcpy(password, tempPassword);

  return true;
}
String WifiMan::GenerateConfigJSON(bool forSaveFile = false)
{

  String gc('{');
  gc = gc + F("\"s\":\"") + ssid + '"';

  if (forSaveFile)
    gc = gc + F(",\"p\":\"") + password + '"';
  else
    //there is a predefined special password (mean to keep already saved one)
    gc = gc + F(",\"p\":\"") + (__FlashStringHelper *)predefPassword + '"';
  gc = gc + F(",\"h\":\"") + hostname + '"';

  if (forSaveFile)
  {
    gc = gc + F(",\"ip\":") + ip;
    gc = gc + F(",\"gw\":") + gw;
    gc = gc + F(",\"mask\":") + mask;
    gc = gc + F(",\"dns1\":") + dns1;
    gc = gc + F(",\"dns2\":") + dns2;
  }
  else
  {
    gc = gc + F(",\"staticip\":") + (ip ? true : false);
    if (ip)
      gc = gc + F(",\"ip\":\"") + IPAddress(ip).toString() + '"';
    if (gw)
      gc = gc + F(",\"gw\":\"") + IPAddress(gw).toString() + '"';
    else
      gc = gc + F(",\"gw\":\"0.0.0.0\"");
    if (mask)
      gc = gc + F(",\"mask\":\"") + IPAddress(mask).toString() + '"';
    else
      gc = gc + F(",\"mask\":\"0.0.0.0\"");
    if (dns1)
      gc = gc + F(",\"dns1\":\"") + IPAddress(dns1).toString() + '"';
    if (dns2)
      gc = gc + F(",\"dns2\":\"") + IPAddress(dns2).toString() + '"';
  }

  gc = gc + '}';

  return gc;
}

String WifiMan::GenerateStatusJSON()
{

  String gs('{');
  if ((WiFi.getMode() & WIFI_AP))
  {
    gs = gs + F("\"ap\":\"on\"");
    gs = gs + F(",\"ai\":\"") + WiFi.softAPIP().toString().c_str() + '"';
  }
  else
  {
    gs = gs + F("\"ap\":\"off\"");
    gs = gs + F(",\"ai\":\"-\"");
  }

  gs = gs + F(",\"sta\":\"") + (ssid[0] ? F("on") : F("off")) + '"';
  gs = gs + F(",\"stai\":\"") + (ssid[0] ? (WiFi.isConnected() ? ((WiFi.localIP().toString() + ' ' + (ip ? F("Static IP") : F("DHCP"))).c_str()) : "Not Connected") : "-") + '"';

  gs = gs + F(",\"mac\":\"") + WiFi.macAddress() + '"';

  gs = gs + '}';

  return gs;
}

bool WifiMan::AppInit(bool reInit = false)
{
  bool result = false;

  if (!reInit)
  {
    //build "unique" AP name (DEFAULT_AP_SSID + 4 last digit of ChipId)
    _apSsid[0] = 0;
    strcpy(_apSsid, DEFAULT_AP_SSID);
    uint16 id = ESP.getChipId() & 0xFFFF;
    byte endOfSsid = strlen(_apSsid);
    byte num = (id & 0xF000) / 0x1000;
    _apSsid[endOfSsid] = num + ((num <= 9) ? 0x30 : 0x37);
    num = (id & 0xF00) / 0x100;
    _apSsid[endOfSsid + 1] = num + ((num <= 9) ? 0x30 : 0x37);
    num = (id & 0xF0) / 0x10;
    _apSsid[endOfSsid + 2] = num + ((num <= 9) ? 0x30 : 0x37);
    num = id & 0xF;
    _apSsid[endOfSsid + 3] = num + ((num <= 9) ? 0x30 : 0x37);
    _apSsid[endOfSsid + 4] = 0;
  }
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.print(F("N-CH"));
  if (n)
  {
    while (_apChannel < 12)
    {
      int i = 0;
      while (i < n && WiFi.channel(i) != _apChannel)
        i++;
      if (i == n)
        break;
      _apChannel++;
    }
  }
  Serial.print(_apChannel);

  //make next changes saved to flash
  WiFi.persistent(true);

  //if STA is requested
  if (ssid[0])
  {

    //Set hostname
    WiFi.hostname(hostname);

    //if not connected or config changed then reconnect
    if (!WiFi.isConnected() || WiFi.SSID() != ssid || WiFi.psk() != password)
    {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    WiFi.config(ip, gw, mask, dns1, dns2);

    //right config so no need to touch again flash
    WiFi.persistent(false);

    //Wait 20sec for connection
    for (int i = 0; i < 200 && !WiFi.isConnected(); i++)
    {
      if ((i % 10) == 0)
        Serial.print(".");
      delay(100);
    }
    if (WiFi.isConnected())
    {
      if (!reInit) //in case of reinit, _wifiHandler1 already do this job
      {
        Serial.print('(');
        Serial.print(WiFi.localIP());
        Serial.print(F(") "));
        WiFi.enableAP(false);
      };
      result = true;
#ifdef STATUS_LED_GOOD
      STATUS_LED_GOOD
#endif
    }
    else
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(_apSsid, DEFAULT_AP_PSK, _apChannel);
      Serial.print(F("Enabling AP ("));
      Serial.print(WiFi.softAPIP());
      Serial.print(')');
      _retryStation = true;
#ifdef STATUS_LED_WARNING
      STATUS_LED_WARNING
#endif
    }

    //Configure handlers
    if (!reInit)
    {
      _wifiHandler1 = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected &evt) {
        if (!(WiFi.getMode() & WIFI_AP) && ssid[0])
        {
          WiFi.mode(WIFI_AP);
          WiFi.softAPdisconnect();
          WiFi.softAP(_apSsid, DEFAULT_AP_PSK, _apChannel);
          Serial.print(F("WiFiDisco : Enabling AP ("));
          Serial.print(WiFi.softAPIP());
          Serial.println(')');
          _retryStation = true;
        }
#ifdef STATUS_LED_WARNING
        STATUS_LED_WARNING
#endif
      });

      _wifiHandler2 = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &evt) {
        if (WiFi.getMode() & WIFI_AP)
          WiFi.enableAP(false);
        Serial.print(F("WiFiReco : ("));
        Serial.print(WiFi.localIP());
        Serial.print(F(") "));
#ifdef STATUS_LED_GOOD
        STATUS_LED_GOOD
#endif
      });
    }
  }
  else
  {
    _retryStation = false;
    WiFi.disconnect();
    //Enable AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apSsid, DEFAULT_AP_PSK, _apChannel);
    Serial.print(F(" AP mode("));
    Serial.print(WiFi.softAPIP());
    Serial.print(F(") "));
    WiFi.persistent(false);
    result = true;
#ifdef STATUS_LED_GOOD
    STATUS_LED_GOOD
#endif
  }

  return result;
};

const uint8_t *WifiMan::GetHTMLContent(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return (const uint8_t *)statuswhtmlgz;
    break;
  case config:
    return (const uint8_t *)configwhtmlgz;
    break;
  default:
    return nullptr;
    break;
  };
  return nullptr;
};
size_t WifiMan::GetHTMLContentSize(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return sizeof(statuswhtmlgz);
    break;
  case config:
    return sizeof(configwhtmlgz);
    break;
  default:
    return 0;
    break;
  };
  return 0;
};

void WifiMan::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{

  server.on("/wnl", HTTP_GET, [this](AsyncWebServerRequest *request) {
    int8_t n = WiFi.scanComplete();
    if (n == -2)
    {
      request->send(200, F("text/json"), F("{\"r\":-2,\"wnl\":[]}"));
      WiFi.scanNetworks(true);
    }
    else if (n == -1)
    {
      request->send(200, F("text/json"), F("{\"r\":-1,\"wnl\":[]}"));
    }
    else
    {
      String networksJSON(F("{\"r\":"));
      networksJSON = networksJSON + n + F(",\"wnl\":[");
      for (byte i = 0; i < n; i++)
      {
        networksJSON = networksJSON + '"' + WiFi.SSID(i) + '"';
        if (i != (n - 1))
          networksJSON += ',';
      }
      networksJSON += F("]}");
      request->send(200, F("text/json"), networksJSON);
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2)
        WiFi.scanNetworks(true);
    }
  });
}

void WifiMan::AppRun()
{

  if (_retryStation && (millis() / (_retryPeriod * 100) % 10 == 0))
  {
    Serial.print(F("Try WiFiReco"));
    //WiFi.begin(config.ssid, config.password); //ssid and password still stored because no WiFi.disconnect called
    WiFi.begin();
    WiFi.config(ip, gw, mask, dns1, dns2);

    //Wait 10sec for connection
    for (int i = 0; i < 100 && !WiFi.isConnected(); i++)
    {
      if ((i % 10) == 0)
        Serial.print(".");
      delay(100);
    }

    //if not connected
    if (!WiFi.isConnected())
    {
      Serial.println(F("Failed"));
      //disable station mode
      WiFi.mode(WIFI_AP);
    }
    // disable retry and AP mode is disabled by wifiHandler(2)
    else
    {
      Serial.println();
      _retryStation = false;
    }
  }
};