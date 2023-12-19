#ifndef __DJI_MOTOR_H
#define __DJI_MOTOR_H	 

#include "bsp_can.h"



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

}dji_motor_data;


enum status
{
	id_l,    //1-4
	id_h

};


extern dji_motor_data GM6020;


void DJI_MOTOR_GET_MESSAGE(dji_motor_data *ptr,CAN_HandleTypeDef* hcan,uint8_t RXdata[8]);
void DJI_MOTOR_SET_VOLTAGE(CAN_HandleTypeDef* hcan,uint8_t id,int16_t iq1,int16_t iq2,int16_t iq3,int16_t iq4);


#endif