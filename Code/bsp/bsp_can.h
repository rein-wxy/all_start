#ifndef __BSP_CAN_H
#define __BSP_CAN_H	 



#include "main.h"
#include "can.h"
#include "stm32f4xx_hal.h"
#include "dji_motor.h"



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
}motor_data;
/*CAN����ֵ�ṹ�幦�ʰ�*/
typedef struct
{
	uint16_t power;
}power_data;


										//CANͨѶ��������Ϣ
extern CAN_RxHeaderTypeDef RxMessage;											//CANͨѶ�õ�����Ϣ
extern CAN_TxHeaderTypeDef TxMessage;											//CANͨѶ��������Ϣ

/*��������*/
void can_filter_init(void);

#endif

