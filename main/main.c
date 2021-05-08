#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gen_signal.h"

#include "driver/timer.h"
#include "driver/dac.h"

#include "driver/i2s.h"
#include "driver/adc.h"

#include "app_bt.h"

#define TIMER_DIVIDER          16
#define TIMER_SCALE            (TIMER_BASE_CLK / TIMER_DIVIDER)     // convert counter value to seconds
#define TIMER_INTERVAL_SEC     (20)                                  // sample test interval for the first timer
#define ALARM_VAL_US           (TIMER_INTERVAL_SEC * TIMER_SCALE)
#define WITH_RELOAD            1

uint64_t alarm_counter = 0;

TaskHandle_t dac_task_handler;
TaskHandle_t gen_signal_handler;
TaskHandle_t i2c_adc_task_handler;
TaskHandle_t watermark_task_handler;

/* Rotina de interrupcao */
static void IRAM_ATTR timer0_ISR(void *ptr)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    alarm_counter++;
    // liberar task que seta valores na GPIO - Sinal de saida
}

//i2s number
#define EXAMPLE_I2S_NUM           (0)
//i2s sample rate
#define EXAMPLE_I2S_SAMPLE_RATE   (1000 * 1000)
//i2s data bits
#define EXAMPLE_I2S_SAMPLE_BITS   (16)
//enable display buffer for debug
#define EXAMPLE_I2S_BUF_DEBUG     (0)
//I2S read buffer length
#define EXAMPLE_I2S_READ_LEN      (16 * 1024)
//I2S data format
#define EXAMPLE_I2S_FORMAT        (I2S_CHANNEL_FMT_RIGHT_LEFT)
//I2S channel number
#define EXAMPLE_I2S_CHANNEL_NUM   ((EXAMPLE_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))
//I2S built-in ADC unit
#define I2S_ADC_UNIT              ADC_UNIT_1
//I2S built-in ADC channel
#define I2S_ADC_CHANNEL           ADC1_CHANNEL_0

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

void config_i2s_adc(void)
{
    esp_err_t err;
    int i2s_num = EXAMPLE_I2S_NUM;
    i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
    .sample_rate =  EXAMPLE_I2S_SAMPLE_RATE,
    .bits_per_sample = EXAMPLE_I2S_SAMPLE_BITS,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .channel_format = EXAMPLE_I2S_FORMAT,
    .intr_alloc_flags = 0,
    .dma_buf_count = 2,
    .dma_buf_len = 1024,
    .use_apll = 1,
    };
    //install and start i2s driver
    err = i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    ESP_ERROR_CHECK(err);
    //init DAC pad
    //i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    //init ADC pad
    err = i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);
    ESP_ERROR_CHECK(err);
}

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

static void generate_signal_task(void *arg)
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

static void i2c_adc_task(void *arg)
{
    double init_time, final_time;
    double * time_ptr;

    while (1) {
        int i2s_read_len = EXAMPLE_I2S_READ_LEN;
        size_t bytes_read;

        // ADC I2C TEST
        char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));
        i2s_adc_enable(EXAMPLE_I2S_NUM);
        time_ptr = &init_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        i2s_read(EXAMPLE_I2S_NUM, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        time_ptr = &final_time;
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, (double *)time_ptr);
        printf("ADC 1000 writes: %lf ms ! bytes read :%d\n\n", (final_time - init_time)*1000, bytes_read);
        i2s_adc_disable(EXAMPLE_I2S_NUM);
        free(i2s_read_buff);
        i2s_read_buff = NULL;
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


void app_main(void)
{
    config_timer(TIMER_0, WITH_RELOAD);
    config_gpio();
    // PINO 25 - dac
    dac_output_enable(DAC_CHANNEL_1);
    config_i2s_adc();

    bluetooth_init();

    //Create and start stats task
    xTaskCreatePinnedToCore(dac_gpio_task, "dac_gpio", 4096, NULL, 1, dac_task_handler, 0);
    xTaskCreatePinnedToCore(bluetooth_mng_task, "bluetooth", 2048, NULL, 1, bluetooth_handler, 1);
//    xTaskCreatePinnedToCore(generate_signal_task, "gen_signal", 4096, NULL, 3, gen_signal_handler, 1);
//    xTaskCreatePinnedToCore(i2c_adc_task, "adc_i2c", 4096, NULL, 1, i2c_adc_task_handler, 0);
//    xTaskCreatePinnedToCore(water_mark_stack_task, "stack_wm", 4096, NULL, 0, watermark_task_handler, 0);
}
