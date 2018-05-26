#include "Core.h"
#include <SPIFFSEditor.h>
#include "..\Main.h" //for VERSION define
#include "Version.h" //for BASE_VERSION define

#include "data\pure-min.css.gz.h"
#include "data\side-menu.css.gz.h"
#include "data\side-menu.js.gz.h"
#include "data\jquery-3.2.1.min.js.gz.h"

void CoreApplication::SetConfigDefaultValues(){};
void CoreApplication::ParseConfigJSON(JsonObject &root){};
bool CoreApplication::ParseConfigWebRequest(AsyncWebServerRequest *request) { return true; };
String CoreApplication::GenerateConfigJSON(bool clearPassword = false) { return String(); };
String CoreApplication::GenerateStatusJSON()
{
  String gs('{');

  char sn[9];
  snprintf_P(sn, sizeof(sn), PSTR("%08x"), ESP.getChipId());
  unsigned long minutes = millis() / 60000;

  gs = gs + F("\"sn\":\"") + sn + '"';
  gs = gs + F(",\"b\":\"") + BASE_VERSION + '/' + VERSION + '"';
  gs = gs + F(",\"u\":\"") + (byte)(minutes / 1440) + 'd' + (byte)(minutes / 60 % 24) + 'h' + (byte)(minutes % 60) + 'm' + '"';
  gs = gs + F(",\"f\":") + ESP.getFreeHeap();
  gs = gs + F(",\"fcrs\":") + ESP.getFlashChipRealSize();

  gs = gs + '}';

  return gs;
};
bool CoreApplication::AppInit(bool reInit = false) { return true; };
void CoreApplication::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  //root is status
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t *)statushtmlgz, sizeof(statushtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t *)confightmlgz, sizeof(confightmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //sn url is a way to find module on network
  char discoURL[10];
  sprintf_P(discoURL, PSTR("/%08x"), ESP.getChipId());
  server.on(discoURL, HTTP_GET, [](AsyncWebServerRequest *request) {
    char chipID[9];
    sprintf_P(chipID, PSTR("%08x"), ESP.getChipId());
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", chipID);
    response->addHeader("Access-Control-Allow-Origin", "*"); //allow this URL to be requested from everywhere
    request->send(response);
  });

  //ffffffff url is a way to find all modules on the network
  server.on("/ffffffff", HTTP_GET, [](AsyncWebServerRequest *request) {
    //answer with a JSON string containing sn, model and version numbers
    char discoJSON[128];
    sprintf_P(discoJSON, PSTR("{\"sn\":\"%08x\",\"m\":\"%s\",\"v\":\"%s\"}"), ESP.getChipId(), APPLICATION1_NAME, BASE_VERSION "/" VERSION);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/json", discoJSON);
    response->addHeader("Access-Control-Allow-Origin", "*"); //allow this URL to be requested from everywhere
    request->send(response);
  });

  //Special Discover page (not listed in default menu
  server.on("/discover", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t *)discoverhtmlgz, sizeof(discoverhtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //FirmWare URL to get page
  server.on("/fw", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t *)fwhtmlgz, sizeof(fwhtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //FirmWare POST URL allows to push new firmware
  server.on("/fw", HTTP_POST, [&shouldReboot, &pauseApplication](AsyncWebServerRequest *request) {
    shouldReboot = !Update.hasError();
    if (shouldReboot) {
      AsyncWebServerResponse *response = request->beginResponse(200, F("text/html"), F("Firmware Successfully Uploaded<script>setTimeout(function(){if('referrer' in document)window.location=document.referrer;},10000);</script>"));
      response->addHeader("Connection", "close");
      request->send(response);
    }
    else {
      //Upload failed so restart to Run Application in loop
      pauseApplication = false;
      //Prepare response
      String errorMsg(Update.getError());
      errorMsg+=' ';
      switch(Update.getError()){
        case UPDATE_ERROR_WRITE:
          errorMsg=F("Flash Write Failed");
          break;
        case UPDATE_ERROR_ERASE:
          errorMsg=F("Flash Erase Failed");
          break;
        case UPDATE_ERROR_READ:
          errorMsg=F("Flash Read Failed");
          break;
        case UPDATE_ERROR_SPACE:
          errorMsg=F("Not Enough Space");
          break;
        case UPDATE_ERROR_SIZE:
          errorMsg=F("Bad Size Given");
          break;
        case UPDATE_ERROR_STREAM:
          errorMsg=F("Stream Read Timeout");
          break;
        case UPDATE_ERROR_MD5:
          errorMsg=F("MD5 Check Failed");
          break;
        case UPDATE_ERROR_FLASH_CONFIG:
          errorMsg=F("Flash config wrong");
          break;
        case UPDATE_ERROR_NEW_FLASH_CONFIG:
          errorMsg=F("New Flash config wrong");
          break;
        case UPDATE_ERROR_MAGIC_BYTE:
          errorMsg=F("Magic byte is wrong, not 0xE9");
          break;
        case UPDATE_ERROR_BOOTSTRAP:
          errorMsg=F("Invalid bootstrapping state, reset ESP8266 before updating");
          break;
        default:
          errorMsg=F("Unknown error");
          break;
      }
      AsyncWebServerResponse *response = request->beginResponse(500, F("text/html"), errorMsg);
      response->addHeader("Connection", "close");
      request->send(response);
    } }, [&pauseApplication](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      //stop to Run Application in loop
      pauseApplication = true;
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    if (final) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    } });

  //Ressources URLs
  server.on("/pure-min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), (const uint8_t *)puremincssgz, sizeof(puremincssgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/side-menu.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), (const uint8_t *)sidemenucssgz, sizeof(sidemenucssgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/side-menu.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/javascript"), (const uint8_t *)sidemenujsgz, sizeof(sidemenujsgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/jquery-3.2.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/javascript"), (const uint8_t *)jquery321minjsgz, sizeof(jquery321minjsgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  //Special Developper pages
#if DEVELOPPER_MODE
  server.addHandler(new SPIFFSEditor("TODO", "TODO"));
#endif

  //404 on not found
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
}