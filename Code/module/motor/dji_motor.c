#include "dji_motor.h"


uint8_t RXData[8];				//CANͨѶ�õ�����Ϣ����
uint8_t TxData[8]; 				//CANͨѶ��������Ϣ����

dji_motor_data GM6020;

/**
  * @name   DJI_MOTOR_GET_MESSAGE
  *	@brief  dji������Ľ��մ���
  *	@param  
  * @retval none
  * @attention 
  */

void DJI_MOTOR_GET_MESSAGE(dji_motor_data *ptr,CAN_HandleTypeDef* hcan,uint8_t RXdata[8])
{
	ptr->last_angle = ptr->angle;
	ptr->last_speed = ptr->speed;
	ptr->last_total_angle = ptr->total_angle;
	
	ptr->angle = (RXdata[0] << 8)|RXdata[1];
	ptr->speed = (RXdata[2] << 8)|RXdata[3];
	ptr->torque = (RXdata[4] << 8)|RXdata[5];
	ptr->temperature = RXdata[6];
	
	if(ptr->angle - ptr->last_angle > 4096)
		ptr->round_cnt --;
	else if (ptr->angle - ptr->last_angle < -4096)
		ptr->round_cnt ++;
	ptr->total_angle = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle; 				//��������ʲô�ã�����cnt������ ����>��ȷ����
	ptr->angle_err = ptr->last_total_angle - ptr->total_angle;
	
}


/**
  * @name   DJI_MOTOR_SET_VOLTAGE
  *	@brief  dji������͵�ѹ����
  *	@param  hcan: 
			id: ���ID 1-4 -> id_l   5-8 -> id_h 
			iq: ���͵�ѹֵ				
  * @retval none
  * @attention iq��Χ  -30000~ 30000
				GM6020���001->id205
  */

void DJI_MOTOR_SET_VOLTAGE(CAN_HandleTypeDef* hcan,uint8_t id, int16_t iq1, int16_t iq2, int16_t iq3, int16_t iq4)
{
	if(id == id_l)
		TxMessage.StdId = 0x200;	
	else 
		TxMessage.StdId = 0x1FF;
	TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;    
    TxMessage.DLC = 8;
    TxMessage.TransmitGlobalTime = DISABLE;
    
    TxData[0] = (iq1 >> 8)&0xff;
	TxData[1] = (iq1)&0xff;
	TxData[2] = (iq2 >> 8)&0xff;
	TxData[3] = (iq2)&0xff;
	TxData[4] = (iq3 >> 8)&0xff;
	TxData[5] = (iq3)&0xff;
    TxData[6] = (iq4 >> 8)&0xff;
	TxData[7] = (iq4)&0xff;
	
	HAL_CAN_AddTxMessage(hcan,&TxMessage,TxData,CAN_FILTER_FIFO0);

}







