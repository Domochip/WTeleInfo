#include <ESP8266WiFi.h>
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Version.h"
#include "Core.h"
#include "WifiMan.h"
#include "Base.h"
#include "..\Main.h"

//System
CoreApplication coreApplication('0', "Core");

//WifiMan
WifiMan wifiMan('w', "WiFi");

//AsyncWebServer
AsyncWebServer server(80);
//flag to pause application Run during Firmware Update
bool pauseApplication = false;
//variable used by objects to indicate system reboot is required
bool shouldReboot = false;

//Application1 object
#ifdef APPLICATION1_CLASS
APPLICATION1_CLASS application1('1', APPLICATION1_NAME);
#endif

//-----------------------------------------------------------------------
// Setup function
//-----------------------------------------------------------------------
void setup()
{

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

  Serial.print(F(APPLICATION1_DESC " "));
  Serial.println(BASE_VERSION "/" VERSION);
  Serial.println(F("---Booting---"));
  Serial.println(F("Wait Rescue button for 5 seconds"));

  bool skipExistingConfig = false;
  pinMode(RESCUE_BTN_PIN, (RESCUE_BTN_PIN != 16) ? INPUT_PULLUP : INPUT);
  for (int i = 0; i < 100 && skipExistingConfig == false; i++)
  {
    if (digitalRead(RESCUE_BTN_PIN) == LOW)
      skipExistingConfig = true;
    delay(50);
  }

  if (skipExistingConfig)
    Serial.println(F("-> RESCUE MODE : Stored configuration won't be loaded."));

  if (!SPIFFS.begin())
  {
    Serial.println(F("/!\\   File System Mount Failed   /!\\"));
    Serial.println(F("/!\\ Configuration can't be saved /!\\"));
  }

  //Init Core
  coreApplication.Init(skipExistingConfig);

  //Init WiFi
  wifiMan.Init(skipExistingConfig);

//Init Application
#ifdef APPLICATION1_CLASS
  application1.Init(skipExistingConfig);
#endif

  Serial.print(F("Start WebServer : "));

  coreApplication.InitWebServer(server, shouldReboot, pauseApplication);
  wifiMan.InitWebServer(server, shouldReboot, pauseApplication);
#ifdef APPLICATION1_CLASS
  application1.InitWebServer(server, shouldReboot, pauseApplication);
#endif
  server.begin();
  Serial.println(F("OK"));

  Serial.println(F("---End of setup()---"));
}

//-----------------------------------------------------------------------
// Main Loop function
//-----------------------------------------------------------------------
void loop(void)
{

#ifdef APPLICATION1_CLASS
  if (!pauseApplication)
    application1.Run();
#endif

  wifiMan.Run();

  if (shouldReboot)
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  yield();
}
