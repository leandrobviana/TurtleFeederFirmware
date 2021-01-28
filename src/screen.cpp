#include <config.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <images.h>
#include <time.h>

Adafruit_SSD1306 OLED(OLED_RESET);

void start_display()
{
    OLED.begin();
    OLED.clearDisplay();
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

void drawHomeScreen(String status_message, String timer_time, boolean wifi_connected, boolean food_sensor, boolean food_low)
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
        OLED.println("Food time: " + timer_time);
        OLED.println(status_message);
    }

    OLED.display(); //output 'display buffer' to screen
}

void drawEditTimerScreen(String timer_time, boolean wifi_connected, boolean food_sensor, boolean food_low, boolean start_editing, boolean editing_blink, int timerHourSet, int timerMinuteSet)
{
    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawFastHLine(0, 11, 128, WHITE);
    OLED.setCursor(0, 0);

    top_message(food_sensor, food_low);
    wifi_status_img(wifi_connected);

    OLED.println("food time: " + timer_time);

    if (start_editing)
    {
        if (editing_blink)
        {
            OLED.println("New food time: " + formatTime(timerHourSet) + ":" + formatTime(timerMinuteSet));
        }
        else
        {
            OLED.println("New food time: ");
        }
    }
    else
    {
        OLED.println("Click to edit time");
    }

    OLED.display(); //output 'display buffer' to screen
}

void drawScreen(int screenNumber, String status_message, String timer_time, boolean wifi_connected, boolean food_sensor, boolean food_low, boolean start_editing, boolean editing_blink, int timerHourSet, int timerMinuteSet)
{
    switch (screenNumber)
    {
    case 0:
        drawHomeScreen(status_message, timer_time, wifi_connected, food_sensor, food_low);
        break;
    case 1:
        drawEditTimerScreen(timer_time, wifi_connected, food_sensor, food_low, start_editing, editing_blink, timerHourSet, timerMinuteSet);
        break;
    default:
        break;
    }
}

void show_msg_display(String text)
{
    //Add stuff into the 'display buffer'
    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.setCursor(0, 0);
    OLED.println(text);
    OLED.drawBitmap(110, 0, myBitmap, 15, 10, WHITE);
    OLED.display(); //output 'display buffer' to screen
}

void show_msg_display_feed(String text)
{
    //Add stuff into the 'display buffer'
    OLED.clearDisplay();
    OLED.setTextWrap(false);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.setCursor(0, 10);
    OLED.println(text);
    OLED.drawBitmap(110, 0, myBitmap, 15, 10, WHITE);
    OLED.display(); //output 'display buffer' to screen
}