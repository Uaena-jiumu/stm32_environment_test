#include "usart_transmit.h"





/* 只发字符串 */
 void uart_send_str(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 100);
}

/* 发整数（用于打印气压，不带小数） */
 void uart_send_int(int32_t v)
{
    char b[12];
    int n = 0;
    uint8_t neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    if (v == 0) b[n++] = '0';
    while (v > 0) { b[n++] = '0' + (v % 10); v /= 10; }
    if (neg) b[n++] = '-';
    for (int i = 0; i < n / 2; i++) 
		{
        char t = b[i]; b[i] = b[n-1-i]; b[n-1-i] = t;
    }
    HAL_UART_Transmit(&huart1, (uint8_t*)b, n, 100);
}

/* 发浮点数（保留1位小数，用于温度） */
 void uart_send_float_1(float v)
{
    int32_t i = (int32_t)(v * 10.0f);
    int32_t ip = i / 10;
    int32_t fp = i % 10;
    if (fp < 0) fp = -fp;
    
    uart_send_int(ip);
    uart_send_str(".");
    uart_send_int(fp);
}
