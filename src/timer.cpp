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
#include <stdio.h> /* puts, printf */
#include <config.h>
#include <TimeAlarms.h>
#include <EEPROM.h>
#include <utils.h>
#include <timer.h>
#include <time.h>

int timerHour;
int timerMinute;

int firstFeedHour;
int firstFeedMinute;

int feedHowManyTimes;

int feedMinutesInterval;

int currentFeedInterval = 0;

AlarmID_t alarms[dtNBR_ALARMS];

void RegularFeed()
{
    Feed(regularFeed);
}

void turnOnLights(int turn_on)
{
    digitalWrite(relePin, turn_on);
    Serial.println(digitalRead(relePin));
}

void initTimer()
{
    EEPROM.begin(6);
    delay(10);
    if (EEPROM.read(0) == 1)
    {
        firstFeedHour = EEPROM.read(1);
        firstFeedMinute = EEPROM.read(2);
        feedHowManyTimes = EEPROM.read(3);
        feedMinutesInterval = EEPROM.read(4);
        setNextFeedTime();
    }
    EEPROM.end();
}

String formatTime(int time)
{
    if (time < 10)
    {
        return "0" + String(time);
    }
    else
    {
        return String(time);
    }
}

void setNextFeedTime()
{

    Serial.println(formatTime(hour()) + ":" + formatTime(minute()));

    for (int i = 0; i < dtNBR_ALARMS; i++)
    {
        if (Alarm.isAlarm(alarms[i]))
        {
            Alarm.free(alarms[i]);
        }
        alarms[i] = 0;
    }

    for (int i = 0; i < feedHowManyTimes; i++)
    {
        int next_feed_minutes_to_add = i * feedMinutesInterval;

        int next_feed_hour = firstFeedHour + next_feed_minutes_to_add / 60;
        int next_feed_minute = firstFeedMinute + next_feed_minutes_to_add % 60;

        if (next_feed_minute > 59)
        {
            next_feed_minute = next_feed_minute - 60;
            next_feed_hour = next_feed_hour + 1;
        }
        if (next_feed_hour > 23)
        {
            next_feed_hour = next_feed_hour - 24;
        }
        timerHour = next_feed_hour;
        timerMinute = next_feed_minute;

        alarms[i] = Alarm.alarmRepeat(next_feed_hour, next_feed_minute, 0, RegularFeed);

        Serial.println(formatTime(next_feed_hour) + ":" + formatTime(next_feed_minute));
    }

    Serial.println("Alarm count: ");
    Serial.println(Alarm.count());

    for (int i = 0; i < feedHowManyTimes; i++)
    {
        if (Alarm.isAlarm(alarms[i]))
        {
            Serial.println("next ");
            Serial.println(formatTime(hour(Alarm.getNextTrigger(alarms[i]))) + ":" + formatTime(minute(Alarm.getNextTrigger(alarms[i]))));
        }
    }
}

String nextTriggerStr()
{
    return formatTime(hour(Alarm.getNextTrigger())) + ":" + formatTime(minute(Alarm.getNextTrigger()));
}

void setTimer(int hour, int minute, int times, int interval)
{
    EEPROM.begin(6);
    delay(10);
    EEPROM.write(0, 1);
    EEPROM.write(1, hour);
    EEPROM.write(2, minute);
    EEPROM.write(3, times);
    EEPROM.write(4, interval);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("Timer data saved to EEPROM. Restarting.");
    ESP.restart();
}

String getTimerHour()
{
    if (timerHour < 10)
    {
        return "0" + String(timerHour);
    }
    else
    {
        return String(timerHour);
    }
}

String getTimerMinute()
{
    if (timerMinute < 10)
    {
        return "0" + String(timerMinute);
    }
    else
    {
        return String(timerMinute);
    }
}

String getTimerFirstHour()
{
    if (firstFeedHour < 10)
    {
        return "0" + String(firstFeedHour);
    }
    else
    {
        return String(firstFeedHour);
    }
}

String getTimerFirstMinute()
{
    if (firstFeedMinute < 10)
    {
        return "0" + String(firstFeedMinute);
    }
    else
    {
        return String(firstFeedMinute);
    }
}

String getTimerHowManyTimes()
{
    return String(feedHowManyTimes);
}

String getTimerMinutesInterval()
{
    if (feedMinutesInterval < 10)
    {
        return "00" + String(feedMinutesInterval);
    }
    else if (feedMinutesInterval < 100)
    {
        return "0" + String(feedMinutesInterval);
    }
    else
    {
        return String(feedMinutesInterval);
    }
}

int getTimerIntHour()
{
    return timerHour;
}
int getTimerIntMinute()
{
    return timerMinute;
}

int getTimerIntFirstHour()
{
    return firstFeedHour;
}
int getTimerIntFirstMinute()
{
    return firstFeedMinute;
}

int getTimerIntHowManyTimes()
{
    return feedHowManyTimes;
}
int getTimerIntMinutesInterval()
{
    return feedMinutesInterval;
}
void disableTimer()
{
    EEPROM.begin(6);
    delay(10);
    EEPROM.write(0, 0);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("Timer data saved to EEPROM. Restarting.");
    ESP.restart();
}