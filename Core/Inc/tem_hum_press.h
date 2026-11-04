#ifndef _THA_HAL
#define _THA_HAL

#include "stdint.h"
#include "stdlib.h"
#include <math.h>
#include "stm32f1xx_hal.h"
#include "string.h"

extern I2C_HandleTypeDef hi2c2;
extern uint8_t i2c_tx_buf[];
extern uint8_t i2c_rx_buf[];

#define AHT20_ADDR              (0x38 << 1)
#define BMP280_ADDR             (0x77 << 1)

#define BMP280_REG_ID           0xD0
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_CALIB        0x88
#define BMP280_REG_CTRL         0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_PRESS        0xF7
#define BMP280_REG_TEMP         0xFA

#define BMP280_CHIP_ID          0x58

typedef enum
{
    I2C_SPACE = 0,
    I2C_TX_CMD,
    I2C_WAIT_TX_CPLT,
    I2C_DELAY_WAIT,
    I2C_WAIT_RX_CPLT,
    I2C_PROCESS_DATA,
} I2C_StateTypeDef;

typedef struct
{
    float temp;
    float hum;
    float press;
    uint8_t new_data_flag;
} SensorDataTypeDef;

typedef enum
{
    SENSOR_NULL = 0,
    SENSOR_READ_AHT20,
    SENSOR_READ_BMP280
} SensorTargetTypeDef;

extern SensorDataTypeDef sensor_data;
extern SensorTargetTypeDef sensor_target;
extern I2C_StateTypeDef i2c_state;

void GetBMP280Value(void);
void GetAHT20Value(void);
void DataProcess(void);
void Sensor_Task(void); 

uint8_t BMP280_ReadID(I2C_HandleTypeDef *hi2c);
uint8_t BMP280_Init(I2C_HandleTypeDef *hi2c);
int32_t BMP280_CalcT(int32_t adc_T);
uint32_t BMP280_CalcP(int32_t adc_P, int32_t t_fine);

uint8_t AHT20_Init(void);

#endif