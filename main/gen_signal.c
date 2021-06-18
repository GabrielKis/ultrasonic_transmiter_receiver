/**
 *  This file contains functions used on analog signal generation
 *
 *  Gabriel Kis Silvestre
 *
 */

#include "gen_signal.h"
#include "config_timers.h"

#include "driver/ledc.h"
#include "driver/periph_ctrl.h"

/**
 * @brief Config Signal output GPIOs
 *
 * @note The output signal will be generated as an digital signal and converted by hardware
 *
 */
void config_gpio(void)
{
    // Configuracao dos pinos de saida do DAC
    gpio_config_t config_gpio_output = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config_gpio_output);

    // Configuracao dos pinos de entrada do ADC
    gpio_config_t config_gpio_input = {
        .pin_bit_mask = GPIO_INPUT_GPIO_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config_gpio_input);

    // PINO DE CLOCK PARA ADC
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_1_BIT,
        .freq_hz = 4000000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        //.clk_cfg = LEDC_AUTO_CLK,
        .clk_cfg = LEDC_USE_APB_CLK
    };

    ledc_timer_config(&ledc_timer);

    // Set up GPIO PIN
    ledc_channel_config_t channel_config = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 1,
        .gpio_num   = ADC_CLOCK_SIG,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    };

    ledc_channel_config(&channel_config);

/*
*/
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

    printf("Waveform Buffer:\n");
    for (uint index_buf = 0; index_buf<SAMPLES_PER_PERIOD; index_buf++){
        printf("%x ", waveform_buffer[index_buf].dac_sample);
    }
    printf("\n");

    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    time_ptr = &init_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);
    // iniciar timer
    start_test_timer();

    for(uint32_t i=0; i<SAMPLES_PER_PERIOD * qtd_periods; i++){
        bit32_register = 0x00000000;
        sample_value.dac_sample = waveform_buffer[i%SAMPLES_PER_PERIOD].dac_sample;
        bit32_register = ((sample_value.dac_sample_bits.bit0 << GPIO_OUTPUT_DAC_0) + \
                            (sample_value.dac_sample_bits.bit1 << GPIO_OUTPUT_DAC_1)+ \
                            (sample_value.dac_sample_bits.bit2 << GPIO_OUTPUT_DAC_2) + \
                            (sample_value.dac_sample_bits.bit3 << GPIO_OUTPUT_DAC_3) + \
                            (sample_value.dac_sample_bits.bit4 << GPIO_OUTPUT_DAC_4) + \
                            (sample_value.dac_sample_bits.bit5 << GPIO_OUTPUT_DAC_5) + \
                            (sample_value.dac_sample_bits.bit6 << GPIO_OUTPUT_DAC_6) + \
                            (sample_value.dac_sample_bits.bit7 << GPIO_OUTPUT_DAC_7));
//        printf("%d - bit register: %.08x - %.02x \n", (i%SAMPLES_PER_PERIOD), bit32_register, sample_value.dac_sample);
        REG_WRITE(GPIO_OUT_W1TS_REG, (DAC_OUTPUT_REG_MASK & bit32_register));
        REG_WRITE(GPIO_OUT_W1TC_REG, (DAC_OUTPUT_REG_MASK & ~(bit32_register)));
        // Instrução para garantir 40kHz de frequencia
        __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
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
