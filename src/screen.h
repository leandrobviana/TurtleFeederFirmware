

#include <Arduino.h>
#include <config.h>

#define MAX_SCREEN 1
#define MIN_SCREEN 0


void start_display();
void drawScreen(int screenNumber, String status_message, String timer_time, boolean wifi_connected, boolean food_sensor, boolean food_low, boolean start_editing, boolean editing_blink, int timerHourSet, int timerMinuteSet);