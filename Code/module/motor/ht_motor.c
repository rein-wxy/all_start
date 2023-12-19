#include "ht_motor.h"

#define LIMIT_MIN_MAX(x,min,max) (x) = (((x)<=(min))?(min):(((x)>=(max))?(max):(x)))

ht_motor_data ht_motor[4];
/*
	�����������ݰ�����Ż�Ӧ��
*/

/**
  * @brief �����ʼ�� 1_���ÿ���ģʽ 2_���ϵ������λ������w
  * @param	
  * @retval none
  * @attention ����ϵ粻��¼λ��
  */

void HT_motor_init()
{
	for(int i = 1;i <= 4;i++){
		HT_MOTOR_ControlCmd(CMD_MOTOR_MODE,i);
		HAL_Delay(5);
		HT_MOTOR_SendControlPara(0,0,0,0,0,i);
		HT_MOTOR_ControlCmd(CMD_ZERO_POSITION,i);//prudence
		HAL_Delay(5);
	}
}


/**
  * @brief  Converts a float to an unsigned int, given range and number of bits
  * @param
  * @retval 
  */
static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    
    return (uint16_t) ((x-offset)*((float)((1<<bits)-1))/span);
}

/**
  * @brief  converts unsigned int to float, given range and number of bits
  * @param
  * @retval 
  */
static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}

/**
  * @name   HT_MOTOR_GET_MESSAGE
  *	@brief  ��̩������Ľ��մ���
  * @param  position: ��ǰλ�ã����ȣ�
  * @param  velocity����ѹ 0����4096
  * @param  current�� ���� 0����4096  -40����40 Amps
  * @retval none
  * @attention 
  */

void HT_MOTOR_GET_MESSAGE(ht_motor_data *ptr,uint8_t RXdata[8])
{
	ptr->motor_id = RXdata[0];
    ptr->position = uint_to_float(((RXdata[1] << 8 ) | (RXdata[2])),P_MIN,P_MAX,16);      
    ptr->velocity = uint_to_float(((RXdata[3] << 4 ) | (RXdata[4]>>4)),V_MIN,V_MAX,12);   
    ptr->current  = uint_to_float(((RXdata[4] << 4 ) | (RXdata[5])),T_MIN,T_MAX,12);  
	
}


/**
  * @brief          send parameter data from CAN1 to control joint motor
  * @param[in]      buf: 8 bytes data, including motor control parameter information
  * @param[in]      len: size of buf
  * @param[in]      motor_id: id of joint motor,0x01~0x04
  * @retval         none
  */
/**
  * @brief          ͨ��CAN1���Ͳ�������ȥ���ƹؽڵ��
  * @param[in]      buf: 8���ֽڵ����ݣ�����������Ʋ�����Ϣ
  * @param[in]      len: buf�ĳ���
  * @param[in]      motor_id: �����ID����0x01��0x04
  * @retval         none
  */
static void CanTransmit(uint8_t *buf, uint8_t len,uint32_t motor_id)
{
    CAN_TxHeaderTypeDef TxHead;             /**!< canͨ�ŷ���Э��ͷ */
    uint32_t canTxMailbox;
    
    if((buf != NULL) && (len != 0))
    {
		TxHead.StdId    = motor_id;         /* ָ����׼��ʶ������ֵ��0x01-0x04 */
		TxHead.IDE      = CAN_ID_STD;       /* ָ����Ҫ������Ϣ�ı�ʶ������ */
		TxHead.RTR      = CAN_RTR_DATA;     /* ָ����Ϣ����֡���� */
		TxHead.DLC      = len;              /* ָ����Ҫ�����֡���� */
			
		if(HAL_CAN_AddTxMessage(&hcan1, &TxHead, buf, (uint32_t *)&canTxMailbox) == HAL_OK )
		{
		}
    }
}


/**
  * @brief          send control parameters of joint motor (0x01, 0x02, 0x03, 0x04)
  * @param[in]      f_p: position command, range [-95.5,95.5] rad
  * @param[in]      f_v: velocity command, range [-45,45] rad/s
  * @param[in]      f_kp: kp parameter, range [0,500] N.m/rad
  * @param[in]      f_kd: kd parameter, range [0,5] N.m/rad/s
  * @param[in]      f_t:  torque command,range [-18,18] N.m
  * @param[in]      motor_id: id of joint motor,0x01~0x04
  * @retval         none
  */
/**
  * @brief          ���͹ؽڵ�����Ʋ���(0x01,0x02,0x03,0x04)
  * @param[in]      f_p: Ŀ��λ�ã���Χ [-95.5,95.5] rad
  * @param[in]      f_v: Ŀ��ת�٣���Χ [-45,45] rad/s
  * @param[in]      f_kp: kp������ ��Χ [0��500] N.m/rad
  * @param[in]      f_kd: kd����,  ��Χ [0,5] N.m/rad/s
  * @param[in]      f_t: Ŀ������, ��Χ [-18,18] N.m
  * @param[in]      motor_id: �����ID����0x01��0x04
  * @retval         none
  */
void HT_MOTOR_SendControlPara(float f_p, float f_v, float f_kp, float f_kd, float f_t,uint32_t motor_id)
{
    uint16_t p, v, kp, kd, t;
    uint8_t buf[8];
    
    /* ��������Ĳ����ڶ���ķ�Χ�� */
    LIMIT_MIN_MAX(f_p,  P_MIN,  P_MAX);
    LIMIT_MIN_MAX(f_v,  V_MIN,  V_MAX);
    LIMIT_MIN_MAX(f_kp, KP_MIN, KP_MAX);
    LIMIT_MIN_MAX(f_kd, KD_MIN, KD_MAX);
    LIMIT_MIN_MAX(f_t,  T_MIN,  T_MAX);
    
    /* ����Э�飬��float��������ת�� */
    p = float_to_uint(f_p,   P_MIN,  P_MAX,  16);            
    v = float_to_uint(f_v,   V_MIN,  V_MAX,  12);
    kp = float_to_uint(f_kp, KP_MIN, KP_MAX, 12);
    kd = float_to_uint(f_kd, KD_MIN, KD_MAX, 12);
    t = float_to_uint(f_t,   T_MIN,  T_MAX,  12);
    
    /* ���ݴ���Э�飬������ת��ΪCAN���������ֶ� */
    buf[0] = p>>8;
    buf[1] = p&0xFF;
    buf[2] = v>>4;
    buf[3] = ((v&0xF)<<4)|(kp>>8);
    buf[4] = kp&0xFF;
    buf[5] = kd>>4;
    buf[6] = ((kd&0xF)<<4)|(t>>8);
    buf[7] = t&0xff;
    
    /* ͨ��CAN�ӿڰ�buf�е����ݷ��ͳ�ȥ */
    CanTransmit(buf, sizeof(buf),motor_id);
}


/**
  * @brief          send mode data to set the control mode of joint motor 
  * @param[in]      cmd: mode command, CMD_MOTOR_MODE(0x05), CMD_RESET_MODE(0x06), CMD_ZERO_POSITION(0x07)
  * @param[in]      motor_id: id of joint motor,0x01~0x04
  * @retval         none
  */
/**
  * @brief          ����ģʽ����ȥ���ùؽڵ���Ŀ���ģʽ
  * @param[in]      cmd��ģʽָ����ģʽ(0x05)��ʧЧģʽ(0x06)����λ����ģʽ(0x07)
  * @param[in]      motor_id: �����ID����0x01��0x04
  * @retval         none
  */
void HT_MOTOR_ControlCmd(uint8_t cmd,uint32_t motor_id)
{
    uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    switch(cmd)
    {
			case CMD_MOTOR_MODE:
					buf[7] = 0xFC;
					break;
			
			case CMD_RESET_MODE:
					buf[7] = 0xFD;
			break;
			
			case CMD_ZERO_POSITION:
					buf[7] = 0xFE;
			break;
			
			default:
			return; /* ֱ���˳����� */
    }
    CanTransmit(buf,sizeof(buf),motor_id);
}

/**
  * @brief          motor stop
  * @param[in]     
  * @retval       
  */
