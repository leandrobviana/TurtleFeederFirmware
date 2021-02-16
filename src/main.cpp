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
#include <BlynkSimpleEsp8266.h>
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

#define BLYNK_PRINT Serial
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "AuthToken";

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

boolean editing_feed_portion = false;

boolean editing_blink = false;

boolean editing_minute_on = false;
boolean editing_hour_off = false;
boolean editing_minute_off = false;

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
int feedPortionSet = 0;
int blynkFeedPortionSet = 0;

int lightOnHourSet = 0;
int lightOnMinuteSet = 0;
int lightOffHourSet = 0;
int lightOffMinuteSet = 0;

String what_is_editing;

WidgetTable table;
BLYNK_ATTACH_WIDGET(table, blynkTable);

int rowIndex = 0;

boolean sendedStartNotify = false;

boolean updateFeed = false;

int blynkLightsButtonValue;

boolean updateLights;

BLYNK_CONNECTED()
{
    //Blynk.syncVirtual(blynkFeedButton);
    //Blynk.syncVirtual(blynkLightsButton);
    //Blynk.syncVirtual(blynkNextFeedText);
    //Blynk.syncVirtual(blynkFoodLevel);

    //seconds from the start of a day. 0 - min, 86399 - max
    int FirstTimeFood = (getTimerIntFirstHour() * 60 + getTimerIntFirstMinute()) * 60;

    int LightsOnTime = (getTimerLightsOnHour() * 60 + getTimerLightsOnMinute()) * 60;
    int LightsOffTime = (getTimerLightsOffHour() * 60 + getTimerLightsOffMinute()) * 60;

    //timezone
    //full list of supported timezones could be found here
    //https://www.mkyong.com/java/java-display-list-of-timezone-with-gmt/
    char tz[] = "America/Sao_Paulo";

    Blynk.virtualWrite(blynkNextFeedText, nextTriggerStr());

    Blynk.virtualWrite(blynkFirstTimeFood, FirstTimeFood, FirstTimeFood, tz);
    Blynk.virtualWrite(blynkHowManyTimesFeed, getTimerIntHowManyTimes());
    Blynk.virtualWrite(blynkIntervalFeed, getTimerIntMinutesInterval());
    Blynk.virtualWrite(blynkPortionFeed, getFeedPortion() + 1);

    Blynk.virtualWrite(blynkLights, LightsOnTime, LightsOffTime, tz);
}

BLYNK_WRITE(blynkFeedButton)
{
    //int buttonState = param.asInt();
    if (param.asInt() == 1)
    {
        Feed(1);
        //updateFeed = true;
    }
}

BLYNK_WRITE(blynkLightsButton)
{
    blynkLightsButtonValue = param.asInt();
    updateLights = true;
}

BLYNK_WRITE(blynkFirstTimeFood)
{
    TimeInputParam t(param);
    if (t.hasStartTime())
    {
        firstFeedHourSet = t.getStartHour();
        firstFeedMinuteSet = t.getStartMinute();
    }
}

BLYNK_WRITE(blynkHowManyTimesFeed)
{
    feedHowManyTimesSet = param.asInt();
}

BLYNK_WRITE(blynkIntervalFeed)
{
    feedMinutesIntervalSet = param.asInt();
}

BLYNK_WRITE(blynkPortionFeed)
{
    blynkFeedPortionSet = param.asInt() - 1;
}

BLYNK_WRITE(blynkButtonSetFeedTime)
{
    int buttonState = param.asInt();
    if (buttonState == 1)
    {
        setTimer(firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                 feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet,
                 lightOffHourSet, lightOffMinuteSet, blynkFeedPortionSet);
    }
}

BLYNK_WRITE(blynkLights)
{
    TimeInputParam t(param);
    if (t.hasStartTime() && t.hasStopTime())
    {
        lightOnHourSet = t.getStartHour();
        lightOnMinuteSet = t.getStartMinute();
        lightOffHourSet = t.getStopHour();
        lightOffMinuteSet = t.getStopMinute();
    }
}

BLYNK_WRITE(blynkRestartESP)
{
    ESP.restart();
}

void blynkLightButton(int value)
{
    Blynk.virtualWrite(blynkLightsButton, value);
}

void blynkTimeString(String value)
{
    Blynk.virtualWrite(blynkNextFeedText, value);
}

void blynkNotify(String text)
{
    Blynk.email("Fish Feed Report", text);
}

void blynkAddToTable(String text, String time)
{
    table.addRow(rowIndex, text, time);
    table.pickRow(rowIndex);
    rowIndex++;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting...");

    start_display();
    //show_msg_display("Starting...");
    drawScreen(currentScreen, "Starting...", "", wifi_connected, outOfFood(), outOfFoodLED, start_editing,
               editing_blink, what_is_editing, timerHourSet, timerMinuteSet, firstFeedHourSet,
               firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet, lightOnHourSet,
               lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);
    pinMode(encoderSW, INPUT_PULLUP);
    //pinMode(led_RED, OUTPUT);
    //pinMode(led_GREEN, OUTPUT);
    //pinMode(led_BLUE, OUTPUT);
    pinMode(sensorPin, INPUT);
    pinMode(relePin, OUTPUT); // Declara o relé como uma saída
    digitalWrite(relePin, LOW);
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

    timerHourSet = getTimerIntHour();
    timerMinuteSet = getTimerIntMinute();

    firstFeedHourSet = getTimerIntFirstHour();
    firstFeedMinuteSet = getTimerIntFirstMinute();
    feedHowManyTimesSet = getTimerIntHowManyTimes();
    feedMinutesIntervalSet = getTimerIntMinutesInterval();
    feedPortionSet = getFeedPortion();

    lightOnHourSet = getTimerLightsOnHour();
    lightOnMinuteSet = getTimerLightsOnMinute();
    lightOffHourSet = getTimerLightsOffHour();
    lightOffMinuteSet = getTimerLightsOffMinute();

    drawScreen(currentScreen, "Connecting Blynk...", "", wifi_connected, outOfFood(),
               outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet,
               timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
               feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);

    Blynk.begin(auth, ssid, password);

    table.clear();
}

void setUpdateFeed(boolean state)
{
    updateFeed = state;
}

void setUpdateLights(boolean state)
{
    updateLights = state;
}

void loop()
{
    Blynk.run();

    if (!sendedStartNotify)
    {
        blynkNotify("Started Now " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        Blynk.notify("Started Now " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        sendedStartNotify = true;
    }

    if (updateFeed)
    {
        Blynk.virtualWrite(blynkNextFeedText, nextTriggerStr());
        if (outOfFood())
        {
            Blynk.notify("Low on Food!");
            blynkNotify("Low on Food! Feeded at " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
            blynkAddToTable("Low on Food! Feeded", formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        }
        else
        {
            blynkNotify("Feeded at " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
            blynkAddToTable("Feeded", formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        }
        updateFeed = false;
    }

    if (updateLights)
    {
        if (blynkLightsButtonValue == 1)
        {
            digitalWrite(relePin, HIGH);
            blynkAddToTable("Lights on", formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
            blynkNotify("Lights on " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        }
        else
        {
            digitalWrite(relePin, LOW);
            blynkAddToTable("Lights off", formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
            blynkNotify("Lights off " + formatTime(day()) + "/" + formatTime(month()) + " " + formatTime(hour()) + ":" + formatTime(minute()));
        }
        updateLights = false;
    }

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

        if (currentScreen == 1)
        {
            if (firstFeedHourSet < 23 &&
                start_editing &&
                !editing_minute &&
                !editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                firstFeedHourSet++;
            }

            if (firstFeedMinuteSet < 59 &&
                editing_minute &&
                !editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                firstFeedMinuteSet++;
            }

            if (feedHowManyTimesSet < 6 &&
                editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                feedHowManyTimesSet++;
            }

            if (feedMinutesIntervalSet < 240 &&
                editing_feed_time_interval &&
                !editing_feed_portion)
            {
                feedMinutesIntervalSet++;
            }

            if (feedPortionSet < 8 &&
                editing_feed_portion)
            {
                feedPortionSet++;
            }
        }

        if (currentScreen == 3)
        {
            if (lightOnHourSet < 23 &&
                start_editing &&
                !editing_minute_on &&
                !editing_hour_off &&
                !editing_minute_off)
            {
                lightOnHourSet++;
            }

            if (lightOnMinuteSet < 59 &&
                editing_minute_on &&
                !editing_hour_off &&
                !editing_minute_off)
            {
                lightOnMinuteSet++;
            }

            if (lightOffHourSet < 23 &&
                editing_hour_off &&
                !editing_minute_off)
            {
                lightOffHourSet++;
            }

            if (lightOffMinuteSet < 59 &&
                editing_minute_off)
            {
                lightOffMinuteSet++;
            }
        }

        encoderPosition = newPos;
    }

    if (newPos < encoderPosition - 2)
    {
        if (currentScreen > MIN_SCREEN && !start_editing)
        {
            currentScreen--;
        }

        if (currentScreen == 1)
        {
            if (firstFeedHourSet > 0 &&
                start_editing &&
                !editing_minute &&
                !editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                firstFeedHourSet--;
            }

            if (firstFeedMinuteSet > 0 &&
                editing_minute &&
                !editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                firstFeedMinuteSet--;
            }

            if (feedHowManyTimesSet > 0 &&
                editing_feed_how_many_times &&
                !editing_feed_time_interval &&
                !editing_feed_portion)
            {
                feedHowManyTimesSet--;
            }

            if (feedMinutesIntervalSet > 0 &&
                editing_feed_time_interval &&
                !editing_feed_portion)
            {
                feedMinutesIntervalSet--;
            }

            if (feedPortionSet > 0 &&
                editing_feed_portion)
            {
                feedPortionSet--;
            }
        }

        if (currentScreen == 3)
        {
            if (lightOnHourSet > 0 &&
                start_editing &&
                !editing_minute_on &&
                !editing_hour_off &&
                !editing_minute_off)
            {
                lightOnHourSet--;
            }

            if (lightOnMinuteSet > 0 &&
                editing_minute_on &&
                !editing_hour_off &&
                !editing_minute_off)
            {
                lightOnMinuteSet--;
            }

            if (lightOffHourSet > 0 &&
                editing_hour_off &&
                !editing_minute_off)
            {
                lightOffHourSet--;
            }

            if (lightOffMinuteSet > 0 &&
                editing_minute_off)
            {
                lightOffMinuteSet--;
            }
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
        drawScreen(currentScreen, "Connecting...", "", wifi_connected, outOfFood(), outOfFoodLED,
                   start_editing, editing_blink, what_is_editing, timerHourSet, timerMinuteSet,
                   firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet, feedMinutesIntervalSet, lightOnHourSet,
                   lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);
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
            drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected,
                       outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet,
                       timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                       feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);
        }
        else
        {
            drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected,
                       outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet,
                       timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                       feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);
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
                    if (editing_feed_how_many_times)
                    {
                        if (editing_feed_time_interval)
                        {
                            if (editing_feed_portion)
                            {
                                start_editing = !start_editing;
                                editing_minute = !editing_minute;
                                editing_feed_how_many_times = !editing_feed_how_many_times;
                                editing_feed_time_interval = !editing_feed_time_interval;
                                setTimer(firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                                         feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet,
                                         lightOffHourSet, lightOffMinuteSet, feedPortionSet);
                            }
                            editing_feed_portion = !editing_feed_portion;
                            what_is_editing = "feed_portion";
                        }
                        else
                        {
                            editing_feed_time_interval = !editing_feed_time_interval;
                            what_is_editing = "interval";
                        }
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

        if (currentScreen == 2)
        {
            changeLights();
        }

        if (currentScreen == 3)
        {
            if (start_editing)
            {
                if (editing_minute_on)
                {
                    if (editing_hour_off)
                    {
                        if (editing_minute_off)
                        {
                            start_editing = !start_editing;
                            editing_minute_on = !editing_minute_on;
                            editing_hour_off = !editing_hour_off;
                            setTimer(firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                                     feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet,
                                     lightOffHourSet, lightOffMinuteSet, feedPortionSet);
                        }
                        editing_minute_off = !editing_minute_off;
                        what_is_editing = "minuteLightOff";
                    }
                    else
                    {
                        editing_hour_off = !editing_hour_off;
                        what_is_editing = "hourLightOff";
                    }
                }
                else
                {
                    editing_minute_on = !editing_minute_on;
                    what_is_editing = "minuteLightOn";
                }
            }
            else
            {
                start_editing = !start_editing;
                what_is_editing = "hourLightOn";
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
        Blynk.virtualWrite(blynkFoodLevel, "Low");
    }
    else
    {
        outOfFoodLED = false;

        //RGB_color(0, standbyIntensivity, 0); // Default standby
        //show_msg_display("Wait time");
        drawScreen(currentScreen, "Waiting", getTimerHour() + ":" + getTimerMinute(), wifi_connected,
                   outOfFood(), outOfFoodLED, start_editing, editing_blink, what_is_editing, timerHourSet,
                   timerMinuteSet, firstFeedHourSet, firstFeedMinuteSet, feedHowManyTimesSet,
                   feedMinutesIntervalSet, lightOnHourSet, lightOnMinuteSet, lightOffHourSet, lightOffMinuteSet, feedPortionSet);

        Blynk.virtualWrite(blynkFoodLevel, "Ok");
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