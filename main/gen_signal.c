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
    double init_time, final_time;
    double * time_ptr;
    uint16_t qtd_periods = 0x0000;

    qtd_periods = (data_buffer[0] << 8) + data_buffer[1];
    printf("qtd_periods: %d\n", qtd_periods);

    // dois loops
    // quantidade de iteracoes
    // valores dentro do periodo
    // preencher buffer de resposta com zeros

    // garantir 5MSPS

    //TODO: Receive as param buffer containing um period data
    uint64_t prov = 0;

    // iniciar timer
    start_test_timer();
    start_main_timer();

    for (uint16_t jjj=0; jjj < 10; jjj++){

        //time_ptr = &init_time;
        //timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);

        prov = get_micro_sec_counter();

        printf("buffer received: %d\n", jjj);
        for(int i=2; i<127; i++){
            printf("%d ", data_buffer[i]);
            while(!get_data_ready_flag()){}
            reset_data_ready_flag();
            REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
            //period_signal_buffer[i] = lookup_sine_table[i];
            //printf("look_up_table_value - %d\n", period_signal_buffer[i]);
        }
        //printf("\n");


        //time_ptr = &final_time;
        //timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        //printf("125 samples in: %lf us\n", (final_time - init_time)*10);
    }

    stop_test_timer();
    stop_main_timer();

    // Verificação de taxa da GPIO




    //REG_WRITE(GPIO_ENABLE_REG, BIT2);//Define o GPIO2 como saída
}

void generate_signal_task(void *arg)
{
    uint8_t period_signal_buffer[SAMPLES_PER_PERIOD];
    uint8_t buffer_index;
    buffer_index = 0;
    while(1)
    {
        period_signal_buffer[buffer_index] = lookup_sine_table[buffer_index];
        printf("look_up_table_value - %d\n", period_signal_buffer[buffer_index]);
        printf("value = %d\n", 2000 / portTICK_PERIOD_MS);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

//TODO: MAKE A FUNCTION TO WRITE UINT8_T TO GPIO PORTS
