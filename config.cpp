// Hippotronics LED lamp configuration functions

#include "hippoled.h"
#include "config.h"
#include "sonoff.h"

#include <ArduinoJson.h>

void save_config()
{
  File f = SPIFFS.open("config.json", "w");
  if (! f)
  {
    SERIALPORT.println("Could not save config to disk");
    return;
  }

  StaticJsonDocument<500> obj;
  
  obj["name"] = lampName;
  obj["behavior"] = startAfterPowerOff;
  obj["lamptype"] = type;

  for (int i=0; i<3; i++)
  {
    String butname("sonoff_button" + String(i));
    obj[butname] = SonoffButBehavior[i];
  }

  serializeJsonPretty(obj, f);
  f.close();
}

bool load_config()
{
  File f = SPIFFS.open("config.json", "r");
  if (! f) 
  {
    SERIALPORT.println("Cannot open config file");
    return false;
  }

  StaticJsonDocument<500> obj;
  DeserializationError error = deserializeJson(obj, f);
  f.close();

  if (error)
  {
    SERIALPORT.println("Could not parse config file");
    return false;
  }
  else
  {
    SERIALPORT.println("Config file:");
    serializeJsonPretty(obj,SERIALPORT);
  }

  set_name(obj["name"].as<String>());
  startAfterPowerOff = obj["behavior"];
  if (obj.containsKey("lamptype"))
    type = obj["lamptype"];
  else
    type = UNDEFINED;

  for (int i=0; i<3; i++)
  {
    String butname("sonoff_button" + String(i));
    if (obj.containsKey(butname))
        SonoffButBehavior[i] = obj[butname];
  }

  return true;
}

void saveLampState()
{
  File f = SPIFFS.open("lampstate.json", "w");
  if (! f)
  {
    SERIALPORT.println("Could not save lamp state to disk");
    return;
  }

  StaticJsonDocument<200> obj;
  
  obj["burn"] = burning;
  obj["red"] = oldRed;
  obj["green"] = oldGreen;
  obj["blue"] = oldBlue;

  serializeJsonPretty(obj, f);
  f.close();
}

bool loadLampState()
{
  File f = SPIFFS.open("lampstate.json", "r");
  if (! f) 
  {
    SERIALPORT.println("Cannot open lamp state file");
    return false;
  }

  StaticJsonDocument<200> obj;
  DeserializationError error = deserializeJson(obj, f);
  f.close();

  if (error)
  {
    SERIALPORT.println("Could not parse lamp state file");
    return false;
  }
  else
  {
    SERIALPORT.println("Lamp state file:");
    serializeJsonPretty(obj, SERIALPORT);
  }

  burning = obj["burn"];
  oldRed = obj["red"];
  oldGreen = obj["green"];
  oldBlue = obj["blue"];

  return true;
}
