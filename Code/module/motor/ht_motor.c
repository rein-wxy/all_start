#include "ht_motor.h"

#define LIMIT_MIN_MAX(x,min,max) (x) = (((x)<=(min))?(min):(((x)>=(max))?(max):(x)))

ht_motor_data ht_motor[4];
/*
	向电机发送数据包电机才会应答
*/

/**
  * @brief 电机初始化 1_设置控制模式 2_重上电编码器位置置零w
  * @param	
  * @retval none
  * @attention 电机断电不记录位置
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
  *	@brief  海泰电机报文接收处理
  * @param  position: 当前位置（弧度）
  * @param  velocity：电压 0——4096
  * @param  current： 电流 0——4096  -40——40 Amps
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
  * @brief          通过CAN1发送参数数据去控制关节电机
  * @param[in]      buf: 8个字节的数据，包含电机控制参数信息
  * @param[in]      len: buf的长度
  * @param[in]      motor_id: 电机的ID，从0x01到0x04
  * @retval         none
  */
static void CanTransmit(uint8_t *buf, uint8_t len,uint32_t motor_id)
{
    CAN_TxHeaderTypeDef TxHead;             /**!< can通信发送协议头 */
    uint32_t canTxMailbox;
    
    if((buf != NULL) && (len != 0))
    {
		TxHead.StdId    = motor_id;         /* 指定标准标识符，该值在0x01-0x04 */
		TxHead.IDE      = CAN_ID_STD;       /* 指定将要传输消息的标识符类型 */
		TxHead.RTR      = CAN_RTR_DATA;     /* 指定消息传输帧类型 */
		TxHead.DLC      = len;              /* 指定将要传输的帧长度 */
			
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
  * @brief          发送关节电机控制参数(0x01,0x02,0x03,0x04)
  * @param[in]      f_p: 目标位置，范围 [-95.5,95.5] rad
  * @param[in]      f_v: 目标转速，范围 [-45,45] rad/s
  * @param[in]      f_kp: kp参数， 范围 [0，500] N.m/rad
  * @param[in]      f_kd: kd参数,  范围 [0,5] N.m/rad/s
  * @param[in]      f_t: 目标力矩, 范围 [-18,18] N.m
  * @param[in]      motor_id: 电机的ID，从0x01到0x04
  * @retval         none
  */
void HT_MOTOR_SendControlPara(float f_p, float f_v, float f_kp, float f_kd, float f_t,uint32_t motor_id)
{
    uint16_t p, v, kp, kd, t;
    uint8_t buf[8];
    
    /* 限制输入的参数在定义的范围内 */
    LIMIT_MIN_MAX(f_p,  P_MIN,  P_MAX);
    LIMIT_MIN_MAX(f_v,  V_MIN,  V_MAX);
    LIMIT_MIN_MAX(f_kp, KP_MIN, KP_MAX);
    LIMIT_MIN_MAX(f_kd, KD_MIN, KD_MAX);
    LIMIT_MIN_MAX(f_t,  T_MIN,  T_MAX);
    
    /* 根据协议，对float参数进行转换 */
    p = float_to_uint(f_p,   P_MIN,  P_MAX,  16);            
    v = float_to_uint(f_v,   V_MIN,  V_MAX,  12);
    kp = float_to_uint(f_kp, KP_MIN, KP_MAX, 12);
    kd = float_to_uint(f_kd, KD_MIN, KD_MAX, 12);
    t = float_to_uint(f_t,   T_MIN,  T_MAX,  12);
    
    /* 根据传输协议，把数据转换为CAN命令数据字段 */
    buf[0] = p>>8;
    buf[1] = p&0xFF;
    buf[2] = v>>4;
    buf[3] = ((v&0xF)<<4)|(kp>>8);
    buf[4] = kp&0xFF;
    buf[5] = kd>>4;
    buf[6] = ((kd&0xF)<<4)|(t>>8);
    buf[7] = t&0xff;
    
    /* 通过CAN接口把buf中的内容发送出去 */
    CanTransmit(buf, sizeof(buf),motor_id);
}


/**
  * @brief          send mode data to set the control mode of joint motor 
  * @param[in]      cmd: mode command, CMD_MOTOR_MODE(0x05), CMD_RESET_MODE(0x06), CMD_ZERO_POSITION(0x07)
  * @param[in]      motor_id: id of joint motor,0x01~0x04
  * @retval         none
  */
/**
  * @brief          发送模式数据去设置关节电机的控制模式
  * @param[in]      cmd：模式指令，电机模式(0x05)、失效模式(0x06)、零位设置模式(0x07)
  * @param[in]      motor_id: 电机的ID，从0x01到0x04
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
			return; /* 直接退出函数 */
    }
    CanTransmit(buf,sizeof(buf),motor_id);
}

/**
  * @brief          motor stop
  * @param[in]     
  * @retval       
  */
