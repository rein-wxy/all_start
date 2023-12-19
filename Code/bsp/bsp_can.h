#ifndef __BSP_CAN_H
#define __BSP_CAN_H	 



#include "main.h"
#include "can.h"
#include "stm32f4xx_hal.h"
#include "dji_motor.h"
#include "ht_motor.h"


typedef struct
{
	uint16_t angle;            //转子角度 (0~8191)
	int16_t  speed;            //转子速度
	int16_t  last_speed;			 //上次转子速度
	int16_t  real_current;     //电机电流
	int16_t  temperature;      //电机温度 
	uint16_t last_angle;			 //上次转子角度
	uint16_t offset_angle;     //补偿角度
	int32_t  round_cnt;        //转子转动圈数
	int32_t  total_angle;      //转子转动总角度
	int32_t  last_total_angle;
	int32_t  angle_err;
	int16_t  torque;					 //转矩
}motor_data;
/*CAN反馈值结构体功率板*/
typedef struct
{
	uint16_t power;
}power_data;



/*函数声明*/
void can_filter_init(void);

#endif

