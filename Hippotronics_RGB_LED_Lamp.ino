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

#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266mDNS.h>        // Include the mDNS library
#include <FS.h>


AsyncWebServer server(8080);
const int RED_PIN = 12;
const int GREEN_PIN = 13;
const int BLUE_PIN = 14;
bool fsOK = false;

// Lamp state
int burning = 1, oldRed = 200, oldGreen = 200, oldBlue = 100;
// Configuration
enum
{
  START_OFF,      // Do not power on lamp when power comes on
  START_ON,       // Always power on lamp when power comes on (allows using regular light switch)
  START_LAST      // Power lamp or not depending on whether it was on or off when the power was cut
};
int startAfterPowerOff = START_ON;
String lampName = "HippotronicsLED";
bool gResetFlag = false;

void setup()
{
  // Configure pin outputs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setRGB(0,0,0,0);                // Default is off

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  // Generate random postfix for LED first time around
  lampName = "HippotronicsLED-" + String(random(1000, 2000));
  
  Serial.begin(74880);
  Serial.println("Hippotronics RGB LED Lamp started, initial name: " + lampName);
  Serial.println("Chip real flash size: " + String(ESP.getFlashChipRealSize()));

  // Initialize file system
  fsOK = SPIFFS.begin();
  if (fsOK)
  {
    Serial.println("File system ready");
    
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
    Serial.println("File system not available");
  }
 
  WiFiManager wifiManager;

  // Create standard access point before configuration
  wifiManager.autoConnect("Hippotronics");

  // Register with MDNS
  String mdnsName = "HippoLed-" + lampName;
  if (!MDNS.begin(mdnsName.c_str())) 
  {             
    Serial.println("Error setting up MDNS responder!");
  }
  else
  {
    Serial.println("mDNS responder started with name " + mdnsName);

    // Add service
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("hippohttp", "tcp", 8080);
  }


  // If this point is reached, the unit is configured and 
  // the web server for the LED controls running on port 8080
  server.on("/", HTTP_GET, handle_index);
  server.on("/on.html", HTTP_GET, handle_switch_on);
  server.on("/off.html", HTTP_GET, handle_switch_off);
  server.on("/rgb.html", HTTP_GET, handle_rgb);

  server.on("/status.html", HTTP_GET, report_status);
  server.on("/status.json", HTTP_GET, report_status_json);
  
  server.on("/config.html", HTTP_GET, handle_config);
  server.on("/setconfig.html", HTTP_GET, handle_setconfig);

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (gResetFlag)
  {
    delay(1000);    // Reset because config changed, give time to send data
    ESP.restart();
  }

  delay(1000);

}

void handle_index(AsyncWebServerRequest *request)
{
  String response("<h1>Hippotronics LED lamp</h1>"
    "<p><a href=\"on.html\">Switch On</a></p>" 
    "<p><a href=\"off.html\">Switch Off</a></p>"
    "<p><form method=\"GET\" action=\"rgb.html\">"
    "Red <input type=\"text\" name=\"red\" value=\"");

  response += oldRed;
  response += "\"/><br/>Green <input type=\"text\" name=\"green\" value=\"";
  response += oldGreen;
  response += "\"/><br/>Blue <input type=\"text\" name=\"blue\" value=\"";
  response += oldBlue;
  response += "\"/><br/><input type=\"submit\"/></form></p>";
  response += "</br><a href=\"/config.html\">Configuration</a>";

  sendHtml(request, "Hippotronics LED lamp " + lampName, response);
}

void handle_config(AsyncWebServerRequest *request)
{
  String response("<h1>Configuration</h1>"
    "<p style=\"color: red\">Note: setting host name resets lamp unit</p>"
    "<p><form method=\"GET\" action=\"setconfig.html\">"
    "Name <input type=\"text\" name=\"name\" value=\"");

  response += lampName;
  response += "\"/><br/>Power-on behavior<br/><label for=\"b_off\">Off</label><input type=\"radio\" name=\"behavior\" id=\"b_off\" value=\"0\" ";
  response += (START_OFF == startAfterPowerOff) ? "checked" : "";
  response += "/><br/><label for=\"b_on\">On</label><input type=\"radio\" name=\"behavior\" id=\"b_on\" value=\"1\" ";
  response += (START_ON == startAfterPowerOff) ? "checked" : "";
  response += "/><br/><label for=\"b_last\">Return to last state when power off</label><input type=\"radio\" name=\"behavior\" id=\"b_last\" value=\"2\" ";
  response += (START_LAST == startAfterPowerOff) ? "checked" : "";
  response += "/><br/><input type=\"submit\"/></form></p>";
  response += "</br><a href=\"/\">Home</a>";
  
  sendHtml(request, "Hippotronics LED lamp configuration", response);
}

void handle_setconfig(AsyncWebServerRequest *request)
{
  int newBehavior = startAfterPowerOff;
  bool bReset = false;
  
  if (request->hasParam("name"))
  {
    AsyncWebParameter *p = request->getParam("name");
    String val = p->value().c_str();

    if (! val.equals(lampName))
    {
      // Set the new name
      set_name(val);

      // Reset the processor, but first redirect the client
      bReset = true;
    }
  }
  if (request->hasParam("behavior"))
  {
    AsyncWebParameter *p = request->getParam("behavior");
    String val = p->value().c_str();
    newBehavior = val.toInt();
    
    if (newBehavior >= START_OFF && newBehavior <= START_LAST) startAfterPowerOff = newBehavior;
  }

  save_config();

  if (bReset)
  {
    // Reset the unit and redirect to the home page
    String response = "<h1>Resetting</h1><p>New host name set, resetting lamp unit and returning to home page in five seconds..."
      "<script language=\"JavaScript\">setTimeout(\"location.href='/'\", 5000);</script>";

    sendHtml(request, "Resetting Hippotronics lamp", response);
    gResetFlag = true;    // Will reset in loop()
  }
  else
  {
    handle_config(request);
  }
}

void handle_switch_on(AsyncWebServerRequest *request)
{
  switch_on();
  report_status(request);
}

void handle_switch_off(AsyncWebServerRequest *request)
{
  switch_off();
  report_status(request);
}

void report_status(AsyncWebServerRequest *request)
{
  String response = "Lamp is "; 
  if (isOn()) response += "ON"; else response += "OFF";
  response += "<br/>";
  
  response += "Red = ";
  response += oldRed;
  response += "</br>Green = ";
  response += oldGreen;
  response += "</br>Blue = ";
  response += oldBlue;
  response += "</br><a href=\"/\">Home</a>";

  sendHtml(request, "Lamp status", response);
}

void report_status_json(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("text/json");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  obj["burn"] = burning;
  obj["red"] = oldRed;
  obj["green"] = oldGreen;
  obj["blue"] = oldBlue;   

  obj.printTo(*response);
  request->send(response);
}


void handle_rgb(AsyncWebServerRequest *request)
{
  int newRed = oldRed, newGreen = oldGreen, newBlue = oldBlue;

  if (request->hasParam("red"))
  {
    AsyncWebParameter *p = request->getParam("red");
    String val = p->value().c_str();
    newRed = val.toInt();
  }
  if (request->hasParam("green"))
  {
    AsyncWebParameter *p = request->getParam("green");
    String val = p->value().c_str();
    newGreen = val.toInt();
  }
  if (request->hasParam("blue"))
  {
    AsyncWebParameter *p = request->getParam("blue");
    String val = p->value().c_str();
    newBlue = val.toInt();
  }

  if (newRed < 0) newRed = 0;
  if (newRed > 255) newRed = 255;
  if (newGreen < 0) newGreen = 0;
  if (newGreen > 255) newGreen = 255;
  if (newBlue < 0) newBlue = 0;
  if (newBlue > 255) newBlue = 255;

  setRGB(burning, newRed, newGreen, newBlue);
  saveLampState();
  
  report_status(request);
}

void sendHtml(AsyncWebServerRequest *request, String title, String body)
{
  String response("<html><head><title>");

  response += title;
  response += "</title></head><body>";
  response += body;
  response += "</body>";
  
  request->send(200, "text/html", response);
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
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }
  else
  {
    analogWrite(RED_PIN, oldRed);
    analogWrite(GREEN_PIN, oldGreen);
    analogWrite(BLUE_PIN, oldBlue);
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

void saveLampState()
{
  File f = SPIFFS.open("lampstate.json", "w");
  if (! f)
  {
    Serial.println("Could not save lamp state to disk");
    return;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  obj["burn"] = burning;
  obj["red"] = oldRed;
  obj["green"] = oldGreen;
  obj["blue"] = oldBlue;

  obj.prettyPrintTo(f);
  f.close();
}

bool loadLampState()
{
  File f = SPIFFS.open("lampstate.json", "r");
  if (! f) 
  {
    Serial.println("Cannot open lamp state file");
    return false;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(f);
  f.close();

  if (! obj.success())
  {
    Serial.println("Could not parse lamp state file");
    return false;
  }
  else
  {
    Serial.println("Lamp state file:");
    obj.prettyPrintTo(Serial);
  }

  burning = obj["burn"];
  oldRed = obj["red"];
  oldGreen = obj["green"];
  oldBlue = obj["blue"];

  return true;
}

void save_config()
{
  File f = SPIFFS.open("config.json", "w");
  if (! f)
  {
    Serial.println("Could not save config to disk");
    return;
  }

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  obj["name"] = lampName;
  obj["behavior"] = startAfterPowerOff;

  obj.prettyPrintTo(f);
  f.close();
}

bool load_config()
{
  File f = SPIFFS.open("config.json", "r");
  if (! f) 
  {
    Serial.println("Cannot open config file");
    return false;
  }

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(f);
  f.close();

  if (! obj.success())
  {
    Serial.println("Could not parse config file");
    return false;
  }
  else
  {
    Serial.println("Config file:");
    obj.prettyPrintTo(Serial);
  }

  set_name(obj["name"].as<String>());
  startAfterPowerOff = obj["behavior"];

  return true;
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




