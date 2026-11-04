#include "alarm.h"

/* ================= 蜂鸣器底层 ================= */
void buzzeron(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void buzzeroff(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

/* ================= 间歇报警状态机 ================= */
#define BUZZER_ON_MS    500U   /* 响的时长 */
#define BUZZER_OFF_MS   500U   /* 停的时长 */

static uint8_t  buzzer_on_flag = 0;       /* 当前是否在响 */
static uint32_t buzzer_tick    = 0;       /* 上次切换的时间点 */
static uint8_t  buzzer_active  = 0;       /* 是否处于报警激活状态 */

/* 每帧调用一次，参数 1=报警中，0=正常 */
void buzzer_poll(uint8_t alarm_active)
{
    if (!alarm_active)
    {
        /* 恢复正常：立即关，复位状态 */
        if (buzzer_active)
        {
            buzzeroff();
            buzzer_on_flag = 0;
            buzzer_active  = 0;
        }
        return;
    }

    /* 刚进入报警：先响 */
    if (!buzzer_active)
    {
        buzzer_active  = 1;
        buzzer_on_flag = 1;
        buzzer_tick    = HAL_GetTick();
        buzzeron();
        return;
    }

    /* 报警持续中：根据当前状态判断是否该切换 */
    uint32_t elapsed = HAL_GetTick() - buzzer_tick;

    if (buzzer_on_flag)
    {
        /* 正在响，响了够 BUZZER_ON_MS 就停 */
        if (elapsed >= BUZZER_ON_MS)
        {
            buzzeroff();
            buzzer_on_flag = 0;
            buzzer_tick    = HAL_GetTick();
        }
    }
    else
    {
        /* 正在停，停了够 BUZZER_OFF_MS 就响 */
        if (elapsed >= BUZZER_OFF_MS)
        {
            buzzeron();
            buzzer_on_flag = 1;
            buzzer_tick    = HAL_GetTick();
        }
    }
}

/* ================= 报警检测 ================= */
void alarm_check(void)
{
    uint8_t now = 0;

    if (sensor_data.temp  > AlarmData.temp)  now = 1;
    if (sensor_data.hum   > AlarmData.hum)   now = 1;
    if (sensor_data.press < AlarmData.press) now = 1;

    buzzer_poll(now);      /* 每帧都调，传报警状态 */
}