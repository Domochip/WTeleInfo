#ifndef Core_h
#define Core_h

#include "..\Main.h"

class CoreApplication : public Application
{
private:
  void SetConfigDefaultValues();
  void ParseConfigJSON(JsonObject &root);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool clearPassword);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun(){};

public:
  CoreApplication(char appId, String fileName) : Application(appId, fileName) {}
};

#endif