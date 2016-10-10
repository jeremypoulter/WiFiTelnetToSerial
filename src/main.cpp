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

#define HOSTNAME "espserial"

const char* ssid = "wibble";
const char* password = "TheB1gJungle2";

SerialTask serial;
EspOtaTask espOta(HOSTNAME);
WebUiTask webUi(serial);
TelnetTask telnet(serial);

void setup() {
  DEBUG_BEGIN(115200);

  WiFi.begin(ssid, password);
  DBUGF("\nConnecting to %s\n", ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21) {
    DBUGF("Could not connect to %s\n", ssid);
    while(1) delay(500);
  }

  SPIFFS.begin();

  MicroTask.startTask(espOta);
  MicroTask.startTask(serial);
  MicroTask.startTask(webUi);
  MicroTask.startTask(telnet);

  DBUG("Ready! Use 'telnet ");
  DBUG(WiFi.localIP());
  DBUGLN(" 23' to connect");
}

void loop() {
  MicroTask.update();
}
