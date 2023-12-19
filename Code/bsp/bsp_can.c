#include "bsp_can.h"




void can_filter_init(void)//ɸѡ������
{
    CAN_FilterTypeDef can_filter_st;
    can_filter_st.FilterActivation = ENABLE;
    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter_st.FilterIdHigh = 0x0000;
    can_filter_st.FilterIdLow = 0x0000;
    can_filter_st.FilterMaskIdHigh = 0x0000;
    can_filter_st.FilterMaskIdLow = 0x0000;
    can_filter_st.FilterBank = 0;
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
    HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);//ʹ���ж�
 
    can_filter_st.SlaveStartFilterBank = 14;
    can_filter_st.FilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan2, &can_filter_st);
    HAL_CAN_Start(&hcan2);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
	
}


/*
	can�жϻص�����

*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

	uint8_t RXData[8];
	CAN_RxHeaderTypeDef RxMessage;							//CANͨѶ�õ�����Ϣ
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxMessage,RXData);
	
	if(hcan->Instance == CAN1)
	{
		switch (RxMessage.StdId){
		case 0x01:
			HT_MOTOR_GET_MESSAGE(&ht_motor[0],RXData); break;
		case 0x02:
			HT_MOTOR_GET_MESSAGE(&ht_motor[1],RXData); break;
		case 0x03:
			HT_MOTOR_GET_MESSAGE(&ht_motor[2],RXData); break;
		case 0x04:
			HT_MOTOR_GET_MESSAGE(&ht_motor[3],RXData); break;
		}
	}
	
	if(hcan->Instance == CAN2){
		switch (RxMessage.StdId){
			
		}
	}	
}





///** @brief �������**//**CAN1**//**power**/

//void SET_POWER_CURRENT_CAN(CAN_HandleTypeDef* hcan)
//{
//	TxMessage.StdId = 0x100;
//  TxMessage.IDE = CAN_ID_STD;
//  TxMessage.RTR = CAN_RTR_DATA;    
//  TxMessage.DLC = 6;
//  TxMessage.TransmitGlobalTime = DISABLE;
//  
////	if(POWER_LIMIT > 110)
////		TxData[1] = (uint8_t)110;
////	else 
////		TxData[1] = (uint8_t)POWER_LIMIT;
////	
////	TxData[2] = (uint8_t)chassis_power_buffer*1.0f;

////	//1ȫ�Զ�2ֻ��粻�ŵ�3����Ҳ����
////	if(task_flag.cap_mode == 0)		TxData[4] = 2;
////	else 							TxData[4] = 1;
//	
//	HAL_CAN_AddTxMessage(&hcan1,&TxMessage,TxData,CAN_FILTER_FIFO0);
//}





