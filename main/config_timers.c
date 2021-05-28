#include "config_timers.h"

uint64_t alarm_counter = 0;

/* Rotina de interrupcao */
void IRAM_ATTR timer0_ISR(void *ptr)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    alarm_counter++;
    // liberar task que seta valores na GPIO - Sinal de saida
}

/* Inicializacao TIMER_0 - group_0 */
void config_timer(int timer_idx, bool auto_reload)
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
