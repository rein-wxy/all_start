#ifndef __HT_MOTOR_H
#define __HT_MOTOR_H	 

#include "main.h"

#define HT_MOTOR_CNT 4
#define CURRENT_SMOOTH_COEF 0.9f
#define SPEED_BUFFER_SIZE 5
#define HT_SPEED_BIAS -0.0109901428f // 电机速度偏差,单位rad/s

#define P_MIN -95.5f // Radians
#define P_MAX 95.5f
#define V_MIN -45.0f // Rad/s
#define V_MAX 45.0f
#define T_MIN -18.0f // N·m
#define T_MAX 18.0f
#define KP_MIN 0.0f // N-m/rad
#define KP_MAX 500.0f
#define KD_MIN 0.0f // N-m/rad/s
#define KD_MAX 5.0f




typedef struct // HT04
{
    float total_angle; // 角度为多圈角度,范围是-95.5~95.5,单位为rad
    float last_angle;

    float speed_rads;
    float speed_buff[SPEED_BUFFER_SIZE];

    float real_current;

    float feed_dt;
    uint32_t feed_cnt;
} HTMotor_Measure_t;

/* HT电机类型定义*/
typedef struct
{
    HTMotor_Measure_t measure;

    Motor_Control_Setting_s motor_settings;

    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;
    float pid_ref;

    Motor_Working_Type_e stop_flag; // 启停标志

    CANInstance *motor_can_instace;

    DaemonInstance *motor_daemon;
    uint32_t lost_cnt;
} HTMotorInstance;


/* HT电机模式,初始化时自动进入CMD_MOTOR_MODE*/
typedef enum
{
    CMD_MOTOR_MODE = 0xfc,   // 使能,会响应指令
    CMD_RESET_MODE = 0xfd,   // 停止
    CMD_ZERO_POSITION = 0xfe // 将当前的位置设置为编码器零位
} HTMotor_Mode_t;

















#endif