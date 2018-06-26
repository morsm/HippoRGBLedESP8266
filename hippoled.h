// Header file with global variables for Hippotronics LED lamp ESP8266
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
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


#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

// Prototypes
void setRGB(const int burn, const int red, const int green, const int blue);
void switch_on();
void switch_off();
void set_name(String val);
bool isOn();

// Web server
extern AsyncWebServer server;

// File system OK?
extern bool fsOK;

// Lamp state
extern int burning, oldRed, oldGreen, oldBlue;

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

