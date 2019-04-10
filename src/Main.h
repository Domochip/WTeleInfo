#ifndef Main_h
#define Main_h

#include <arduino.h>

//DomoChip Informations
//------------Compile for 1M 64K SPIFFS------------
//Configuration Web Pages :
//http://IP/
//http://IP/config
//http://IP/fw
//TeleInfo Request Web Pages
//http://IP/getAllLabel
//http://IP/getLabel?name=HCHC

//include Application header file
#include "WirelessTeleInfo.h"

#define APPLICATION1_NAME "WTeleInfo"
#define APPLICATION1_DESC "DomoChip Wireless TeleInfo"
#define APPLICATION1_CLASS WebTeleInfo

#define VERSION_NUMBER "3.3.1"

#define DEFAULT_AP_SSID "WirelessTeleInfo"
#define DEFAULT_AP_PSK "PasswordTeleInfo"

//Enable developper mode (SPIFFS editor)
#define DEVELOPPER_MODE 0

//Choose Serial Speed
#define SERIAL_SPEED 1200

//Choose Pin used to boot in Rescue Mode
//#define RESCUE_BTN_PIN 2

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


