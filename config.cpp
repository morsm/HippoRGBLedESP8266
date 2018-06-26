// Hippotronics LED lamp configuration functions

#include "hippoled.h"
#include "config.h"


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


