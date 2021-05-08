#ifndef APP_BT__H
#define APP_BT__H

TaskHandle_t bluetooth_handler;

void bluetooth_mng_task(void *pvParameters);
void bluetooth_init(void);

#endif