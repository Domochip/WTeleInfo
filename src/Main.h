#ifndef Main_h
#define Main_h

#include <arduino.h>

//DomoChip Informations
//Configuration Web Pages :
//http://IP/
//TeleInfo Request Web Pages
//http://IP/getAllLabel
//http://IP/getLabel?name=HCHC

#define APPLICATION1_HEADER "WirelessTeleInfo.h"
#define APPLICATION1_NAME "WTeleInfo"
#define APPLICATION1_DESC "DomoChip Wireless TeleInfo"
#define APPLICATION1_CLASS WebTeleInfo

#define VERSION_NUMBER "3.4.1"

#define DEFAULT_AP_SSID "WirelessTeleInfo"
#define DEFAULT_AP_PSK "PasswordTeleInfo"

//Enable status webpage EventSource
#define ENABLE_STATUS_EVENTSOURCE 1

//Enable developper mode (SPIFFS editor)
#define DEVELOPPER_MODE 0

//Log Serial Object
#define LOG_SERIAL Serial
//Choose Log Serial Speed
#define LOG_SERIAL_SPEED 1200

//Choose Pin used to boot in Rescue Mode
//#define RESCUE_BTN_PIN 2

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


