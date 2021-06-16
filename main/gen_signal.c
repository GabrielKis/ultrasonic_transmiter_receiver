/**
 *  This file contains functions used on analog signal generation
 *
 *  Gabriel Kis Silvestre
 *
 */

#include "gen_signal.h"
#include "config_timers.h"

/**
 * @brief Config Signal output GPIOs
 *
 * @note The output signal will be generated as an digital signal and converted by hardware
 *
 */
void config_gpio(void)
{
    gpio_config_t config_gpio_output = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config_gpio_output);

//    gpio_config_t config_gpio_input = {
//        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
//        .mode = GPIO_MODE_INPUT,
//        .pull_up_en = GPIO_PULLDOWN_DISABLE,
//        .pull_down_en = GPIO_PULLUP_DISABLE,
//        .intr_type = GPIO_INTR_DISABLE
//    };
    //gpio_config(&config_gpio_input);

    //REG_WRITE(GPIO_OUT_W1TS_REG, DAC_OUTPUT_REG_MASK);

}

/**
 * @brief Generate dac wave
 *
 * @note Write to GPIO 8-bit data
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
    printf("SAMPLE PER PERIOD: %d\n", SAMPLES_PER_PERIOD);
    printf("SAMPLE RATE: %d kSPS\n", SAMPLE_RATE/1000);

    dac_data_t sample_value;
    dac_data_t waveform_buffer[SAMPLES_PER_PERIOD];
    for (uint8_t index=0; index<SAMPLES_PER_PERIOD; index++){
        waveform_buffer[index].dac_sample = data_buffer[index+2];
    }

    uint32_t bit32_register;

    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    time_ptr = &init_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    // iniciar timer
    start_test_timer();

    for(uint32_t i=0; i<SAMPLES_PER_PERIOD * qtd_periods; i++){
        bit32_register = 0x00000000;
        sample_value.dac_sample = waveform_buffer[i%SAMPLES_PER_PERIOD].dac_sample;
        bit32_register = ((sample_value.dac_sample_bits.bit0 << 23) + \
                            (sample_value.dac_sample_bits.bit1 << 22)+ \
                            (sample_value.dac_sample_bits.bit2 << 1) + \
                            (sample_value.dac_sample_bits.bit3 << 3) + \
                            (sample_value.dac_sample_bits.bit4 << 21) + \
                            (sample_value.dac_sample_bits.bit5 << 19) + \
                            (sample_value.dac_sample_bits.bit6 << 18) + \
                            (sample_value.dac_sample_bits.bit7 << 5));
        REG_WRITE(GPIO_OUT_W1TS_REG, (DAC_OUTPUT_REG_MASK & bit32_register));
        REG_WRITE(GPIO_OUT_W1TC_REG, (DAC_OUTPUT_REG_MASK & ~(bit32_register)));
        //read_register = REG_READ(GPIO_IN_REG);
    }
    // apos escrever todos os valores de forma de onda, zerar o registrador de saida
    REG_WRITE(GPIO_OUT_W1TC_REG, DAC_OUTPUT_REG_MASK);

    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    time_ptr = &final_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    printf("GPIO %d writes: %lld us\n", (qtd_periods * SAMPLES_PER_PERIOD), (final_time - init_time));
    stop_test_timer();
}

/**
 * @brief Obtain adc wave (from GPIO)
 *
 * @note Read GPIO data from ADC
 *
 */
void obtain_wave(dac_data_t * recv_buffer)
{
    // CONFIGURAR CLOCK PARA CI
    // VERIFICAR USO DO PINO OE DO CI (GPIO)
    uint64_t init_time, final_time;
    uint64_t * time_ptr;

    dac_data_t sample_value;
    register_32_t read_register_1;
    register_32_t read_register_2;

    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    time_ptr = &init_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    // iniciar timer
    start_test_timer();

    for(uint32_t i=0; i<DAC_SAMPLES_BUF_SIZE; i++){
        read_register_1.register_32 = REG_READ(GPIO_IN_REG);
        read_register_2.register_32 = REG_READ(GPIO_IN1_REG);
        /*
        sample_value.dac_sample = ((read_register_2.register_32_bits.bit4 << 0) + \
                                    (read_register_2.register_32_bits.bit7 << 1) + \
                                    (read_register_2.register_32_bits.bit2 << 2) + \
                                    (read_register_2.register_32_bits.bit3 << 3) + \
                                    (read_register_2.register_32_bits.bit0 << 4) + \
                                    (read_register_2.register_32_bits.bit1 << 5) + \
                                    (read_register_1.register_32_bits.bit25 << 6) + \
                                    (read_register_1.register_32_bits.bit26 << 7));
        recv_buffer[i].dac_sample = sample_value.dac_sample;
        */
    }

    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    time_ptr = &final_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    printf("GPIO %d writes: %lld us\n", (DAC_SAMPLES_BUF_SIZE), (final_time - init_time));
    stop_test_timer();
}
