#include <ESP8266WiFi.h>

#include "sonoff.h"
#include "hippoled.h"
#include "udp.h"


void readSonoffButtons(int *dest)
{
  dest[0] = digitalRead(SONOFF_BUTTON_1);
  dest[1] = digitalRead(SONOFF_BUTTON_2);
  dest[2] = digitalRead(SONOFF_BUTTON_3);
}

void sonoffButtonPush(int but)
{
  SERIAL.println("Sonoff Button " + String(but) + " pressed");
  String butname = lampName + String("_button_") + but;
  char *szLampName = (char *) butname.c_str();

  switch (SonoffButBehavior[but])
  {
    case BUTTON_RELAY:
      toggle();
      break;

    case BUTTON_UDP:
      // TODO: remove hardcoded address
      broadcastUdpPacket(11000, szLampName);
      //sendUdpPacket(IPAddress(192,168,1,10), 11000, szLampName);
      break;

    case BUTTON_DONOTHING:
    default:
      break;
  }
}

void sonoffButtonChange(int but, int state)
{
  // Store new state
  SonoffButton[but] = state;

  if (state == LOW) sonoffButtonPush(but);
}


void sonoffLoop()
{
  // Turn on the WiFi LED if Wifi is connected
  // It's active low
  digitalWrite(SONOFF_LED, isWifiConnected() ? LOW : HIGH);
  
  // Read sonoff buttons
  int newState[3];
  readSonoffButtons(newState);

  if (SonoffButton[0] != newState[0]) sonoffButtonChange(0, newState[0]);
  if (SonoffButton[1] != newState[1]) sonoffButtonChange(1, newState[1]);
  if (SonoffButton[2] != newState[2]) sonoffButtonChange(2, newState[2]);
}


void initSonoffButtonData()
{
  for (int i=0; i<3; i++)
  {
    SonoffButton[i] = HIGH;  // unpressed
    SonoffButBehavior[i] = BUTTON_DONOTHING;  
  }
}
