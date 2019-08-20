// Hippotronics LED lamp main program

#include "hippoled.h"
#include "config.h"
#include "webrequests.h"
#include "ota.h"

#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266mDNS.h>        // Include the mDNS library
#include <FS.h>


#ifdef LOGTELNET
TelnetSpy LOG;
#endif


AsyncWebServer server(8080);
bool fsOK = false;

// Lamp state
int burning = 1, oldRed = 200, oldGreen = 200, oldBlue = 100;

// Configuration
int type = UNDEFINED;
int startAfterPowerOff = START_ON;
String lampName = "HippotronicsLED";
bool gResetFlag = false;

void setup()
{
  SERIAL.print("Hippotronics RGB_LED_Lamp ");
  SERIAL.println(VERSION);
  
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  // Generate random postfix for LED first time around
  lampName = "HippotronicsLED-" + String(random(1000, 2000));
  
  SERIAL.begin(74880);
  SERIAL.println("Hippotronics RGB LED Lamp started, initial name: " + lampName);
  SERIAL.println("Chip real flash size: " + String(ESP.getFlashChipRealSize()));

  // Initialize file system
  fsOK = SPIFFS.begin();
  if (fsOK)
  {
    SERIAL.println("File system ready");
    
    bool bConfigSuccess = load_config();
    
    bool bStateSuccess = loadLampState();
    if (bStateSuccess || bConfigSuccess)
    {
      // Switch lamp on?
      bool bSwitchOn = (START_ON == startAfterPowerOff) || (START_LAST == startAfterPowerOff && 1 == burning);
      if (bSwitchOn) setRGB(1, oldRed, oldGreen, oldBlue);
    }
  }
  else
  {
    SERIAL.println("File system not available");
  }
 
  WiFiManager wifiManager;

  // Create standard access point before configuration
  wifiManager.autoConnect("Hippotronics");

  // Register with MDNS
  String mdnsName = "HippoLed-" + lampName;
  if (!MDNS.begin(mdnsName.c_str())) 
  {             
    SERIAL.println("Error setting up MDNS responder!");
  }
  else
  {
    SERIAL.println("mDNS responder started with name " + mdnsName);

    // Add service
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("hippohttp", "tcp", 8080);
  }

  // Set up lamp based on configuration
  setupLamp();
  
  // Configure over-the-air upgrade (OTA)
  setupOTA();
  
  // Setup internal web server
  setup_web_paths();
}


void loop() 
{
  // Handle logging
#ifdef LOGTELNET
  LOG.handle();
#endif
  
  if (gResetFlag)
  {
    delay(1000);    // Reset because config changed, give time to send data
    ESP.restart();
  }

  delay(1000);

  // See if over-the-air upgrade (OTA) is available
  handleOTA();
}


void setRGB(const int burn, const int red, const int green, const int blue)
{
  burning = burn;
  
  if (red >= 0 && green >= 0 && blue >= 0)
  {
    oldRed = red;
    oldGreen = green; 
    oldBlue = blue; 
  }
  
  if (0 == burn)
  {
    switch (type)
    {
      case COLORRGB:
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
        break;

      case DIMMABLE:
        analogWrite(RED_PIN, 0);
        break;

      case COLOR1D:
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        break;

      case SWITCH:
        digitalWrite(SWITCH_PIN, LOW);
        break;
    }
  }
  else
  {
    switch (type)
    {
      case COLORRGB:
        analogWrite(RED_PIN, oldRed);
        analogWrite(GREEN_PIN, oldGreen);
        analogWrite(BLUE_PIN, oldBlue);
        break;

      case DIMMABLE:
        analogWrite(RED_PIN, oldRed);
        break;

      case COLOR1D:
        analogWrite(RED_PIN, oldRed);
        analogWrite(GREEN_PIN, oldGreen);
        break;

      case SWITCH:
        digitalWrite(SWITCH_PIN, HIGH);
        break;
    }
  }
}

void switch_off()
{
  setRGB(0, -1, -1, -1);
  saveLampState();
}

void switch_on()
{
  setRGB(1, oldRed, oldGreen, oldBlue);
  saveLampState();
}

bool isOn()
{
  return (1 == burning);
}

void set_name(String val)
{
    char hostName[54];
    
    int len = val.length();
    if (len < 1) return;
    if (len > 53) len = 53;   // max hostname length, need to add 'HippoLed-' in front of it

    val.toCharArray(hostName, 54);
    for (int i=0; i<len; i++)
    {
      // Only numbers, letters and the dash ('-') allowed
      if (isAlphaNumeric(hostName[i]) || '-' == hostName[i]) continue;

      // Illegal character, replace
      hostName[i] = '-';
    }
    
    lampName = hostName;
}

void setupLamp()
{
  switch (type)
  {
    case UNDEFINED:
      break;

    case DIMMABLE:
      // One analog output
      pinMode(RED_PIN, OUTPUT);
      break;

    case COLOR1D:
      // Two analog outputs
      pinMode(RED_PIN, OUTPUT);
      pinMode(GREEN_PIN, OUTPUT);
      break;

    case COLORRGB:
      // Three analog outputs
      pinMode(RED_PIN, OUTPUT);
      pinMode(GREEN_PIN, OUTPUT);
      pinMode(BLUE_PIN, OUTPUT);
      break;

    case SWITCH:
      // One digital output
      pinMode(SWITCH_PIN, OUTPUT);
      break;
  }

    setRGB(0,0,0,0);                // Default is off
}


