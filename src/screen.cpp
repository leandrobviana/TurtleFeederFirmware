#include <config.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <images.h>
#include <time.h>
#include <timer.h>
#include <main.h>

Adafruit_SSD1306 OLED(OLED_RESET);

void start_display()
{
    OLED.begin();
    OLED.ssd1306_command(SSD1306_COMSCANINC);
    OLED.ssd1306_command(SSD1306_SEGREMAP);
    OLED.clearDisplay();
}

void top_message(boolean food_sensor, boolean food_low)
{
    if (food_sensor)
    {
        if (food_low)
        {
            OLED.println(formatTime(hour()) + ":" + formatTime(minute()) + "   Low food");
        }
        else
        {
            OLED.println(formatTime(hour()) + ":" + formatTime(minute()));
        }
        OLED.println("");
    }
    else
    {
        OLED.println(formatTime(hour()) + ":" + formatTime(minute()) + "   food ok");
        OLED.println("");
    }
}

void wifi_status_img(boolean wifi_connected)
{
    if (wifi_connected)
    {
        OLED.drawBitmap(110, 0, myBitmap, 15, 10, WHITE);
    }
}

String formatFeedMinutesInterval(int interval)
{
    if (interval < 10)
    {
        return "00" + String(interval);
    }
    else
    {
        if (interval < 100)
        {
            return "0" + String(interval);
        }
        else
        {
            return String(interval);
        }
    }
}

void drawHomeScreen(String status_message, String timer_time, boolean wifi_connected, boolean food_sensor, boolean food_low,
                    int feedPortionSet)
{
    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawFastHLine(0, 11, 128, WHITE);
    OLED.setCursor(0, 0);

    top_message(food_sensor, food_low);
    wifi_status_img(wifi_connected);

    if (status_message == "Starting..." || status_message == "Connecting...")
    {
        OLED.println(status_message);
    }
    else
    {
        OLED.println("Food: " + nextTriggerStr() + "  P: " + (feedPortionSet + 1));
        OLED.println(status_message);
    }

    OLED.display(); //output 'display buffer' to screen
}

void drawEditTimerScreen(String timer_time, boolean wifi_connected, boolean food_sensor,
                         boolean food_low, boolean start_editing, boolean editing_blink, String what_is_editing,
                         int firstFeedHour, int firstFeedMinute, int feedHowManyTimes, int feedMinutesInterval,
                         int feedPortionSet)
{
    String blink_in;
    String blink_out;

    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawFastHLine(0, 11, 128, WHITE);
    OLED.setCursor(0, 0);

    top_message(food_sensor, food_low);
    wifi_status_img(wifi_connected);

    if (what_is_editing == "hour")
    {
        blink_in = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                   feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                   "min\nPortion: " + (feedPortionSet + 1);
        blink_out = "New:   :" + formatTime(firstFeedMinute) + " " + feedHowManyTimes + " x " +
                    formatFeedMinutesInterval(feedMinutesInterval) +
                    "min\nPortion: " + (feedPortionSet + 1);
    }
    if (what_is_editing == "minute")
    {
        blink_in = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                   feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                   "min\nPortion: " + (feedPortionSet + 1);
        blink_out = "New: " + formatTime(firstFeedHour) + ":   " + feedHowManyTimes + " x " +
                    formatFeedMinutesInterval(feedMinutesInterval) +
                    "min\nPortion: " + (feedPortionSet + 1);
    }
    if (what_is_editing == "time")
    {
        blink_in = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                   feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                   "min\nPortion: " + (feedPortionSet + 1);
        blink_out = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + "   x " +
                    formatFeedMinutesInterval(feedMinutesInterval) +
                    "min\nPortion: " + (feedPortionSet + 1);
    }
    if (what_is_editing == "interval")
    {
        blink_in = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                   feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                   "min\nPortion: " + (feedPortionSet + 1);
        blink_out = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                    feedHowManyTimes + " x    min\nPortion: " + (feedPortionSet + 1);
    }

    if (what_is_editing == "feed_portion")
    {
        blink_in = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                   feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                   "min\nPortion: " + (feedPortionSet + 1);
        blink_out = "New: " + formatTime(firstFeedHour) + ":" + formatTime(firstFeedMinute) + " " +
                    feedHowManyTimes + " x " + formatFeedMinutesInterval(feedMinutesInterval) +
                    "min\nPortion: ";
    }

    if (start_editing)
    {

        if (editing_blink)
        {
            OLED.println(blink_in);
        }
        else
        {
            OLED.println(blink_out);
        }
    }
    else
    {
        OLED.println("Click to edit");
        OLED.println("food time and portion");
    }

    OLED.display(); //output 'display buffer' to screen
}

void drawEditTimerLightsScreen(boolean wifi_connected, boolean food_sensor, boolean food_low,
                               boolean start_editing, boolean editing_blink, String what_is_editing,
                               int LightsOnHour, int LightsOnMinute, int LightsOffHour, int LightsOffMinute)
{
    String blink_in;
    String blink_out;

    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawFastHLine(0, 11, 128, WHITE);
    OLED.setCursor(0, 0);

    top_message(food_sensor, food_low);
    wifi_status_img(wifi_connected);

    if (what_is_editing == "hourLightOn")
    {
        blink_in = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
        blink_out = "New on:    :" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
    }
    if (what_is_editing == "minuteLightOn")
    {
        blink_in = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
        blink_out = "New on:  " + formatTime(LightsOnHour) + ":  \nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
    }
    if (what_is_editing == "hourLightOff")
    {
        blink_in = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
        blink_out = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off:   :" + formatTime(LightsOffMinute);
    }
    if (what_is_editing == "minuteLightOff")
    {
        blink_in = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":" + formatTime(LightsOffMinute);
        blink_out = "New on:  " + formatTime(LightsOnHour) + ":" + formatTime(LightsOnMinute) + "\nNew off: " + formatTime(LightsOffHour) + ":  ";
    }

    if (start_editing)
    {

        if (editing_blink)
        {
            OLED.println(blink_in);
        }
        else
        {
            OLED.println(blink_out);
        }
    }
    else
    {
        OLED.println("Click to edit");
        OLED.println("lights time");
    }

    OLED.display(); //output 'display buffer' to screen
}

void drawHomeLightsScreen(boolean wifi_connected, boolean food_sensor, boolean food_low)
{
    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawFastHLine(0, 11, 128, WHITE);
    OLED.setCursor(0, 0);

    top_message(food_sensor, food_low);
    wifi_status_img(wifi_connected);

    OLED.println("Lights On:  " + lightsOnString());
    OLED.println("Lights Off: " + lightsOffString());

    OLED.display(); //output 'display buffer' to screen
}

void drawScreen(int screenNumber, String status_message, String timer_time, boolean wifi_connected,
                boolean food_sensor, boolean food_low, boolean start_editing, boolean editing_blink,
                String what_is_editing, int timerHourSet, int timerMinuteSet, int firstFeedHour,
                int firstFeedMinute, int feedHowManyTimes, int feedMinutesInterval,
                int LightsOnHour, int LightsOnMinute, int LightsOffHour, int LightsOffMinute,
                int feedPortionSet)
{
    switch (screenNumber)
    {
    case 0:
        drawHomeScreen(status_message, timer_time, wifi_connected, food_sensor, food_low, feedPortionSet);
        break;
    case 1:
        drawEditTimerScreen(timer_time, wifi_connected, food_sensor, food_low, start_editing, editing_blink,
                            what_is_editing, firstFeedHour, firstFeedMinute, feedHowManyTimes, feedMinutesInterval,
                            feedPortionSet);
        break;

    case 2:
        drawHomeLightsScreen(wifi_connected, food_sensor, food_low);
        break;
    case 3:
        drawEditTimerLightsScreen(wifi_connected, food_sensor, food_low, start_editing, editing_blink,
                                  what_is_editing, LightsOnHour, LightsOnMinute, LightsOffHour, LightsOffMinute);
    default:
        break;
    }
}