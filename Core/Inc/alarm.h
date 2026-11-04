#ifndef  _key_alarm_H
#define  _key_alarm_H

#include "stdint.h"
#include "stdlib.h"	
#include <math.h>
#include "stm32f1xx_hal.h"
#include "tem_hum_press.h"
#include "key_control.h"



#define BUZZER_PORT  GPIOB
#define BUZZER_PIN	 GPIO_PIN_12



void buzzeron(void);
void buzzeroff(void);
void buzzer_poll(uint8_t alarm_active);
void alarm_check(void);



#endif


