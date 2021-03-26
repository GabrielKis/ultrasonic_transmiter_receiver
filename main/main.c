#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include <driver/dac.h>

#include "driver/ledc.h"

#define TIMER_DIVIDER          16
#define TIMER_SCALE            (TIMER_BASE_CLK / TIMER_DIVIDER)     // convert counter value to seconds
#define TIMER_INTERVAL_SEC     (20)                                  // sample test interval for the first timer
#define ALARM_VAL_US           (TIMER_INTERVAL_SEC * TIMER_SCALE)
#define WITH_RELOAD            1

// ledc defines
#define SIGNAL_OUTPUT_FREQ                  40 * 1000   // Fixed 40kHz signal for output
#define LEDC_PWM_SIGNAL_FREQ                1250 * 1000 // 1,25MHz frequency for PWM signal

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

uint32_t alarm_counter = 0;

TaskHandle_t dac_task_handler;
TaskHandle_t watermark_task_handler;
TaskHandle_t signal_output_task_handler;

/* Rotina de interrupcao */
static void IRAM_ATTR timer0_ISR(void *ptr)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    alarm_counter++;
}

/* Inicializacao TIMER_0 - group_0 */
static void config_timer(int timer_idx, bool auto_reload)
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_START,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = auto_reload,
    };

    // Inicia o timer (idx) do grupo 0 com as configs acima
    timer_init(TIMER_GROUP_0, timer_idx, &config);
    // Seta o timer com 0x00000000 (ULL)
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);
    // Configura o alarme
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, ALARM_VAL_US);
    // Habilita interrupcao
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    // Registra a chamada da interrupcao
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer0_ISR, (void *)alarm_counter, ESP_INTR_FLAG_IRAM, NULL);
}

static void dac_gpio_task(void *arg)
{
    double init_time, final_time;
    double * time_ptr;
    while (1) {
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        dac_output_voltage(DAC_CHANNEL_1, 200);
        time_ptr = &final_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("Delta time: %f ms\n", (final_time - init_time)*1000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void water_mark_stack_task(void *arg)
{
    while (1) {
        printf("------WATERMARKS-------\n");
        printf("dac_task_watermark: %d\n", uxTaskGetStackHighWaterMark(dac_task_handler));
        printf("sig_out_watermark: %d\n", uxTaskGetStackHighWaterMark(signal_output_task_handler));
        printf("stack_task_watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void out_signal_task(void *arg)
{
    double init_time, final_time, time_function_time;
    double * time_ptr;
    while (1) {
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 4);
        time_ptr = &final_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        time_ptr = &time_function_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("Delta time PWM: %f us\n", (final_time - init_time - (time_function_time - final_time))*1000*1000);

        ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);
        printf("get_duty: %d\n", ledc_get_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL));
        ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 5);
        ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);
        printf("get_duty: %d\n", ledc_get_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL));
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void config_ledc_pwm(){
    // PWM Timer config
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_6_BIT,    // resolution of PWM duty
        .freq_hz = LEDC_PWM_SIGNAL_FREQ,        // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,             // timer mode
        .timer_num = LEDC_HS_TIMER,             // timer index
        .clk_cfg = LEDC_AUTO_CLK,               // Auto select the source clock
    };
    //TODO: Verify if error check is needed
    esp_err_t err = ledc_timer_config(&ledc_timer);
    ESP_ERROR_CHECK(err);

    // PWM Channel config
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    };
    err = ledc_channel_config(&ledc_channel);
    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    config_timer(TIMER_0, WITH_RELOAD);
    config_ledc_pwm();
    dac_output_enable(DAC_CHANNEL_1);

    //Create and start stats task
//    xTaskCreatePinnedToCore(dac_gpio_task, "dac_gpio", 4096, NULL, 3, dac_task_handler, 0);
    xTaskCreatePinnedToCore(water_mark_stack_task, "stack_wm", 4096, NULL, 0, watermark_task_handler, 0);
//    xTaskCreatePinnedToCore(out_signal_task, "out_sign", 4096, NULL, 0, signal_output_task_handler, 0);
}
