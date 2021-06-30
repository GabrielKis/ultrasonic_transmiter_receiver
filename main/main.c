#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

// Project files
#include "gen_signal.h"
#include "config_timers.h"
#include "wifi_mqtt_interface.h"

TaskHandle_t watermark_task_handler;
TaskHandle_t main_task_handler;
TaskHandle_t gen_signal_handler;
TaskHandle_t get_signal_task_handler;

char wf_send_buffer[DAC_SAMPLES_BUF_SIZE];
dac_data_t wf_recv_buffer[DAC_SAMPLES_BUF_SIZE];
char adc_prov_data[1];

static void water_mark_stack_task(void *arg)
{
    while (1) {
        printf("------WATERMARKS-------\n");
        printf("GET_SIGNAL_WM: %d\n", uxTaskGetStackHighWaterMark(get_signal_task_handler));
        printf("GEN_SIGNAL_WM: %d\n", uxTaskGetStackHighWaterMark(gen_signal_handler));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void main_task(void *arg)
{
    EventBits_t uxBits;
    uint8_t state_ctrl = 0;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

    uint8_t publish_flag = 0;
    uint8_t task_control = 0x00;
    uint32_t signals_tasks_notify_ret = 0;

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
            if (mqtt_app_start(esp_mqtt_client) == 0)
                state_ctrl = 2;
        }else if (state_ctrl == 2){
            // espera os dados do receptor
            printf("state_ctrl: %d\n", state_ctrl);
            if (get_recv_flag()){
                printf("frame recebido\n");
                write_recv_flag(0);
                state_ctrl = 3;
            }
        }else if (state_ctrl == 3){
            printf("state_ctrl: %d\n", state_ctrl);
            if (task_control == 0x00){
                printf("CHAMA TASK PARA ENVIO DOS DADOS\n");
                xTaskNotifyGive(gen_signal_handler);
                xTaskNotifyGive(get_signal_task_handler);
                task_control = 1;
            }
            if (task_control == 1){
                signals_tasks_notify_ret += ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
                printf("RECEBEU RETORNO DA TASK DE SINAL: %d\n", signals_tasks_notify_ret);
                if (signals_tasks_notify_ret == 2){
                    state_ctrl = 4;
                    task_control = 0x00;
                    signals_tasks_notify_ret = 0;
                }
            }
        }else if (state_ctrl == 4){
            printf("state_ctrl: %d\n", state_ctrl);
            // Publica valores dos dados enviados
            printf("PUBLICA OS VALORES PARA O PYTHON\n");
            if (esp_mqtt_client_publish(esp_mqtt_client, "ultrasound_send", wf_send_buffer, DAC_SAMPLES_BUF_SIZE, 2, 0) > 0){
                publish_flag += 0x01;
            }
            if (esp_mqtt_client_publish(esp_mqtt_client, "ultrasound_recv", (dac_data_t*) wf_recv_buffer, DAC_SAMPLES_BUF_SIZE, 2, 0) > 0){
                publish_flag += 0x02;
            }
            // Certificar que valores foram publicados antes de alterar o estado
            if (publish_flag == 0x03){
                state_ctrl = 5;
            }
        }else if (state_ctrl == 5){
            printf("state_ctrl: %d\n", state_ctrl);
            // volta para estado 1
            printf("VOLTA PARA O ESTADO 1\n");
            state_ctrl = 1;
            publish_flag = 0x00;
            // desconecta do broker
            mqtt_app_stop(esp_mqtt_client);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void generate_signal_task(void *arg)
{
    uint32_t notify_ret = 0x00000000;
    esp_err_t err;
    while(1)
    {
        // TODO: SUSPENDER ESTA TASK E APENAS ACORDAR QUANDO MAIN TASK MANDAR
        notify_ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("Notify Taken from GEN TASK: %.08x\n", notify_ret);
        if (notify_ret){
            generate_wave(data_received_mqtt, wf_recv_buffer);
            xTaskNotifyGive(main_task_handler);
        }
    }
}

void get_signal_task(void *arg)
{
    uint32_t notify_ret = 0x00000000;
    while(1)
    {
        // TODO: SUSPENDER ESTA TASK E APENAS ACORDAR QUANDO MAIN TASK MANDAR
        notify_ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("Notify Taken from GET TASK: %.08x\n", notify_ret);
        if (notify_ret){
            xTaskNotifyGive(main_task_handler);
        }
    }
}

void app_main(void)
{
    esp_err_t err;
    config_timer(TIMER_0, WITH_RELOAD);
    config_test_timer(1);
    config_gpio();
    err = gpio_set_level(ADC_EO_GPIO, 0);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();

    esp_mqtt_client = mqtt_init();

    //Create and start stats task
    xTaskCreatePinnedToCore(main_task, "main", 2048, NULL, 1, &main_task_handler, 1);
    xTaskCreatePinnedToCore(generate_signal_task, "gen_signal", 8192, NULL, 3, &gen_signal_handler, 1);
    xTaskCreatePinnedToCore(get_signal_task, "get_signal", 8192, NULL, 3, &get_signal_task_handler, 0);
//    xTaskCreatePinnedToCore(water_mark_stack_task, "stack_wm", 4096, NULL, 4, watermark_task_handler, 0);
}
