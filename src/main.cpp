/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FS.h>

#include "debug.h"
#include "esp_ota.h"
#include "serial.h"
#include "telnet.h"
#include "web_ui.h"
#include "wifi_manager.h"

#ifndef HOSTNAME
#define HOSTNAME "espserial"
#endif

const char* ssid = "wibble";
const char* password = "TheB1gJungle2";

SerialTask serial;
EspOtaTask espOta(HOSTNAME);
WebUiTask webUi(serial);
TelnetTask telnet(serial);
WiFiManagerTask wifi(HOSTNAME, ssid, password);

void setup() {
  DEBUG_BEGIN(115200);
  SPIFFS.begin();

  MicroTask.startTask(espOta);
  MicroTask.startTask(serial);
  MicroTask.startTask(webUi);
  MicroTask.startTask(telnet);
  MicroTask.startTask(wifi);
}

void loop() {
  MicroTask.update();
}
