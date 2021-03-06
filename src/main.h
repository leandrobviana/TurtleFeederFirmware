/**
 * TurtleFeederFirmware
 * Copyright (C) 2019 kacpi2442 [https://github.com/kacpi2442/TurtleFeederFirmware]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Servo.h>
#include <IPAddress.h>

void buttonHandle();
void ledHandle();
void checkConnection();
void blynkNotify(String text);
void blynkAddToTable(String text, String time);
void blynkLightButton(int value);
void blynkTimeString(String value);
void setUpdateFeed(boolean state);
void setUpdateLights(boolean state);

boolean get_wifi_connected();
boolean get_outOfFoodLED();

extern IPAddress timeServerIP;
extern Servo feeder;
