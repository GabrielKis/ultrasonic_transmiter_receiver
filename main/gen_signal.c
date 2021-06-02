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
    gpio_config_t config_gpio = {
        //TODO: Configurar multiplas portas como output do gpio
        //.pin_bit_mask = GPIO_SEL_23 | GPIO_SEL_22 | GPIO_SEL_1 | GPIO_SEL_3 | GPIO_SEL_21 | GPIO_SEL_19 | GPIO_SEL_18 | GPIO_SEL_5 ,
        .pin_bit_mask = GPIO_SEL_0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config_gpio);
    REG_WRITE(GPIO_ENABLE_REG, BIT2);//Define o GPIO2 como saída
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
    printf("qtd_periods: %d\n", qtd_periods);

    // garantir 5MSPS
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    //  USADO PARA VERIFICAR O TEMPO DE ESCRITA
    time_ptr = &init_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_1, (uint64_t *)time_ptr);

    // iniciar timer
    start_test_timer();

    // loop que repete a quantidade de indices
    //qtd_periods = 40000;
    qtd_periods = 50;
    //for (uint16_t periods_index = 0; periods_index < qtd_periods; periods_index++){
        // cada amostra tem
        for(uint32_t i=0; i<125 * qtd_periods; i++){
            // substituir por funcao que Lê e converte o valor
            REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
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

void write_dac_out_data(uint8_t dac_sample_value){

}

//TODO: MAKE A FUNCTION TO WRITE UINT8_T TO GPIO PORTS
