/**
 *  This file contains functions used on analog signal generation
 *
 *  Gabriel Kis Silvestre
 *
 */

#include "gen_signal.h"
#include "config_timers.h"

#define GPIO_OUTPUT_DAC_0    23
#define GPIO_OUTPUT_DAC_1    22
//TODO: ALTERAR PARA PINO1 (PERDE O DEBUG)
#define GPIO_OUTPUT_DAC_2    3
#define GPIO_OUTPUT_DAC_3    3
#define GPIO_OUTPUT_DAC_4    21
#define GPIO_OUTPUT_DAC_5    19
#define GPIO_OUTPUT_DAC_6    18
#define GPIO_OUTPUT_DAC_7    5
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_DAC_0) | (1ULL<<GPIO_OUTPUT_DAC_1) \
                            | (1ULL<<GPIO_OUTPUT_DAC_2) | (1ULL<<GPIO_OUTPUT_DAC_3) \
                            | (1ULL<<GPIO_OUTPUT_DAC_4) | (1ULL<<GPIO_OUTPUT_DAC_6) \
                            | (1ULL<<GPIO_OUTPUT_DAC_6) | (1ULL<<GPIO_OUTPUT_DAC_7))

/**
 * @brief Config Signal output GPIOs
 *
 * @note The output signal will be generated as an digital signal and converted by hardware
 *
 */
void config_gpio(void)
{
    gpio_config_t config_gpio = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config_gpio);
}

void write_dac_out_data_to_gpio_register(uint8_t dac_sample_value){
    uint32_t bit32_register = 0x00000000;
    bit32_register = ((dac_sample_value & 0b00000001) * BIT23 + \
                        ((dac_sample_value & 0b00000010) >> 1) * BIT22 + \
                        ((dac_sample_value & 0b00000100) >> 2) * BIT1 + \
                        ((dac_sample_value & 0b00001000) >> 3) * BIT3 + \
                        ((dac_sample_value & 0b00010000) >> 4) * BIT21 + \
                        ((dac_sample_value & 0b00100000) >> 5) * BIT19 + \
                        ((dac_sample_value & 0b01000000) >> 6) * BIT18 + \
                        ((dac_sample_value & 0b10000000) >> 7) * BIT5);
    REG_WRITE(GPIO_OUT_W1TC_REG, bit32_register);
}

/**
 * @brief Generate sine wave
 *
 * @note Expand to sine various formats (received via usb)
 *
 */
void generate_wave(uint8_t * data_buffer)
{
    uint64_t init_time, final_time;
    uint64_t * time_ptr;

    uint16_t qtd_periods = 0x0000;

    qtd_periods = (data_buffer[0] << 8) + data_buffer[1];
    qtd_periods = 40000;
    printf("qtd_periods: %d\n", qtd_periods);

    uint8_t sample_value = 0;
    uint8_t waveform_buffer[SAMPLES_PER_PERIOD];
    for (uint8_t index=0; index<100; index++){
        waveform_buffer[index] = data_buffer[index+2];
    }

    // garantir 5MSPS
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    time_ptr = &init_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);

    // iniciar timer
    start_test_timer();

    // loop que repete a quantidade de indices
    //for (uint16_t periods_index = 0; periods_index < qtd_periods; periods_index++){
        // cada amostra tem
    uint32_t bit32_register = 0x00000000;
        for(uint32_t i=0; i<SAMPLES_PER_PERIOD * qtd_periods; i++){
            // substituir por funcao que Lê e converte o valor
            //write_dac_out_data_to_gpio_register(prov_data);
            bit32_register = 0x00000000;
            sample_value = waveform_buffer[0];
            bit32_register = ((sample_value & 0b00000001) * BIT23 + \
                                ((sample_value & 0b00000010) >> 1) * BIT22 + \
                                ((sample_value & 0b00000100) >> 2) * BIT1 + \
                                ((sample_value & 0b00001000) >> 3) * BIT3 + \
                                ((sample_value & 0b00010000) >> 4) * BIT21 + \
                                ((sample_value & 0b00100000) >> 5) * BIT19 + \
                                ((sample_value & 0b01000000) >> 6) * BIT18 + \
                                ((sample_value & 0b10000000) >> 7) * BIT5);
            REG_WRITE(GPIO_OUT_W1TC_REG, bit32_register);
            //REG_WRITE(GPIO_OUT_W1TC_REG, bit32_register);
            //REG_WRITE(GPIO_OUT_W1TC_REG, bit32_register);
            //REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
            //REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
            //REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
            //read_register = REG_READ(GPIO_IN_REG);
        }
    // apos escrever todos os valores de forma de onda, zerar o registrador de saida
    REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);

    //*  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    time_ptr = &final_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    printf("GPIO %d writes: %lld us\n", (qtd_periods * 125), (final_time - init_time));
    //*/

    stop_test_timer();
    //REG_WRITE(GPIO_ENABLE_REG, BIT2);//Define o GPIO2 como saída
}
