#ifndef __HT_MOTOR_H
#define __HT_MOTOR_H	 

#include "main.h"
#include "bsp_can.h"

/**/
#define CMD_MOTOR_MODE      0x05
#define CMD_RESET_MODE      0x06
#define CMD_ZERO_POSITION   0x07


/*HT motor send parameter limit*/
#define P_MIN -95.5f // Radians
#define P_MAX 95.5f
#define V_MIN -45.0f // Rad/s
#define V_MAX 45.0f
#define T_MIN -18.0f // N/m
#define T_MAX 18.0f
#define KP_MIN 0.0f // N-m/rad
#define KP_MAX 500.0f
#define KD_MIN 0.0f // N-m/rad/s
#define KD_MAX 5.0f

/*CAN send and receive ID*/
typedef enum
{
    CAN_HT04_M1_ID = 0x01,
	CAN_HT04_M2_ID = 0x02,
	CAN_HT04_M3_ID = 0x03,
	CAN_HT04_M4_ID = 0x04,
}CAN_ID;


typedef struct 
{
    uint32_t motor_id;
    float position;
    float velocity;
    float current;
   
}ht_motor_data;


extern ht_motor_data ht_motor[4];

void HT_motor_init();

void HT_MOTOR_GET_MESSAGE(ht_motor_data *ptr,uint8_t RXdata[8]);

static void CanTransmit(uint8_t *buf, uint8_t len,uint32_t motor_id);

void HT_MOTOR_SendControlPara(float f_p, float f_v, float f_kp, float f_kd, float f_t,uint32_t motor_id);

void HT_MOTOR_ControlCmd(uint8_t cmd,uint32_t motor_id);

#endif
