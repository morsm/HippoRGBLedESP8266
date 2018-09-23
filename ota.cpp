#include "hippoled.h"

#include <ArduinoOTA.h>
#include <FS.h>


void setupOTA()
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    bool unmountSPIFFS = false;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
      unmountSPIFFS = true;
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    if (unmountSPIFFS) SPIFFS.end();
    SERIAL.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    SERIAL.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    SERIAL.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      SERIAL.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      SERIAL.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      SERIAL.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      SERIAL.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      SERIAL.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void handleOTA()
{
  ArduinoOTA.handle();
}

