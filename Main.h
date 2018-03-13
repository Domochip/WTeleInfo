#ifndef Main_h
#define Main_h

#include "data\status.html.gz.h"
#include "data\config.html.gz.h"
#include "data\fw.html.gz.h"
#include "data\discover.html.gz.h"

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

#define APPLICATION_NAME "DomoChip Wireless TeleInfo"
#define APPLICATION_CLASS WebTeleInfo
#define APPLICATION_VAR webTeleInfo

#define VERSION_NUMBER "3.1.1"

#define MODEL "WTeleInfo"

#define DEFAULT_AP_SSID "WirelessTeleInfo"
#define DEFAULT_AP_PSK "PasswordTeleInfo"

//Enable developper mode (fwdev webpage and SPIFFS is used)
#define DEVELOPPER_MODE 0

//Pin RX used to receive TéléInformation signal @ 1200bauds
#define SERIAL_SPEED 1200

//FOR ESP-01 : there is only one serial and RX pin should be up at start, so TeleInfo signal should be controlled by a pin
//(I cabled GPIO0 instead of GND to the Mosfet with a pull-up resistor, that way, TeleInfo signal comes only if GPIO0 go to GND)
#define TELEINFO_CONTROL_PIN 0

//Choose Pin used to boot in Rescue Mode
#define RESCUE_BTN_PIN 2

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


