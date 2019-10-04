#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

void setupWifiManager()
{
    WiFiManager wifiManager;

  // Create standard access point before configuration
  wifiManager.autoConnect("Hippotronics");

}

