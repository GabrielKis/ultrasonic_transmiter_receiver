#ifndef I2C_DAC_H
#define I2C_DAC_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EXAMPLE_I2S_NUM           (0)
#define EXAMPLE_I2S_SAMPLE_RATE   (1000 * 1000)
#define EXAMPLE_I2S_SAMPLE_BITS   (16)
#define EXAMPLE_I2S_BUF_DEBUG     (0)
#define EXAMPLE_I2S_READ_LEN      (16 * 1024)
#define EXAMPLE_I2S_FORMAT        (I2S_CHANNEL_FMT_RIGHT_LEFT)
#define EXAMPLE_I2S_CHANNEL_NUM   ((EXAMPLE_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))
#define I2S_ADC_UNIT              ADC_UNIT_1
#define I2S_ADC_CHANNEL           ADC1_CHANNEL_0

TaskHandle_t i2c_adc_task_handler;

void config_i2s_adc(void);
void i2c_adc_task(void *arg);


#endif