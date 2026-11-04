#ifndef __KEY_CON_H
#define __KEY_CON_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stm32f1xx_hal.h"

/* ================= 按键引脚 ================= */
#define KEY_SWITCH_PIN       GPIO_PIN_4
#define KEY_SWITCH_PORT      GPIOA

#define KEY_ADD_PIN          GPIO_PIN_2
#define KEY_ADD_PORT         GPIOA

#define KEY_SUB_PIN          GPIO_PIN_6
#define KEY_SUB_PORT         GPIOA

#define KEY_INTERFACE_PIN    GPIO_PIN_14
#define KEY_INTERFACE_PORT   GPIOC

/* ================= 超时参数 ================= */
#define KEY_SELECT_TIMEOUT_MS   2000U

/* ================= 报警数据结构 ================= */
typedef struct
{
    float temp;
    float hum;
    float press;
} AlarmDataTypeDef;

/* ================= 界面枚举 ================= */
typedef enum
{
    interface_sensor_date = 0,
    interface_alarm
} InterfaceTypeDef;

/* ================= 修改模式枚举 ================= */
typedef enum
{
    KEY_MODE_NONE = 0,        /* 未选中 */
    KEY_MODE_TEMP_TH,         /* 温度阈值 */
    KEY_MODE_HUM_TH,          /* 湿度阈值 */
    KEY_MODE_PRESS_TH,        /* 气压阈值 */
    KEY_MODE_MAX              /* 仅用于范围判断，不是有效模式 */
} Key_ModeTypeDef;

/* ================= 全局变量声明 ================= */
extern AlarmDataTypeDef AlarmData;
extern Key_ModeTypeDef  key_mode;
extern InterfaceTypeDef Interface;

extern uint32_t key_last_switch_tick;

extern uint8_t key_switch_flag;      /* 模式切换按键标志 */
extern uint8_t key_add_flag;         /* 增加按键标志 */
extern uint8_t key_sub_flag;         /* 减少按键标志 */
extern uint8_t key_interface_change; /* ★ 补上：界面切换按键标志 */

/* ================= 函数声明 ================= */
void KEY_Scan(void);                 /* 按键扫描，主循环调用 */
void InterfaceChange(void);          /* 切换界面并清屏 */
void Alarm_Interface_Refresh(void);  /* ★ 报警界面刷新（含箭头+数值） */
void alarm_check(void);
#endif