#ifndef Base_h
#define Base_h

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class Application
{
protected:

  typedef enum{
    status,
    config,
    fw,
    discover
  } WebPageForPlaceHolder;

  char _appId;
  String _appName;
  bool _reInit = false;

  //already built methods
  bool SaveConfig();
  bool LoadConfig();

  //specialization required from the application
  virtual void SetConfigDefaultValues() = 0;
  virtual void ParseConfigJSON(JsonObject &root) = 0;
  virtual bool ParseConfigWebRequest(AsyncWebServerRequest *request) = 0;
  virtual String GenerateConfigJSON(bool forSaveFile = false) = 0;
  virtual String GenerateStatusJSON() = 0;
  virtual bool AppInit(bool reInit = false) = 0;
  virtual const uint8_t* GetHTMLContent(WebPageForPlaceHolder wp) = 0;
  virtual size_t GetHTMLContentSize(WebPageForPlaceHolder wp) = 0;
  virtual void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication) = 0;
  virtual void AppRun() = 0;

public:
  //already built methods
  Application(char appId, String appName)
  {
    _appId = appId;
    _appName = appName;
  }
  void Init(bool skipExistingConfig);
  void InitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void Run();
};

#endif