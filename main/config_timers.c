#include "config_timers.h"

uint8_t alarm_counter = 0;
uint64_t microsec_counter = 0;

/* Rotina de interrupcao */
void IRAM_ATTR timer0_ISR(void *ptr)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    alarm_counter = 1;
    // liberar task que seta valores na GPIO - Sinal de saida
}

void IRAM_ATTR timer_teste_ISR(void *ptr)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_1);
    microsec_counter++;
    // liberar task que seta valores na GPIO - Sinal de saida
}

/* Inicializacao TIMER_0 - group_0 */
void config_timer(int timer_idx, bool auto_reload)
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
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

void config_test_timer(int timer_idx, bool auto_reload)
{
    timer_config_t config = {
        .divider = 80,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = auto_reload,
    };

    // Inicia o timer (idx) do grupo 0 com as configs acima
    timer_init(TIMER_GROUP_0, timer_idx, &config);
    // Seta o timer com 0x00000000 (ULL)
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);
    // Configura o alarme
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, 1);
    // Habilita interrupcao
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    // Registra a chamada da interrupcao
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_teste_ISR, (void *)microsec_counter, ESP_INTR_FLAG_IRAM, NULL);
}

uint8_t get_data_ready_flag(void){
    return alarm_counter;
}

void reset_data_ready_flag(void){
    alarm_counter = 0;
}

void start_main_timer(void){
    timer_start(TIMER_GROUP_0, TIMER_0);
}

void stop_main_timer(void){
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

// TIMER DE TESTE
uint64_t get_micro_sec_counter(void){
    return microsec_counter;
}

void start_test_timer(void){
    timer_start(TIMER_GROUP_0, TIMER_1);
}

void stop_test_timer(void){
    timer_pause(TIMER_GROUP_0, TIMER_1);
}
