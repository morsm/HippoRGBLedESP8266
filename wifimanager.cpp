#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setupWifiManager()
{
    WiFiManager wifiManager;

    // Callback to print address to serial terminal if autoconnect doesn't work
    wifiManager.setAPCallback(configModeCallback);

    // Three-minute timeout and then try to restart
    wifiManager.setConfigPortalTimeout(180);

    // Create standard access point before configuration
    wifiManager.autoConnect("Hippotronics");
}

void resetWifi()
{
   WiFiManager wifiManager;

   wifiManager.resetSettings();
}

bool isWifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}
