#include "Base.h"

bool Application::SaveConfig()
{
  File configFile = SPIFFS.open(String('/') + _appName + F(".json"), "w");
  if (!configFile)
  {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  configFile.print(GenerateConfigJSON(true));
  configFile.close();
  return true;
}

bool Application::LoadConfig()
{
  //special exception for Core, there is no Core.json file to Load
  if (_appId == '0')
    return true;

  bool result = false;
  File configFile = SPIFFS.open(String('/') + _appName + F(".json"), "r");
  if (configFile)
  {
    DynamicJsonBuffer jsonBuffer(512);

    JsonObject &root = jsonBuffer.parseObject(configFile);
    if (root.success())
    {

      ParseConfigJSON(root);
      result = true;
    }

    configFile.close();
  }

  //if loading failed, then run a Save to write a good one
  if (!result)
    SaveConfig();
    
  return result;
}

void Application::Init(bool skipExistingConfig)
{
  bool result = true;

  Serial.print(F("Start "));
  Serial.print(_appName);
  Serial.print(F(" : "));

  SetConfigDefaultValues();

  if (!skipExistingConfig)
    result = LoadConfig();

  //Execute specific Application Init Code
  result = AppInit() && result;

  if (result)
    Serial.println(F("OK"));
  else
    Serial.println(F("FAILED"));
}

void Application::InitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  char url[16];

  //HTML Status handler
  sprintf_P(url, PSTR("/status%c.html"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), GetHTMLContent(status), GetHTMLContentSize(status));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //HTML Config handler
  sprintf_P(url, PSTR("/config%c.html"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), GetHTMLContent(config), GetHTMLContentSize(config));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //HTML fw handler
  sprintf_P(url, PSTR("/fw%c.html"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), GetHTMLContent(fw), GetHTMLContentSize(fw));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //HTML discover handler
  sprintf_P(url, PSTR("/discover%c.html"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), GetHTMLContent(discover), GetHTMLContentSize(discover));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //JSON Status handler
  sprintf_P(url, PSTR("/gs%c"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, F("text/json"), GenerateStatusJSON());
  });

  //JSON Config handler
  sprintf_P(url, PSTR("/gc%c"), _appId);
  server.on(url, HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, F("text/json"), GenerateConfigJSON());
  });

  sprintf_P(url, PSTR("/sc%c"), _appId);
  server.on(url, HTTP_POST, [this](AsyncWebServerRequest *request) {

    if (!ParseConfigWebRequest(request))
      return;

    //then save
    bool result = SaveConfig();

    //Send client answer
    if (result)
    {
      request->send(200);
      _reInit = true;
    }
    else
      request->send(500, F("text/html"), F("Configuration hasn't been saved"));
  });

  //Execute Specific Application Web Server initialization
  AppInitWebServer(server, shouldReboot, pauseApplication);
}

void Application::Run()
{
  if (_reInit)
  {
    Serial.print(F("ReStart "));
    Serial.print(_appName);
    Serial.print(F(" : "));

    if (AppInit(true))
      Serial.println(F("OK"));
    else
      Serial.println(F("FAILED"));

    _reInit = false;
  }

  AppRun();
}