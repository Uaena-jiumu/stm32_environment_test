#include "OLED_hal.h"
#include "OLED_hal_font.h"      /* ★ 大小写与实际文件名一致 */

#define OLED_W_SCL(x)   HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, \
                                          (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x)   HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, \
                                          (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/* ========== 软件 I2C ========== */
static void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(Byte & (0x80 >> i));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);   /* 应答时钟 */
    OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

/* ========== 初始化 ========== */
void OLED_Init(void)
{
    HAL_Delay(100);

    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0xD5); OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA); OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D); OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);

    OLED_Clear();
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_WriteCommand(0xB0 + j);
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);
        for (i = 0; i < 128; i++)
            OLED_WriteData(0x00);
    }
}

/* ========== 字符 / 字符串 ========== */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    uint8_t c = (uint8_t)Char - ' ';

    /* ★ 可打印字符 0x20~0x7E，索引 0~94，c>94 越界 */
    if (c > 94) return;

    OLED_WriteCommand(0xB0 + Line);
    OLED_WriteCommand(((Column & 0xF0) >> 4) | 0x10);
    OLED_WriteCommand(Column & 0x0F);

    for (i = 0; i < 6; i++)
        OLED_WriteData(F6X8[c][i]);
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
        OLED_ShowChar(Line, Column + i * 6, String[i]);
}

/* ========== 无符号十进制（前导补 0） ========== */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        uint32_t div = 1;
        uint8_t  k;
        for (k = 0; k < (Length - i - 1); k++) div *= 10;
        OLED_ShowChar(Line, Column + i * 6, (char)((Number / div) % 10 + '0'));
    }
}

/* ========== 有符号十进制（Length 不含符号位） ========== */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = (uint32_t)Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        /* ★ 防止 -INT32_MIN 溢出 */
        Number1 = (Number == INT32_MIN) ? 0x80000000U : (uint32_t)(-Number);
    }

    for (i = 0; i < Length; i++)
    {
        uint32_t div = 1;
        uint8_t  k;
        for (k = 0; k < (Length - i - 1); k++) div *= 10;
        OLED_ShowChar(Line, Column + (i + 1) * 6, (char)((Number1 / div) % 10 + '0'));
    }
}

/* ========== 新增：十六进制（Length 不含 0x） ========== */
void OLED_ShowHex(uint8_t Line, uint8_t Column, int32_t Number,
                  uint8_t Length, uint8_t ShowPrefix)
{
    uint8_t  i;
    uint8_t  col = Column;
    uint32_t val = (uint32_t)Number;   /* 负数自动转 32 位补码 */

    if (Length == 0 || Length > 8) Length = 8;

    if (ShowPrefix)
    {
        OLED_ShowChar(Line, col, '0');
        OLED_ShowChar(Line, col + 6, 'x');
        col += 12;
    }

    for (i = 0; i < Length; i++)
    {
        uint8_t shift  = (uint8_t)((Length - 1 - i) * 4);
        uint8_t nibble = (uint8_t)((val >> shift) & 0x0F);
        char    ch     = (nibble < 10) ? (char)('0' + nibble)
                                       : (char)('A' + nibble - 10);
        OLED_ShowChar(Line, col + i * 6, ch);
    }
}
/* OLED_hal.c 增加 */
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float f,
                    uint8_t int_len, uint8_t dec_len)
{
    int32_t  ip;
    uint32_t fp = 0;
    uint8_t  i;
    float    frac;

    ip = (int32_t)f;
    frac = f - (float)ip;
    if (frac < 0.0f) frac = -frac;

    for (i = 0; i < dec_len; i++) frac *= 10.0f;
    fp = (uint32_t)(frac + 0.5f);

    /* 整数部分（带符号） */
    OLED_ShowSignedNum(Line, Column, ip, int_len);

    /* 小数点 */
    OLED_ShowChar(Line, Column + (int_len + 1) * 6, '.');

    /* 小数部分（补前导 0） */
    OLED_ShowNum(Line, Column + (int_len + 2) * 6, fp, dec_len);
}