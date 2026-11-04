#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdint.h>

#define OLED_SCL_PORT   GPIOB
#define OLED_SCL_PIN    GPIO_PIN_8
#define OLED_SDA_PORT   GPIOB
#define OLED_SDA_PIN    GPIO_PIN_9

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

/* 无符号十进制，Length 为位数，前导补 0 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/* 有符号十进制，Length 只算数字位数（不含符号），正数显示 +，负数显示 - */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

/* 十六进制，Length 为数字位数（1~8，不含 0x），负数按 32 位补码
 * ShowPrefix=1 时画 "0x"（额外占 2 列） */
void OLED_ShowHex(uint8_t Line, uint8_t Column, int32_t Number,
                  uint8_t Length, uint8_t ShowPrefix);
/* OLED_hal.h 增加 */
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float f,
                    uint8_t int_len, uint8_t dec_len);
#endif