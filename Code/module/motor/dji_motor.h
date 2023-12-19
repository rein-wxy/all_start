#ifndef __DJI_MOTOR_H
#define __DJI_MOTOR_H	 

#include "bsp_can.h"



typedef struct
{
	uint16_t angle;            //ת�ӽǶ� (0~8191)
	int16_t  speed;            //ת���ٶ�
	int16_t  last_speed;			 //�ϴ�ת���ٶ�
	int16_t  real_current;     //�������
	int16_t  temperature;      //����¶� 
	uint16_t last_angle;			 //�ϴ�ת�ӽǶ�
	uint16_t offset_angle;     //�����Ƕ�
	int32_t  round_cnt;        //ת��ת��Ȧ��
	int32_t  total_angle;      //ת��ת���ܽǶ�
	int32_t  last_total_angle;
	int32_t  angle_err;
	int16_t  torque;					 //ת��

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