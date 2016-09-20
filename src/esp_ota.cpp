#include <Arduino.h>
#include <ArduinoOTA.h>               // local OTA update from Arduino IDE

#include "debug.h"
#include "esp_ota.h"

EspOtaTask::EspOtaTask()
{

}

void EspOtaTask::setup()
{
  // Start local OTA update server
  ArduinoOTA.begin();
}

unsigned long EspOtaTask::loop(MicroTasks::WakeReason reason)
{
  ArduinoOTA.handle();
  return 0;
}
