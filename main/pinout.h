#ifndef PINOUT_H
#define PINOUT_H

#include "driver/gpio.h"

//#define GPIO_OUTPUT_DAC_0    23
//#define GPIO_OUTPUT_DAC_1    22
////TODO: ALTERAR PARA PINO1 (PERDE O DEBUG)
//#define GPIO_OUTPUT_DAC_2    1
//#define GPIO_OUTPUT_DAC_3    2
//#define GPIO_OUTPUT_DAC_4    21
//#define GPIO_OUTPUT_DAC_5    3
//#define GPIO_OUTPUT_DAC_6    16
//#define GPIO_OUTPUT_DAC_7    4

#define GPIO_OUTPUT_DAC_7    23
#define GPIO_OUTPUT_DAC_6    22
//TODO: ALTERAR PARA PINO1 (PERDE O DEBUG)
#define GPIO_OUTPUT_DAC_5    19
#define GPIO_OUTPUT_DAC_4    2
#define GPIO_OUTPUT_DAC_3    21
#define GPIO_OUTPUT_DAC_2    3
#define GPIO_OUTPUT_DAC_1    16
#define GPIO_OUTPUT_DAC_0    4

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_DAC_0) | (1ULL<<GPIO_OUTPUT_DAC_1) \
                            | (1ULL<<GPIO_OUTPUT_DAC_2) | (1ULL<<GPIO_OUTPUT_DAC_3) \
                            | (1ULL<<GPIO_OUTPUT_DAC_4) | (1ULL<<GPIO_OUTPUT_DAC_5) \
                            | (1ULL<<GPIO_OUTPUT_DAC_6) | (1ULL<<GPIO_OUTPUT_DAC_7))

// GPIO Signal output pins
#define ADC_GPIO_INPUT_D0   36
#define ADC_GPIO_INPUT_D1   39
#define ADC_GPIO_INPUT_D2   34
#define ADC_GPIO_INPUT_D3   35
#define ADC_GPIO_INPUT_D4   32
#define ADC_GPIO_INPUT_D5   33
#define ADC_GPIO_INPUT_D6   25
#define ADC_GPIO_INPUT_D7   26
#define GPIO_INPUT_GPIO_PIN_SEL  ((1ULL<<ADC_GPIO_INPUT_D0) | (1ULL<<ADC_GPIO_INPUT_D1) \
                            | (1ULL<<ADC_GPIO_INPUT_D2) | (1ULL<<ADC_GPIO_INPUT_D3) \
                            | (1ULL<<ADC_GPIO_INPUT_D4) | (1ULL<<ADC_GPIO_INPUT_D5) \
                            | (1ULL<<ADC_GPIO_INPUT_D6) | (1ULL<<ADC_GPIO_INPUT_D7))


#endif