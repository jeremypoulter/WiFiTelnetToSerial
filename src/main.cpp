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
#include <ESP8266mDNS.h>

#include "debug.h"
#include "serial.h"

#define HOSTNAME "espserial"

const char* ssid = "wibble";
const char* password = "TheB1gJungle2";

SerialTask serial;

void setup() {
  DEBUG.begin(115200);

  WiFi.begin(ssid, password);
  DEBUG.print("\nConnecting to "); DEBUG.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    DEBUG.print("Could not connect to"); DEBUG.println(ssid);
    while(1) delay(500);
  }

  DEBUG.print("Ready! Use 'telnet ");
  DEBUG.print(WiFi.localIP());
  DEBUG.println(" 23' to connect");

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (MDNS.begin(HOSTNAME)) {
    DEBUG.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("telnet", "tcp", 23);
  }

  MicroTask.startTask(&serial);
}

void loop() {
  MicroTask.update();
}
