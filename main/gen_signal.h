#ifndef GEN_SIGNAL_H
#define GEN_SIGNAL_H

#include "driver/gpio.h"
#include "pinout.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SIGNAL_FREQ             40 * 1000                       // output signal 40kHz
#define SAMPLE_RATE             2 * 1000 * 1000                 // Sample rate 4MSpS
#define SAMPLES_PER_PERIOD      25

#define DAC_SAMPLES_BUF_SIZE    10000

// Output bits from register
#define DAC_OUTPUT_REG_MASK     0x00e9001c

// Input bits from GPIO registers
#define ADC_INPUT_REG0_MASK     0x06000000
#define ADC_INPUT_REG1_MASK     0x0000009f

static const uint8_t lookup_sine_table[SAMPLES_PER_PERIOD+1] = {
    0x80,0x9f,0xbd,0xd7,0xeb,0xf9,0xff,0xfd,
    0xf3,0xe2,0xca,0xae,0x8f,0x70,0x51,0x35,
    0x1d,0x0c,0x02,0x00,0x06,0x14,0x28,0x42,
    0x60,0x80
};

typedef union
{
    uint8_t dac_sample;
    struct
    {
        uint8_t bit0 : 1;
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
    } dac_sample_bits;
}dac_data_t;

typedef union
{
    uint32_t register_32;
    struct
    {
        uint32_t bit0 : 1;
        uint32_t bit1 : 1;
        uint32_t bit2 : 1;
        uint32_t bit3 : 1;
        uint32_t bit4 : 1;
        uint32_t bit5 : 1;
        uint32_t bit6 : 1;
        uint32_t bit7 : 1;

        uint32_t bit8 : 1;
        uint32_t bit9 : 1;
        uint32_t bit10 : 1;
        uint32_t bit11 : 1;
        uint32_t bit12 : 1;
        uint32_t bit13 : 1;
        uint32_t bit14 : 1;
        uint32_t bit15 : 1;

        uint32_t bit16 : 1;
        uint32_t bit17 : 1;
        uint32_t bit18 : 1;
        uint32_t bit19 : 1;
        uint32_t bit20 : 1;
        uint32_t bit21 : 1;
        uint32_t bit22 : 1;
        uint32_t bit23 : 1;

        uint32_t bit24 : 1;
        uint32_t bit25 : 1;
        uint32_t bit26 : 1;
        uint32_t bit27 : 1;
        uint32_t bit28 : 1;
        uint32_t bit29 : 1;
        uint32_t bit30 : 1;
        uint32_t bit31 : 1;
    } register_32_bits;
}register_32_t;

void config_gpio(void);
void generate_wave(uint8_t * data_buffer, dac_data_t * recv_buffer);

#endif