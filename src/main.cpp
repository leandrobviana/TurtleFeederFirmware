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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <config.h>
#include <main.h>
#include <utils.h>
#include <screen.h>
#include <Encoder.h>
#ifdef USE_OTA
#include <OTA.h>
#endif
#ifdef USE_MQTT
#include <mqtt.h>
#endif
#ifdef USE_TELEGRAM
#include <telegram.h>
#endif
#include <timer.h>

const char *ssid = STASSID;
const char *password = STAPSK;

// Tracks the last time event fired
unsigned long previousMillis = 0;

// Used to track if LED should be on or off
boolean outOfFoodLED = false;

boolean wifi_connected = false;

boolean start_editing = false;

boolean editing_minute = false;

boolean editing_feed_how_many_times = false;

boolean editing_feed_time_interval = false;

boolean editing_blink = false;

int timerHourSet = 0;
int timerMinuteSet = 0;

// Button variables
int buttonVal = 0;       // value read from button
int buttonLast = 1;      // buffered value of the button's previous state
unsigned long btnDnTime; // time the button was pressed down
unsigned long btnUpTime; // time the button was released

Servo feeder; // create servo object to control a servo

Encoder myEnc(encoderDT, encoderCLK);

long encoderPosition = 0;

int currentScreen = 0;

int firstFeedHourSet = 0;
int firstFeedMinuteSet = 0;

int feedHowManyTimesSet = 0;

int feedMinutesIntervalSet = 0;

String what_is_editing;

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting...");

    start_display();
    //show_msg_display("Starting...");
    drawScreen(currentScreen, "Starting...", "", wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
    pinMode(encoderSW, INPUT_PULLUP);
    //pinMode(led_RED, OUTPUT);
    //pinMode(led_GREEN, OUTPUT);
    //pinMode(led_BLUE, OUTPUT);
    pinMode(sensorPin, INPUT);
    pinMode(relePin, OUTPUT); // Declara o relé como uma saída
    feeder.attach(servoPin);
    feeder.write(homePosition);

    //RGB_color(255, 0, 0);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && millis() < 15000)
    {
        yield();
    }

    wifi_connected = WiFi.status() == WL_CONNECTED;
#ifdef USE_OTA
    SetupOTA();
#endif
    Udp.begin(NTP_localPort);
    setSyncProvider(getNtpTime);
#ifdef useDST
    if (inSummerTime(now()))
    {
        Serial.println("DST detected, adjusting time");
        setSyncProvider(getNtpTime); // Re-sync time
    }
#endif
#ifdef USE_MQTT
    mqttInit();
    mqttTryToConnect();
    Alarm.timerRepeat(60, mqttTryToConnect);
#endif
#ifdef USE_TELEGRAM
    telegramInit();
    Alarm.timerRepeat(5, telegramHandle);
#endif
    initTimer();
    Alarm.delay(500);
    feeder.detach();

    //RGB_color(0, standbyIntensivity, 0);

    //show_msg_display("Setup Done");
    drawScreen(currentScreen, "Starting...", "", wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);

    timerHourSet = getTimerIntHour();
    timerMinuteSet = getTimerIntMinute();

    firstFeedHourSet = getTimerIntFirstHour();
    firstFeedMinuteSet = getTimerIntFirstMinute();

    feedHowManyTimesSet = getTimerIntHowManyTimes();

    feedMinutesIntervalSet = getTimerIntMinutesInterval();
}

void loop()
{
    checkConnection();
    Alarm.delay(0);

    //boolean editing_feed_how_many_times = false;

    //boolean editing_feed_time_interval = false;

    long newPos = myEnc.read();

    if (newPos > encoderPosition + 2)
    {
        if (currentScreen < MAX_SCREEN && !start_editing)
        {
            currentScreen++;
        }

        if (firstFeedHourSet < 23 &&
            start_editing &&
            !editing_minute &&
            !editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            firstFeedHourSet++;
        }

        if (firstFeedMinuteSet < 59 &&
            editing_minute &&
            !editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            firstFeedMinuteSet++;
        }

        if (feedHowManyTimesSet < 6 &&
            editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            feedHowManyTimesSet++;
        }

        if (feedMinutesIntervalSet < 240 &&
            editing_feed_time_interval)
        {
            feedMinutesIntervalSet++;
        }
        encoderPosition = newPos;
    }

    if (newPos < encoderPosition - 2)
    {
        if (currentScreen > MIN_SCREEN && !start_editing)
        {
            currentScreen--;
        }

        if (firstFeedHourSet > 0 &&
            start_editing &&
            !editing_minute &&
            !editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            firstFeedHourSet--;
        }

        if (firstFeedMinuteSet > 0 &&
            editing_minute &&
            !editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            firstFeedMinuteSet--;
        }

        if (feedHowManyTimesSet > 0 &&
            editing_feed_how_many_times &&
            !editing_feed_time_interval)
        {
            feedHowManyTimesSet--;
        }

        if (feedMinutesIntervalSet > 0 &&
            editing_feed_time_interval)
        {
            feedMinutesIntervalSet--;
        }

        encoderPosition = newPos;
    }

#ifdef USE_MQTT
    client.loop();
#endif
#ifdef USE_OTA
    handleOTA();
#endif

    buttonHandle();
    ledHandle();
}

void checkConnection()
{
    wifi_connected = WiFi.status() == WL_CONNECTED;
    while (WiFi.status() != WL_CONNECTED)
    {

        //show_msg_display("No Wifi");
        drawScreen(currentScreen, "Connecting...", "", wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
        //RGB_color(255, 0, 0);

        Alarm.delay(0);
        buttonHandle();
    }
}

void blink_stuff()
{

    unsigned long currentMillis = millis();

    if ((unsigned long)(currentMillis - previousMillis) >= ledBlinkTime)
    {
        editing_blink = !(editing_blink); // Invert state
        outOfFoodLED = !(outOfFoodLED);   // Invert state
        previousMillis = currentMillis;   // Save the current time to compare "later"

        if (editing_blink)
        {
            drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
        }
        else
        {
            drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
        }
    }
}

void buttonHandle()
{
    // Read the state of the button
    buttonVal = digitalRead(encoderSW);
    // Test for button pressed and store the down time
    if (buttonVal == LOW && buttonLast == HIGH && (millis() - btnUpTime) > long(debounce))
        btnDnTime = millis();

    // Test for button release and store the up time
    if (buttonVal == HIGH && buttonLast == LOW && (millis() - btnDnTime) > long(debounce))
    {
        // Short button press action
        if (currentScreen == 0)
        {
            Feed(1);
            btnUpTime = millis();
        }
        if (currentScreen == 1)
        {
            if (start_editing)
            {
                if (editing_minute)
                {
                    //start_editing = !start_editing;
                    //setTimer(timerHourSet, timerMinuteSet);
                    if (editing_feed_how_many_times)
                    {
                        if (editing_feed_time_interval)
                        {
                            start_editing = !start_editing;
                            editing_minute = !editing_minute;
                            editing_feed_how_many_times = !editing_feed_how_many_times;
                            setTimer(firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
                        }
                        editing_feed_time_interval = !editing_feed_time_interval;
                        what_is_editing = "interval";
                    }
                    else
                    {
                        editing_feed_how_many_times = !editing_feed_how_many_times;
                        what_is_editing = "time";
                    }
                }
                else
                {
                    editing_minute = !editing_minute;
                    what_is_editing = "minute";
                }
            }
            else
            {
                start_editing = !start_editing;
                what_is_editing = "hour";
            }
        }
    }

    if (start_editing)
    {
        blink_stuff();
    }

    // Test for button held down for longer than the hold time
    if (buttonVal == LOW && (millis() - btnDnTime) > long(holdTime))
    {
        // Long button press action
        ESP.restart();
    }
    buttonLast = buttonVal;
}

void ledHandle()
{
    if (outOfFood())
    {
        blink_stuff();
    }
    else
    {
        outOfFoodLED = false;
        //RGB_color(0, standbyIntensivity, 0); // Default standby
        //show_msg_display("Wait time");
        drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected, outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet);
    }
}

boolean get_wifi_connected()
{
    return wifi_connected;
}

boolean get_outOfFoodLED()
{
    return outOfFoodLED;
}