#ifndef  _USART_TRANSMIT_H
#define  _USART_TRANSMIT_H

#include "stdint.h"
#include "stdlib.h"	
#include <math.h>
#include "stm32f1xx_hal.h"
#include "tem_hum_press.h"
#include "key_control.h"


////////////////////////////////////////如果用printf的话会导致内存超过4k，所以用这和专门来打印字符串，方便调试/////////////////////////////////////////////////


extern UART_HandleTypeDef huart1;




 void uart_send_str(const char *s);
 void uart_send_float_1(float v);
 void uart_send_int(int32_t v);

#endif