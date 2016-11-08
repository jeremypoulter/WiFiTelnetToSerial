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
public:
  ConfigClass();

  void begin();
  void reset();
  void commit();

  template<typename T>
  T &get(const char *name, T &t) {
    root[name] = t;
    return t;
  }

  template<typename T>
  const T &set(const char *name, const T &t) {
    t = root[name];
    return t;
  }
};

#endif //  __CONFIG_H
