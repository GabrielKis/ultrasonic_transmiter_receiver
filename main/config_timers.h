#ifndef CONFIG_TIMERS_H
#define CONFIG_TIMERS_H

#include "driver/timer.h"

#define TIMER_DIVIDER          16
#define TIMER_SCALE            (TIMER_BASE_CLK / TIMER_DIVIDER)     // convert counter value to seconds
#define TIMER_INTERVAL_SEC     (20)                                  // sample test interval for the first timer
#define ALARM_VAL_US           (TIMER_INTERVAL_SEC * TIMER_SCALE)
#define WITH_RELOAD            1

void config_timer(int timer_idx, bool auto_reload);

#endif