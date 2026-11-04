//如果I2C正常通信但是BMP280不能获取数据时read me
// uart_send_str("\r\n--- BMP280 Start ---\r\n");
//    uint8_t id = BMP280_ReadID(&hi2c2);
//	///////////I2C通讯的时候要判断ID是否符合一致///////////////////////////////////////
//    if (id != 0x58) {
//        uart_send_str("ID Error!\r\n");
//        while(1);
//    }
//    uart_send_str("ID OK\r\n");
//  //////////// 初始化：读校准参数 + 配置模式///////////////////////////////////////////////
//    if (BMP280_Init(&hi2c2) == 0) {
//        uart_send_str("Init Failed!\r\n");
//        while(1);
//    }
//    uart_send_str("Init OK\r\n");
///////////////////////////////////////////////////////////////////////////////////////
//// 阻塞读取AHT20
//uint8_t AHT20_TestRead(float *temp, float *hum)
//{
//    uint8_t cmd_buf[3] = {0xAC,0x33,0x00};
//    uint8_t recv_buf[6] = {0};
//    char dbg[64];

//    if(HAL_I2C_Master_Transmit(&hi2c2, AHT20_ADDR, cmd_buf, 3, 100) != HAL_OK)
//    {
//        sprintf(dbg,"debug: send 0xAC failed\r\n");
//        HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),50);
//        return 1;
//    }
//    HAL_Delay(80);

//    if(HAL_I2C_Master_Receive(&hi2c2, AHT20_ADDR, recv_buf, 6, 100) != HAL_OK)
//    {
//        sprintf(dbg,"debug: read data failed\r\n");
//        HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),50);
//        return 2;
//    }
//    if(recv_buf[0] & 0x80)
//    {
//        sprintf(dbg,"debug: aht busy\r\n");
//        HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),50);
//        return 3;
//    }

//    uint32_t hum_raw  = ((uint32_t)recv_buf[1] << 12) | ((uint32_t)recv_buf[2] << 4) | (recv_buf[3] >> 4);
//    uint32_t temp_raw = (((uint32_t)recv_buf[3] & 0x0F) << 16) | ((uint32_t)recv_buf[4] << 8) | recv_buf[5];

//    *hum  = hum_raw * 100.0f / 0x100000;
//    *temp = temp_raw * 200.0f / 0x100000 - 50.0f;
//    return 0;
//}



////////////////////////////////* 测试代码，用来看BMP是否正常工作 *////////////////////////////////////////////////////

//// 阻塞读取 BMP280：返回 0 成功，其他失败
//uint8_t BMP280_TestRead(float *temp, float *press)
//{
//    uint8_t recv_buf[6] = {0};
//    char dbg[64];
//    int32_t adc_press, adc_temp;
//    int32_t t_fine_local;
//    uint32_t pa_val;

//    // 1. 读 ID 验证
//    uint8_t id = BMP280_ReadID(&hi2c2);
//    if(id != BMP280_CHIP_ID)
//    {
//        sprintf(dbg,"debug: bmp id=0x%02X, expect 0x58\r\n", id);
//        HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),100);
//        return 1;
//    }

//    // 2. 连续读 0xF7~0xFC 共 6 字节：气压 3 字节 + 温度 3 字节
//    if(HAL_I2C_Mem_Read(&hi2c2, BMP280_ADDR, BMP280_REG_PRESS,
//                        I2C_MEMADD_SIZE_8BIT, recv_buf, 6, 100) != HAL_OK)
//    {
//        sprintf(dbg,"debug: bmp read data failed\r\n");
//        HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),100);
//        return 2;
//    }

//    adc_press = (int32_t)(((uint32_t)recv_buf[0] << 12) | ((uint32_t)recv_buf[1] << 4) | (recv_buf[2] >> 4));
//    adc_temp  = (int32_t)(((uint32_t)recv_buf[3] << 12) | ((uint32_t)recv_buf[4] << 4) | (recv_buf[5] >> 4));

//    // 3. 补偿计算
//    t_fine_local = BMP280_CalcT(adc_temp);
//    pa_val = BMP280_CalcP(adc_press, t_fine_local);

//    // 温度：t_fine 换算成摄氏度
//    *temp = (float)((t_fine_local * 5 + 128) >> 8) / 100.0f;
//    *press = (float)pa_val / 101325.0f;    // 单位 atm，和 DataProcess 保持一致

//    return 0;
//}















//////////////////////////////////////读取传感器的数据（阻塞的方式）用来测试传感器是否有用//////////////////////////////////
//        if (BMP280_TestRead(&t_val, &p_atm) == 0) 
//        {
//            uart_send_str(" C, P=");
//            
//            // 气压 atm 转成 hPa（百帕）：atm * 1013.25
//            int32_t p_hpa = (int32_t)(p_atm * 1013.25f); 
//            uart_send_int(p_hpa);           // 打印气压，如 1012
//            uart_send_str(" hPa\r\n");
//        } 
//        else 
//        {
//            uart_send_str("Read Error\r\n");
//        }
//        HAL_Delay(1000);
//				if (AHT20_TestRead(&t_temp,&t_hum)==0)
//				{
//					uart_send_str("Temp=");
//          uart_send_float_1(t_temp);       // 打印温度，如 25.3
//          uart_send_str(" C, hum=");
//					uart_send_int(t_hum);
//					uart_send_str(" %/r");
//				}
//				else 
//        {
//            uart_send_str("Read Error\r\n");
//        }
//				
/////////////////////////////////////// 