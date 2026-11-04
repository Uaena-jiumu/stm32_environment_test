#include "tem_hum_press.h"
#include "usart_transmit.h"

extern UART_HandleTypeDef huart1;
SensorDataTypeDef sensor_data = {0};
I2C_StateTypeDef i2c_state = I2C_SPACE;
SensorTargetTypeDef sensor_target = SENSOR_NULL;

#define I2C_BUF_LEN 8
uint8_t i2c_tx_buf[I2C_BUF_LEN];
uint8_t i2c_rx_buf[I2C_BUF_LEN];

static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
static int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;
static int32_t  t_fine;

void GetAHT20Value(void)
{
    if (i2c_state != I2C_SPACE) return;

    sensor_target = SENSOR_READ_AHT20;
    
    i2c_tx_buf[0] = 0xAC;
    i2c_tx_buf[1] = 0x33;
    i2c_tx_buf[2] = 0x00;

    //！！！！！！！！！！ AHT20 没有寄存器，用 Master_Transmit_DMA，切记不能用跟BMP280一样的函数否则会导致AHT20直接强行拉低 SDA 线，导致瘫痪
		//可以在串口打印状态机的转台，如果锁死就要看是回调函数没有写好还是说这个函数写错导致总线瘫痪！！！！！！！！！！！！！！
    if(HAL_I2C_Master_Transmit_DMA(&hi2c2, AHT20_ADDR, i2c_tx_buf, 3) == HAL_OK)
    {
        i2c_state = I2C_TX_CMD; 
    }
    else
    {
        i2c_state = I2C_SPACE;
    }
}	

void GetBMP280Value(void)
{
    if (i2c_state != I2C_SPACE) return;

    sensor_target = SENSOR_READ_BMP280;
    // BMP280 有寄存器，用 Mem_Read_DMA
    if(HAL_I2C_Mem_Read_DMA(&hi2c2, BMP280_ADDR, 0xF7, I2C_MEMADD_SIZE_8BIT, i2c_rx_buf, 6) == HAL_OK)
    {
        i2c_state = I2C_WAIT_RX_CPLT;
    }
    else 
    {
        i2c_state = I2C_SPACE;
    }
}

void Sensor_Task(void) 
{
    static uint32_t wait_start_tick = 0;
    uint32_t delay_time = 0;

    switch(i2c_state) 
    {
        case I2C_WAIT_TX_CPLT:
            wait_start_tick = HAL_GetTick();
            i2c_state = I2C_DELAY_WAIT;
            break;

        case I2C_DELAY_WAIT:
            delay_time = (sensor_target == SENSOR_READ_AHT20) ? 80 : 5;
            if (HAL_GetTick() - wait_start_tick >= delay_time) 
            {
                if(sensor_target == SENSOR_READ_AHT20) {
                    // AHT20 直接接收
                    HAL_I2C_Master_Receive_DMA(&hi2c2, AHT20_ADDR, i2c_rx_buf, 6);
                } else if (sensor_target == SENSOR_READ_BMP280) {
                    // BMP280 读寄存器
                    HAL_I2C_Mem_Read_DMA(&hi2c2, BMP280_ADDR, 0xF7, I2C_MEMADD_SIZE_8BIT, i2c_rx_buf, 6);
                }
                i2c_state = I2C_WAIT_RX_CPLT;
            }
            break;

        case I2C_PROCESS_DATA:
            DataProcess(); 
            break;
            
        default:
            break;
    }
}

void DataProcess(void)
{
    if(i2c_state != I2C_PROCESS_DATA) return;

    if(sensor_target == SENSOR_READ_AHT20)
    {
        uint32_t hum_data, temp_data;
        if( (i2c_rx_buf[0] & 0x80) != 0 )
        {
            i2c_state = I2C_SPACE;
            return;
        }
        hum_data  = ((uint32_t)i2c_rx_buf[1] << 12) | ((uint32_t)i2c_rx_buf[2] << 4) | (i2c_rx_buf[3] >> 4);
        temp_data = ((uint32_t)(i2c_rx_buf[3] & 0x0F) << 16) | ((uint32_t)i2c_rx_buf[4] << 8) | i2c_rx_buf[5];

        sensor_data.hum  = hum_data * 100.0f / 0x100000;
        sensor_data.temp = temp_data * 200.0f / 0x100000 - 50.0f;
    }
    else if(sensor_target == SENSOR_READ_BMP280)
    {
        int32_t adc_press, adc_temp;
        int32_t t_fine_local;
        uint32_t pa_val;

        adc_press = (int32_t)(((uint32_t)i2c_rx_buf[0] << 12) | ((uint32_t)i2c_rx_buf[1] << 4) | (i2c_rx_buf[2] >> 4));
        adc_temp  = (int32_t)(((uint32_t)i2c_rx_buf[3] << 12) | ((uint32_t)i2c_rx_buf[4] << 4) | (i2c_rx_buf[5] >> 4));

        t_fine_local = BMP280_CalcT(adc_temp);
        pa_val = BMP280_CalcP(adc_press, t_fine_local);
        sensor_data.press = (float)pa_val / 101325.0f;  
    }

    sensor_data.new_data_flag = 1;
    i2c_state = I2C_SPACE;
}

uint8_t BMP280_ReadID(I2C_HandleTypeDef *hi2c)
{
    uint8_t id = 0;
    HAL_I2C_Mem_Read(hi2c, BMP280_ADDR, BMP280_REG_ID, I2C_MEMADD_SIZE_8BIT, &id, 1, 100);
    return id;
}

uint8_t BMP280_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t buf[24];
    if (BMP280_ReadID(hi2c) != BMP280_CHIP_ID) return 0;
    
    if (HAL_I2C_Mem_Read(hi2c, BMP280_ADDR, BMP280_REG_CALIB, I2C_MEMADD_SIZE_8BIT, buf, 24, 100) != HAL_OK) return 0;
    
    dig_T1 = (uint16_t)(buf[1] << 8) | buf[0];
    dig_T2 = (int16_t)(buf[3] << 8) | buf[2];
    dig_T3 = (int16_t)(buf[5] << 8) | buf[4];

    dig_P1 = (uint16_t)(buf[7]  << 8) | buf[6];
    dig_P2 = (int16_t)(buf[9]  << 8) | buf[8];
    dig_P3 = (int16_t)(buf[11] << 8) | buf[10];
    dig_P4 = (int16_t)(buf[13] << 8) | buf[12];
    dig_P5 = (int16_t)(buf[15] << 8) | buf[14];
    dig_P6 = (int16_t)(buf[17] << 8) | buf[16];
    dig_P7 = (int16_t)(buf[19] << 8) | buf[18];
    dig_P8 = (int16_t)(buf[21] << 8) | buf[20];
    dig_P9 = (int16_t)(buf[23] << 8) | buf[22];

    uint8_t ctrl = (0x01 << 5) | (0x01 << 2) | 0x03;
    HAL_I2C_Mem_Write(hi2c, BMP280_ADDR, BMP280_REG_CTRL, I2C_MEMADD_SIZE_8BIT, &ctrl, 1, 100);
    return 1;
}

int32_t BMP280_CalcT(int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return t_fine;
}

uint32_t BMP280_CalcP(int32_t adc_P, int32_t t_fine_in)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine_in) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p >> 8);
}

uint8_t AHT20_Init(void)
{
    uint8_t cmd = 0xBA;
    if(HAL_I2C_Master_Transmit(&hi2c2, AHT20_ADDR, &cmd, 1, 100) != HAL_OK) return 1;
    HAL_Delay(20);
    return 0;
}