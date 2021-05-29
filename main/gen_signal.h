#ifndef GEN_SIGNAL_H
#define GEN_SIGNAL_H

#include "driver/gpio.h"
#include "pinout.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SIGNAL_FREQ             40 * 1000   // output signal 40kHz
#define SAMPLE_RATE             1000 * 1000 // Sample rate 1MSpS
#define SAMPLES_PER_PERIOD      25          // 1MSpS/40kHz

#define DAC_SAMPLES_BUF_SIZE    30000

static const uint8_t lookup_sine_table[SAMPLES_PER_PERIOD+1] = {
    0x80,0x9f,0xbd,0xd7,0xeb,0xf9,0xff,0xfd,
    0xf3,0xe2,0xca,0xae,0x8f,0x70,0x51,0x35,
    0x1d,0x0c,0x02,0x00,0x06,0x14,0x28,0x42,
    0x60,0x80
};

TaskHandle_t gen_signal_handler;

void config_gpio(void);
void generate_sine_wave(void);
void generate_signal_task(void *arg);

#endif