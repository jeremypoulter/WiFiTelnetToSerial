#ifndef __CONFIG_H
#define __CONFIG_H

#include <MicroTasks.h>
#include <MicroTasksEvent.h>
#include <ArduinoJson.h>

class ConfigStorage;

class ConfigClass : public MicroTasks::Event
{
private:
  DynamicJsonBuffer jsonBuffer;
  JsonVariant root;
  bool modified;
public:
  ConfigClass();

  void begin();
  void reset();
  void commit();

  template<typename T>
  T &get(const char *name, T &t) {
    t = root[name];
    return t;
  }

  template<typename T>
  const T &set(const char *name, const T &t) {
    root[name] = t;
    modified = true;
    Trigger();
    return t;
  }
};

extern ConfigClass Config; 

#endif //  __CONFIG_H
