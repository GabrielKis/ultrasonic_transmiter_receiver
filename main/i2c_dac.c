#include "i2c_dac.h"
#include "driver/i2s.h"
//#include "driver/adc.h"

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

void i2c_adc_task(void *arg)
{
    double init_time, final_time;
    double * time_ptr;

    while (1) {
        int i2s_read_len = EXAMPLE_I2S_READ_LEN;
        size_t bytes_read;
        /*
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
        */
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
