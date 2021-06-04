#ifndef WIFI_MQTT_INTERFACE_H
#define WIFI_MQTT_INTERFACE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "mqtt_client.h"
#include "gen_signal.h"

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       1

#define DATA_RECV_MQTT_PAYLOAD_SIZE 2 + (SAMPLES_PER_PERIOD)

EventGroupHandle_t wifi_event_group;
esp_mqtt_client_handle_t esp_mqtt_client;

uint8_t data_received_mqtt[DATA_RECV_MQTT_PAYLOAD_SIZE];

void wifi_init_softap(void);
esp_mqtt_client_handle_t mqtt_init(void);
esp_err_t mqtt_app_start(esp_mqtt_client_handle_t client);
void mqtt_app_stop(esp_mqtt_client_handle_t client);

void write_recv_flag(uint8_t value);
uint8_t get_recv_flag(void);


#endif