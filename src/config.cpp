#include <EEPROM.h>

#include "config.h"

#define STORAGE_SIZE 4096

ConfigClass::ConfigClass() :
  jsonBuffer(),
  root(jsonBuffer.createObject()),
  modified(false)
{
}

void ConfigClass::begin()
{
  EEPROM.begin(STORAGE_SIZE);
  char start;
  int length;
  EEPROM.get(0, length);
  EEPROM.get(2, start);
  if(2 <= length && length < STORAGE_SIZE &&
    '{' == start)
  {
    char json[length + 1];
    for(int i = 0; i < length; i++) {
      json[i] = EEPROM.read(2+i);
    }
    json[length] = '\0';
    root = jsonBuffer.parse(json);
  } else {
    root = jsonBuffer.createObject();
    modified = true;
  }
}

void ConfigClass::reset()
{
  root = jsonBuffer.createObject();
  modified = true;
  Trigger();
  commit();
}

void ConfigClass::commit()
{
  if(false == modified) {
    return;
  }
  String jsonStr;
  root.printTo(jsonStr);
  const char *json = jsonStr.c_str();
  int length = jsonStr.length();
  EEPROM.put(0, length);
  for(int i = 0; i < length; i++) {
    EEPROM.write(2+i, json[i]);
  }

  EEPROM.commit();
}
