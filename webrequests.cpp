// Functions that deal with web requests

#include "hippoled.h"
#include "config.h"
#include "webrequests.h"

void handle_index(AsyncWebServerRequest *request)
{
  String response("<h1>Hippotronics LED lamp</h1>"
    "<p>Software version:"
    VERSION
    "</p>"
    "<p><a href=\"on.html\">Switch On</a></p>" 
    "<p><a href=\"off.html\">Switch Off</a></p>");

  if (type == COLOR1D || type == COLORRGB || type == DIMMABLE)
  {
    
    response +=  "<p><form method=\"GET\" action=\"rgb.html\">"
      "Red <input type=\"text\" name=\"red\" value=\"";
    response += oldRed;
  
    if (type == COLOR1D || type == COLORRGB)
    {
      response += "\"/><br/>Green <input type=\"text\" name=\"green\" value=\"";
     response += oldGreen;
    }
  
    if (type == COLORRGB)
    {
      response += "\"/><br/>Blue <input type=\"text\" name=\"blue\" value=\"";
     response += oldBlue;
    }
    
    response += "\"/><br/><input type=\"submit\"/></form></p>";

  }
  
  response += "</br><a href=\"/config.html\">Configuration</a>";

  sendHtml(request, "Hippotronics LED lamp " + lampName, response);
}

void handle_config(AsyncWebServerRequest *request)
{
  String response("<h1>Configuration</h1>"
    "<p style=\"color: red\">Note: setting host name or changing the lamp type resets lamp unit</p>"
    "<p><form method=\"GET\" action=\"setconfig.html\">"
    "Name <input type=\"text\" name=\"name\" value=\"");

  response += lampName;
  response += "\"/><br/>Power-on behavior<br/><label for=\"b_off\">Off</label><input type=\"radio\" name=\"behavior\" id=\"b_off\" value=\"0\" ";
  response += (START_OFF == startAfterPowerOff) ? "checked" : "";
  response += "/><br/><label for=\"b_on\">On</label><input type=\"radio\" name=\"behavior\" id=\"b_on\" value=\"1\" ";
  response += (START_ON == startAfterPowerOff) ? "checked" : "";
  response += "/><br/><label for=\"b_last\">Return to last state when power off</label><input type=\"radio\" name=\"behavior\" id=\"b_last\" value=\"2\" ";
  response += (START_LAST == startAfterPowerOff) ? "checked" : "";
  response += "/><br/>";

  response += "<select name=\"lamptype\">";
  response += "<option value=0 ";
  response += (UNDEFINED == type) ? "selected" : "";
  response += ">Not set</option>";
  response += "<option value=1 ";
  response += (DIMMABLE == type) ? "selected" : "";
  response += ">Dimmable</option>";
  response += "<option value=2 ";
  response += (COLOR1D == type) ? "selected" : "";
  response += ">Colour temperature, dimmable</option>";
  response += "<option value=3 ";
  response += (COLORRGB == type) ? "selected" : "";
  response += ">Full colour</option>";
  response += "<option value=4 ";
  response += (SWITCH == type) ? "selected" : "";
  response += ">On/off switch</option>";

  response += "</select>";
  response += "<input type=\"submit\"/></form></p>";
  response += "</br><a href=\"/\">Home</a>";
  
  sendHtml(request, "Hippotronics LED lamp configuration", response);
}

void handle_setconfig(AsyncWebServerRequest *request)
{
  int newBehavior = startAfterPowerOff;
  int newType = type;
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

  if (request->hasParam("lamptype"))
  {
    AsyncWebParameter *p = request->getParam("lamptype");
    String val = p->value().c_str();
    newType = val.toInt();
    
    if (newType >= UNDEFINED && newType <= SWITCH && newType != type) 
    {
      type = newType;

      bReset = true;
    }
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

void handle_setconfig_json(AsyncWebServerRequest *request, const char *data)
{
  int newBehavior = startAfterPowerOff;
  int newType = type;
  bool bReset = false;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(data);

  if (obj.containsKey("name"))
  {
    String val = obj["name"].as<String>();

    if (! val.equals(lampName))
    {
      // Set the new name
      set_name(val);

      // Reset the processor, but first redirect the client
      bReset = true;
    }
  }
  if (obj.containsKey("behavior"))
  {
    newBehavior = obj["behavior"];
    
    if (newBehavior >= START_OFF && newBehavior <= START_LAST) startAfterPowerOff = newBehavior;
  }

  if (obj.containsKey("lamptype"))
  {
    newType = obj["lamptype"];
    
    if (newType >= UNDEFINED && newType <= SWITCH && newType != type) 
    {
      type = newType;

      bReset = true;
    }
  }

  save_config();

  if (bReset)
  {
    request->send(200, "text/plain", "RESET");

    gResetFlag = true;    // Will reset in loop()
  }
  else
  {
    request->send(200, "text/plain", "OK");
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

void report_version_json(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("text/json");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  obj["version"] = VERSION;

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

void handle_rgb_json(AsyncWebServerRequest *request, const char *data)
{
  int newBurning = burning, newRed = oldRed, newGreen = oldGreen, newBlue = oldBlue;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(data);

  if (obj.containsKey("burn")) newBurning = obj["burn"];
  if (obj.containsKey("red")) newRed = obj["red"];
  if (obj.containsKey("green")) newGreen = obj["green"];
  if (obj.containsKey("blue")) newBlue = obj["blue"];

  if (newBurning > 0) newBurning = 1;
  if (newRed < 0) newRed = 0;
  if (newRed > 255) newRed = 255;
  if (newGreen < 0) newGreen = 0;
  if (newGreen > 255) newGreen = 255;
  if (newBlue < 0) newBlue = 0;
  if (newBlue > 255) newBlue = 255;

  setRGB(newBurning, newRed, newGreen, newBlue);
  saveLampState();
  
  request->send(200, "text/plain", "OK");
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


void setup_web_paths()
{
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

  server.on("/version.json", HTTP_GET, report_version_json);

  // Handle JSON posts
  server.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) 
  {
    if (request->url() == "/rgb.json") handle_rgb_json(request, (const char *) data);
    else if (request->url() == "/setconfig.json") handle_setconfig_json(request, (const char *) data);
  });
  
  server.begin();
}

