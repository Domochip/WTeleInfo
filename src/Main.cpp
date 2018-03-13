#include <ESP8266WiFi.h>
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include "Version.h"
#include "WifiMan.h"
#include "Config.h"
#include "..\Main.h"

#include "data\pure-min.css.gz.h"
#include "data\side-menu.css.gz.h"
#include "data\side-menu.js.gz.h"
#include "data\jquery-3.2.1.min.js.gz.h"

//Config object
Config config;

//WifiMan
WifiMan wifiMan;

//AsyncWebServer
AsyncWebServer server(80);
//flag to use from web update to reboot the ESP
bool shouldReboot = false;

//Application object
#if defined(APPLICATION_CLASS) && defined(APPLICATION_VAR)
  APPLICATION_CLASS APPLICATION_VAR;
#endif


//-----------------------------------------------------------------------
void InitSystemWebServer(AsyncWebServer &server) {

  //root is status
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t*)statushtmlgz, sizeof(statushtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //sn url is a way to find module on network
  char discoURL[10];
  sprintf_P(discoURL, PSTR("/%08x"), ESP.getChipId());
  server.on(discoURL, HTTP_GET, [discoURL](AsyncWebServerRequest * request) {
    char chipID[9];
    sprintf_P(chipID, PSTR("%08x"), ESP.getChipId());
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", chipID);
    response->addHeader("Access-Control-Allow-Origin", "*"); //allow this URL to be requested from everywhere
    request->send(response);
  });

  //ffffffff url is a way to find all modules on the network
  server.on("/ffffffff", HTTP_GET, [](AsyncWebServerRequest * request) {
    //answer with a JSON string containing sn, model and version numbers
    char discoJSON[128];
    sprintf_P(discoJSON, PSTR("{\"sn\":\"%08x\",\"m\":\"%s\",\"v\":\"%s\"}"), ESP.getChipId(), MODEL, BASE_VERSION "/" VERSION);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/json", discoJSON);
    response->addHeader("Access-Control-Allow-Origin", "*"); //allow this URL to be requested from everywhere
    request->send(response);
  });

  //Special Discover page (not listed in default menu
  server.on("/discover", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t*)discoverhtmlgz, sizeof(discoverhtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //GetStatus0 is returning system status
  server.on("/gs0", HTTP_GET, [](AsyncWebServerRequest * request) {

    unsigned long minutes = millis() / 60000;
    char ssJSON[150];
    snprintf_P(ssJSON, sizeof(ssJSON), PSTR("{\"sn\":\"%08x\",\"b\":\"%s\",\"u\":\"%dd%dh%dm\""), ESP.getChipId(), BASE_VERSION "/" VERSION, (byte)(minutes / 1440), (byte)(minutes / 60 % 24), (byte)(minutes % 60));
    snprintf_P(ssJSON + strlen(ssJSON), sizeof(ssJSON) - strlen(ssJSON), PSTR(",\"ap\":\"%s\",\"ai\":\"%s\""), ((WiFi.getMode()&WIFI_AP) ? "on" : "off"), ((WiFi.getMode()&WIFI_AP) ? WiFi.softAPIP().toString().c_str() : "-"));
    snprintf_P(ssJSON + strlen(ssJSON), sizeof(ssJSON) - strlen(ssJSON), PSTR(",\"sta\":\"%s\",\"stai\":\"%s\""), (config.systemData.ssid[0] ? "on" : "off"), (config.systemData.ssid[0] ? (WiFi.isConnected() ? ((WiFi.localIP().toString() + ' ' + (config.systemData.ip ? "Static" : "DHCP")).c_str()) : "Not Connected") : "-"));
    snprintf_P(ssJSON + strlen(ssJSON), sizeof(ssJSON) - strlen(ssJSON), PSTR(",\"f\":%d}"), ESP.getFreeHeap());
    request->send(200, F("text/json"), ssJSON);
  });

  //FirmWare URL to get page
  server.on("/fw", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t*)fwhtmlgz, sizeof(fwhtmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //FirmWare POST URL allows to push new firmware
  server.on("/fw", HTTP_POST, [](AsyncWebServerRequest * request) {
    shouldReboot = !Update.hasError();
    if (shouldReboot) {
      AsyncWebServerResponse *response = request->beginResponse(200, F("text/html"), F("Firmware Successfully Uploaded<script>setTimeout(function(){if('referrer' in document)window.location=document.referrer;},10000);</script>"));
      response->addHeader("Connection", "close");
      request->send(response);
    }
    else {
      AsyncWebServerResponse *response = request->beginResponse(500, F("text/html"), F("Firmware Update Error : End failed"));
      response->addHeader("Connection", "close");
      request->send(response);
    }
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Serial.printf("Update Start: %s\n", filename.c_str());
#ifdef STATUS_LED_OFF
      STATUS_LED_OFF
#endif
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
    }
  });

  //Ressources URLs
  server.on("/pure-min.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), (const uint8_t*)puremincssgz, sizeof(puremincssgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/side-menu.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), (const uint8_t*)sidemenucssgz, sizeof(sidemenucssgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/side-menu.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/javascript"), (const uint8_t*)sidemenujsgz, sizeof(sidemenujsgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  server.on("/jquery-3.2.1.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/javascript"), (const uint8_t*)jquery321minjsgz, sizeof(jquery321minjsgz));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=604800, public");
    request->send(response);
  });

  //Special Developper pages
#if DEVELOPPER_MODE
  server.addHandler(new SPIFFSEditor("TODO", "TODO"));
#endif

  //404 on not found
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
}

//-----------------------------------------------------------------------
// Setup function
//-----------------------------------------------------------------------
void setup() {

  Serial.begin(SERIAL_SPEED);
  Serial.println();
  Serial.println();
  delay(200);

#ifdef STATUS_LED_SETUP
  STATUS_LED_SETUP
#endif
#ifdef STATUS_LED_ERROR
  STATUS_LED_ERROR
#endif

  Serial.print(F(APPLICATION_NAME " ")); Serial.println(BASE_VERSION "/" VERSION);
  Serial.println(F("---Booting---"));
  Serial.println(F("Wait Rescue button for 5 seconds"));

  bool skipExistingConfig = false;
  pinMode(RESCUE_BTN_PIN, (RESCUE_BTN_PIN != 16) ? INPUT_PULLUP : INPUT);
  for (int i = 0; i < 100 && skipExistingConfig == false; i++) {
    if (digitalRead(RESCUE_BTN_PIN) == LOW) skipExistingConfig = true;
    delay(50);
  }

  Serial.print(F("Start Config"));

  //initialize config with default values
  config.SetDefaultValues();

  //if skipExistingConfig is false then load the existing config
  if (!skipExistingConfig) {
    if (!config.Load()) Serial.println(F(" : Failed to load config!!!---------"));
    else Serial.println(F(" : OK"));
  }
  else Serial.println(F(" : OK (Config Skipped)"));

  Serial.print(F("Start WiFi : "));
  wifiMan.ConfigIP(config.systemData.ip, config.systemData.gw, config.systemData.mask, config.systemData.dns1, config.systemData.dns2);
  if (wifiMan.Init(config.systemData.ssid, config.systemData.password, config.systemData.hostname, DEFAULT_AP_SSID, DEFAULT_AP_PSK)) Serial.println(F("OK"));
  else Serial.println(F("FAILED"));

  //Init Application
  #if defined(APPLICATION_CLASS) && defined(APPLICATION_VAR)
    APPLICATION_VAR.Init(config.appData1);
  #endif
  
  Serial.print(F("Start WebServer"));
  InitSystemWebServer(server);
  config.InitWebServer(server, shouldReboot);
  #if defined(APPLICATION_CLASS) && defined(APPLICATION_VAR)
    APPLICATION_VAR.InitWebServer(server);
  #endif
  server.begin();
  Serial.println(F(" : OK"));

  Serial.println(F("---End of setup()---"));

}

//-----------------------------------------------------------------------
// Main Loop function
//-----------------------------------------------------------------------
void loop(void) {

#if defined(APPLICATION_CLASS) && defined(APPLICATION_VAR)
  APPLICATION_VAR.Run();
#endif

  wifiMan.Run();

  if (shouldReboot) {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  yield();
}
