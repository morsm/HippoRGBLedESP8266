// Header file with global variables for Hippotronics LED lamp ESP8266
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebSrv.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

#include <async_config.h>
#include <AsyncPrinter.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncTCPbuffer.h>
#include <SyncClient.h>
#include <tcp_axtls.h>

#include "TelnetSpy.h"          // Include logging via telnet

#define VERSION "3.7.1"


/// PIN DEFINITIONS

// Single relay as in original Office, Kitchen, Table with ESP8266 relay board
#define SWITCH_PIN 0

// RGB version as in BoozyLamps
#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

#define SONOFF_SWITCH_PIN 12
#define SONOFF_LED 13

/////



// Logging
#define LOGTELNET 1     // Comment out for standard serial

#ifdef LOGTELNET

extern TelnetSpy LOG;
#define SERIALPORT LOG

#else
#define SERIALPORT Serial
#endif

// New in version 3.5: lamp type
enum {
  UNDEFINED,
  DIMMABLE,
  COLOR1D,
  COLORRGB,
  SWITCH,
  SONOFF_SWITCH     // version 3.6: Sonoff R3
};

extern 
// Prototypes
void setRGB(const int burn, const int red, const int green, const int blue);
void switch_on();
void switch_off();
void toggle();
void set_name(String val);
bool isOn();
void setupLamp();
void setupWifiManager();
void resetWifi();
bool isWifiConnected();

// Web server
extern AsyncWebServer server;

// File system OK?
extern bool fsOK;

// Lamp state
extern int type, burning, oldRed, oldGreen, oldBlue;

// Configuration
enum
{
  START_OFF,      // Do not power on lamp when power comes on
  START_ON,       // Always power on lamp when power comes on (allows using regular light switch)
  START_LAST      // Power lamp or not depending on whether it was on or off when the power was cut
};

extern int startAfterPowerOff;
extern String lampName;

extern bool gResetFlag;
