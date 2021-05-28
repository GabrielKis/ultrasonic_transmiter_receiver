#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/dac.h"

#include "nvs_flash.h"

// Project files
#include "i2c_dac.h"
#include "gen_signal.h"
#include "config_timers.h"
#include "wifi_mqtt_interface.h"


TaskHandle_t dac_task_handler;
TaskHandle_t watermark_task_handler;
TaskHandle_t main_task_handler;


static void dac_gpio_task(void *arg)
{
    uint32_t bit32_register = 0x00000000;
    esp_err_t err;
    double init_time, final_time;
    double * time_ptr;
    while (1) {
        // Verificação da taxa do DAC
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        for (int i=0; i<1000; i++){
            dac_output_voltage(DAC_CHANNEL_1, 200);
        }
        time_ptr = &final_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("DAC 1000 reads: %lf ms\n", (final_time - init_time)*1000);

        // Verificação de taxa da GPIO
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        for (int i=0; i<1000; i++){
            err = gpio_set_level(GPIO_NUM_0, 1);
        }
        time_ptr = &final_time;
        ESP_ERROR_CHECK(err);
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("GPIO 1000 reads: %lf ms\n", (final_time - init_time)*1000);

        // Verificação de taxa da GPIO
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        for (int i=0; i<1000; i++){
            gpio_get_level(GPIO_NUM_0);
        }
        time_ptr = &final_time;
        ESP_ERROR_CHECK(err);
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("GPIO 1000 writes: %lf ms\n", (final_time - init_time)*1000);
        uint8_t prov_gpio_data = 0b00000010;
        // Verificação de taxa da GPIO
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        for (int i=0; i<1000; i++){
            bit32_register = ((prov_gpio_data & 0b00000001) * BIT23 + \
                                ((prov_gpio_data & 0b00000010) >> 1) * BIT22 + \
                                ((prov_gpio_data & 0b00000100) >> 2) * BIT1 + \
                                ((prov_gpio_data & 0b00001000) >> 3) * BIT3 + \
                                ((prov_gpio_data & 0b00010000) >> 4) * BIT21 + \
                                ((prov_gpio_data & 0b00100000) >> 5) * BIT19 + \
                                ((prov_gpio_data & 0b01000000) >> 6) * BIT18 + \
                                ((prov_gpio_data & 0b10000000) >> 7) * BIT5);
            REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
        }
        time_ptr = &final_time;
        ESP_ERROR_CHECK(err);
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("GPIO 1000 REGISTER SETS: %lf ms\n", (final_time - init_time)*1000);
        printf("REG BUILT: 0x%.8x \n", bit32_register);
        printf("BIT23: 0x%.8x \n\n", BIT23);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void water_mark_stack_task(void *arg)
{
    while (1) {
        printf("------WATERMARKS-------\n");
        printf("dac_task_watermark: %d\n", uxTaskGetStackHighWaterMark(dac_task_handler));
        printf("stack_task_watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void main_task(void *arg)
{
    EventBits_t uxBits;
    uint8_t state_ctrl = 0;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

    while (1) {

        /*state_ctrl
        0 - sem conexao com PC
        1 - com conexao com PC, mas sem conexao com broker
        2 - conexao com broker, esperando para receber comando
        3 - recebeu comando e executa as tasks de coleta e envio de dados
        4 - publica valores das tasks de coleta e envio
        5 - desconecta do broker e volta para o estado 1
        */

        // espera sinal do handler do wifi (que o PC conectou na rede)
        if (state_ctrl == 0){
        uxBits = xEventGroupWaitBits(
                wifi_event_group,   /* The event group being tested. */
                0x03, /* The bits within the event group to wait for. */
                pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */
                if (uxBits == 0x01){
                    state_ctrl = 1;
                }
        }else if (state_ctrl == 1){
            printf("state_ctrl: %d\n", state_ctrl);
            mqtt_app_start(esp_mqtt_client);
            state_ctrl = 2;
        }else if (state_ctrl == 2){
            // espera os dados do receptor
            printf("state_ctrl: %d\n", state_ctrl);
            for(uint8_t i=0; i<DATA_RECV_MQTT_PAYLOAD_SIZE; i++)
                printf("%c", data_received_mqtt[i]);
            printf("\n");
            // interpretar frame recebido
            // qtd_periods
            // period values
            if (data_received_mqtt[4] != 0)
                state_ctrl = 3;
        }else if (state_ctrl == 3){
            printf("state_ctrl: %d\n", state_ctrl);
            printf("CHAMA TASK PARA ENVIO DOS DADOS\n");
            state_ctrl = 4;
            // vai para task de envio dos dados
        }else if (state_ctrl == 4){
            printf("state_ctrl: %d\n", state_ctrl);
            printf("PUBLICA OS VALORES PARA O PYTHON\n");
            state_ctrl = 5;
            // Publica valores dos dados enviados
        }else if (state_ctrl == 5){
            printf("state_ctrl: %d\n", state_ctrl);
            printf("VOLTA PARA O ESTADO 1\n");
            // LIMPA OS VETORES DE RECEBIMENTO E RETORNO
            // state_ctrl = 1;
            // verifica retorno dos dados publicados
            // desconecta do broker
            // volta para estado 1
        }
        // checa estado do broker
            // desconectado -> uxBits = 0x01
        // checa estado do wifi (quantos clientes)
            // desconectado -> uxbits = 0x00

        //broker ja conectado, entra na maquina de estados
        // caso 1:
            // espera ate que topicos "cmd e waveform" sejam recebidos e vai para caso 2
        // caso 2:
            // publica topicos "wf_sent" e "wf_recv" e volta para caso 1
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    //config_timer(TIMER_0, WITH_RELOAD);
    config_timer(0, 1);
    config_gpio();
    // PINO 25 - dac
//    dac_output_enable(DAC_CHANNEL_1);
//    config_i2s_adc();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();

    //esp_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client = mqtt_init();

    //Create and start stats task
//    xTaskCreatePinnedToCore(dac_gpio_task, "dac_gpio", 4096, NULL, 1, dac_task_handler, 0);
//    xTaskCreatePinnedToCore(generate_signal_task, "gen_signal", 4096, NULL, 3, gen_signal_handler, 1);
//    xTaskCreatePinnedToCore(i2c_adc_task, "adc_i2c", 4096, NULL, 1, i2c_adc_task_handler, 0);
//    xTaskCreatePinnedToCore(water_mark_stack_task, "stack_wm", 4096, NULL, 0, watermark_task_handler, 0);
    xTaskCreatePinnedToCore(main_task, "main", 2048, NULL, 1, main_task_handler, 1);
}
