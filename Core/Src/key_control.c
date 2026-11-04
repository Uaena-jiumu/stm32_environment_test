#include "key_control.h"
#include "alarm.h"
#include "usart_transmit.h"
#include "tem_hum_press.h"
#include "OLED_hal.h"
#include "OLED_hal_font.h"

AlarmDataTypeDef AlarmData;
InterfaceTypeDef Interface;
Key_ModeTypeDef key_mode = KEY_MODE_NONE;				//初始阶段，未选中任何阈值

/* 新增：记录上次模式按键按下的系统时间戳，用于1s超时判断（非阻塞） */
uint32_t key_last_switch_tick = 0;
uint8_t  key_interface_change =0;


// 3个按键配置
 uint8_t key_switch_flag=0;
 uint8_t key_add_flag=0;
 uint8_t key_sub_flag=0;
 uint8_t key_see=0;

	char *mode ="mode";
	char *add	="add";  /* USER CODE END 2 */
	char *sub	="sub";




void InterfaceChange()
{
	
				if(Interface==interface_sensor_date)
				{
				OLED_ShowString(0,0,"temp");
				OLED_ShowSignedNum(0,50,sensor_data.temp,4);
				OLED_ShowString(1,0,"hum");
				OLED_ShowSignedNum(1,50,sensor_data.hum,4);
				OLED_ShowString(2,0,"press");
				OLED_ShowSignedNum(2,50,sensor_data.press*1000,8);
				}
			
}







/**
 * @brief  按键扫描函数，放在main主循环while(1)里轮询调用
 * @note   外部中断回调仅置标志，业务逻辑全部放在这里，非阻塞架构
 */
void KEY_Scan(void)
{
		// ========== 按键1：切换传感器数据界面和报警信息界面 ==========
		 if(key_interface_change == 1)
    {
			
        key_interface_change = 0;
        HAL_Delay(20);
			if(HAL_GPIO_ReadPin(KEY_INTERFACE_PORT,KEY_INTERFACE_PIN)==GPIO_PIN_SET)
			{
				if(Interface==interface_sensor_date)
				{
					Interface= interface_alarm;
				}
				else 
				{
					Interface=interface_sensor_date;
				}
				OLED_Clear();
			}
			
		}
	if(Interface==interface_alarm)			//只有在报警页面才能修改阈值状态
	{
		// ========== 按键2：切换修改项目（mode切换）按键多选模式 + 等待超时锁定 ==========
    if (key_switch_flag == 1)
{
    key_switch_flag = 0;
    HAL_Delay(20);
    if (HAL_GPIO_ReadPin(KEY_SWITCH_PORT, KEY_SWITCH_PIN) == GPIO_PIN_SET)
    {
        key_mode = (Key_ModeTypeDef)((uint8_t)key_mode + 1);
        if (key_mode >= KEY_MODE_MAX)       /* ★ 用哨兵判断 */
            key_mode = KEY_MODE_NONE;

        key_last_switch_tick = HAL_GetTick();
        /* 不在这里画箭头、不清屏，交给刷新函数 */
    }
}

    

    // ========== 按键3：增加当前选中的阈值 ==========
    if(key_add_flag == 1)
    {
		
        key_add_flag = 0;
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY_ADD_PORT, KEY_ADD_PIN) == GPIO_PIN_SET)
        {
						uart_send_str("add");
					uart_send_str("\r\n");
            switch(key_mode)
            {
                case KEY_MODE_TEMP_TH:
                    AlarmData.temp += 0.5f;
                    if(AlarmData.temp > 80.0f)
                        AlarmData.temp = 80.0f;
                    break;
                case KEY_MODE_HUM_TH:
                    AlarmData.hum += 1.0f;
                    if(AlarmData.hum > 100.0f)
                       AlarmData.hum = 100.0f;
                    break;
                case KEY_MODE_PRESS_TH:
                    AlarmData.press += 0.01f;
                    if(AlarmData.press > 1.2f)
                        AlarmData.press = 1.2f;
                    break;
                case KEY_MODE_NONE:
                default:
                    break;
            }
						key_last_switch_tick = HAL_GetTick();
        }
    }
    // ========== 按键4：减小当前选中的阈值 ==========
    if(key_sub_flag == 1)
    {
			
        key_sub_flag = 0;
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY_SUB_PORT, KEY_SUB_PIN) == GPIO_PIN_SET)
        {
					uart_send_str("sub");
					uart_send_str("\r\n");
            switch(key_mode)
            {
                case KEY_MODE_TEMP_TH:
                    AlarmData.temp -= 0.5f;
                    if(AlarmData.temp < -10.0f)
                        AlarmData.temp = -10.0f;
                    break;
                case KEY_MODE_HUM_TH:
                    AlarmData.hum -= 1.0f;
                    if(AlarmData.hum < 0.0f)
                        AlarmData.hum = 0.0f;
                    break;
                case KEY_MODE_PRESS_TH:
                    AlarmData.press -= 0.01f;
                    if(AlarmData.press < 0.8f)
                       AlarmData.press = 0.8f;
                    break;
                case KEY_MODE_NONE:
                default:
                    break;
            }
						key_last_switch_tick = HAL_GetTick();
        }
				
    }
		/* 新增：超时判断逻辑，2s无按键按下则自动切回NONE，结束选择状态 */
    if(key_add_flag | key_sub_flag | key_switch_flag)
    {
        if( HAL_GetTick() - key_last_switch_tick > KEY_SELECT_TIMEOUT_MS )
        {
            key_mode = KEY_MODE_NONE; //超时未继续按键，退出阈值选择模式
        }
    }
	}
    
	
}

void Alarm_Interface_Refresh(void)
{
    OLED_Clear();   /* 简单起见全清，也可局部清 */

    /* 三行阈值，列 12 起，给箭头留出 0~11 列 */
    OLED_ShowString(0, 12, "T:");
    OLED_ShowFloat (0, 24, AlarmData.temp,  2, 1);

    OLED_ShowString(1, 12, "H:");
    OLED_ShowFloat (1, 24, AlarmData.hum,   3, 1);

    OLED_ShowString(2, 12, "P:");
    OLED_ShowFloat (2, 24, AlarmData.press, 1, 2);

    /* ★ 每帧按 key_mode 画箭头 */
    switch (key_mode)
    {
    case KEY_MODE_TEMP_TH:  OLED_ShowString(0, 0, "->"); break;
    case KEY_MODE_HUM_TH:   OLED_ShowString(1, 0, "->"); break;
    case KEY_MODE_PRESS_TH: OLED_ShowString(2, 0, "->"); break;
    default: break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY_SWITCH_PIN)
    {
        key_switch_flag = 1;
    }
    else if(GPIO_Pin == KEY_ADD_PIN)
    {
        key_add_flag = 1;
        
    }
    else if(GPIO_Pin == KEY_SUB_PIN)
    {
        key_sub_flag = 1;
       
    }
		else if(GPIO_Pin ==KEY_INTERFACE_PIN )
		{
			key_interface_change=1;
		}
}







