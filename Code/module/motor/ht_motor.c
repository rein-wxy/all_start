#include "ht_motor.h"


/**
 * @brief ���õ��ģʽ,��������[0xff,0xff,0xff,0xff,0xff,0xff,0xff,cmd]
 *
 * @param cmd
 * @param motor
 */
static void HTMotorSetMode(HTMotor_Mode_t cmd, HTMotorInstance *motor)
{
    memset(motor->motor_can_instace->tx_buff, 0xff, 7);  // ���͵��ָ���ʱ��ǰ��7bytes����0xff
    motor->motor_can_instace->tx_buff[7] = (uint8_t)cmd; // ���һλ������id
    CANTransmit(motor->motor_can_instace, 1);
    memcpy(motor->motor_can_instace->tx_buff, zero_buff, 6);
}
/* �������ڽ�uintֵ��floatֵ����ӳ��ĺ���,���趨����ֵ�ͽ�������ֵʱʹ�� */
static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (uint16_t)((x - offset) * ((float)((1 << bits) - 1)) / span);
}
static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

/**
 * @brief �����������ֵ
 *
 * @param motor_can �յ�
 */
static void HTMotorDecode(CANInstance *motor_can)
{
    uint16_t tmp; // �����ݴ����ֵ,�Ժ�ת����float����,�����δ�����ʱ����
    uint8_t const *rxbuff = motor_can->rx_buff;
    HTMotorInstance *motor = (HTMotorInstance *)motor_can->id;
    HTMotor_Measure_t *measure = &(motor->measure); // ��canʵ���б����idת���ɵ��ʵ����ָ��

    DaemonReload(motor->motor_daemon);
    measure->feed_dt = DWT_GetDeltaT(&measure->feed_cnt);

    measure->last_angle = measure->total_angle;
    tmp = (uint16_t)((rxbuff[1] << 8) | rxbuff[2]);
    measure->total_angle = uint_to_float(tmp, P_MIN, P_MAX, 16);

    tmp = (uint16_t)(rxbuff[3] << 4) | (rxbuff[4] >> 4);
    measure->speed_rads = AverageFilter((uint_to_float(tmp, V_MIN, V_MAX, 12) - HT_SPEED_BIAS), measure->speed_buff, SPEED_BUFFER_SIZE);

    tmp = (uint16_t)(((rxbuff[4] & 0x0f) << 8) | rxbuff[5]);
    measure->real_current = CURRENT_SMOOTH_COEF * uint_to_float(tmp, T_MIN, T_MAX, 12) +
                            (1 - CURRENT_SMOOTH_COEF) * measure->real_current;
}

static void HTMotorLostCallback(void *motor_ptr)
{
    HTMotorInstance *motor = (HTMotorInstance *)motor_ptr;
    LOGWARNING("[ht_motor] motor %d lost\n", motor->motor_can_instace->tx_id);
    if (++motor->lost_cnt % 10 != 0)
        HTMotorSetMode(CMD_MOTOR_MODE, motor); // ���������õ���������ģʽ
}

/* ��̩���һ����,ʲô����Э��! */
void HTMotorCalibEncoder(HTMotorInstance *motor)
{
    uint16_t p, v, kp, kd, t;
    p = float_to_uint(0, P_MIN, P_MAX, 16);
    v = float_to_uint(0, V_MIN, V_MAX, 12);
    kp = float_to_uint(0, KP_MIN, KP_MAX, 12);
    kd = float_to_uint(0, KD_MIN, KD_MAX, 12);
    t = float_to_uint(0, T_MIN, T_MAX, 12);

    uint8_t *buf = motor->motor_can_instace->tx_buff;
    buf[0] = p >> 8;
    buf[1] = p & 0xFF;
    buf[2] = v >> 4;
    buf[3] = ((v & 0xF) << 4) | (kp >> 8);
    buf[4] = kp & 0xFF;
    buf[5] = kd >> 4;
    buf[6] = ((kd & 0xF) << 4) | (t >> 8);
    buf[7] = t & 0xff;
    memcpy(zero_buff, buf, 6); // ��ʼ����ʱ�����ٵ���һ��,�ʽ�����ָ��Ϊ0ʱ���͵ı��ı���һ��,���ht04���˵��
    CANTransmit(motor->motor_can_instace, 1);
/// DWT_Delay(0.005);
    HTMotorSetMode(CMD_ZERO_POSITION, motor); // sb ����У׼���˱�����Ҳ��Ϊ0
 // DWT_Delay(0.005);
    // HTMotorSetMode(CMD_MOTOR_MODE, motor);
}

HTMotorInstance *HTMotorInit(Motor_Init_Config_s *config)
{
    HTMotorInstance *motor = (HTMotorInstance *)malloc(sizeof(HTMotorInstance));
    memset(motor, 0, sizeof(HTMotorInstance));

    motor->motor_settings = config->controller_setting_init_config;
    PIDInit(&motor->current_PID, &config->controller_param_init_config.current_PID);
    PIDInit(&motor->speed_PID, &config->controller_param_init_config.speed_PID);
    PIDInit(&motor->angle_PID, &config->controller_param_init_config.angle_PID);
    motor->other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    motor->other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;

    config->can_init_config.can_module_callback = HTMotorDecode;
    config->can_init_config.id = motor;
    motor->motor_can_instace = CANRegister(&config->can_init_config);

    Daemon_Init_Config_s conf = {
        .callback = HTMotorLostCallback,
        .owner_id = motor,
        .reload_count = 5, // 20ms
    };
    motor->motor_daemon = DaemonRegister(&conf);

    HTMotorEnable(motor);
    HTMotorSetMode(CMD_MOTOR_MODE, motor); // ȷ������Ѿ��ϵ粢ִ�е��ģʽ
    DWT_Delay(0.05f);
    HTMotorCalibEncoder(motor); // ����ǰ������λ����Ϊ��λ
    DWT_Delay(0.05f);           // ��֤��һ���������ʱCAN�ǿ��е�,ע��Ӧ���ڳ�ʼ��ģ���ʱ��Ӧ�ý����ж�
    ht_motor_instance[idx++] = motor;
    return motor;
}

void HTMotorSetRef(HTMotorInstance *motor, float ref)
{
    motor->pid_ref = ref;
}

/**
 * @brief Ϊ�˱������߶���,Ϊÿ���������һ����������
 * @param argument ����ĵ��ָ��
 */
__attribute__((noreturn)) void HTMotorTask(void const *argument)
{
    float set, pid_measure, pid_ref;
    HTMotorInstance *motor = (HTMotorInstance *)argument;
    HTMotor_Measure_t *measure = &motor->measure;
    Motor_Control_Setting_s *setting = &motor->motor_settings;
    CANInstance *motor_can = motor->motor_can_instace;
    uint16_t tmp;

    while (1)
    {
        pid_ref = motor->pid_ref;
        if ((setting->close_loop_type & ANGLE_LOOP) && setting->outer_loop_type == ANGLE_LOOP)
        {
            if (setting->angle_feedback_source == OTHER_FEED)
                pid_measure = *motor->other_angle_feedback_ptr;
            else
                pid_measure = measure->total_angle;
            // measure��λ��rad,ref�ǽǶ�,ͳһ��angle�¼���,���㽨ģ
            pid_ref = PIDCalculate(&motor->angle_PID, pid_measure * RAD_2_DEGREE, pid_ref);
        }

        if ((setting->close_loop_type & SPEED_LOOP) && setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP))
        {
            if (setting->feedforward_flag & SPEED_FEEDFORWARD)
                pid_ref += *motor->speed_feedforward_ptr;

            if (setting->angle_feedback_source == OTHER_FEED)
                pid_measure = *motor->other_speed_feedback_ptr;
            else
                pid_measure = measure->speed_rads;
            // measure��λ��rad / s ,ref��angle per sec,ͳһ��angle�¼���
            pid_ref = PIDCalculate(&motor->speed_PID, pid_measure * RAD_2_DEGREE, pid_ref);
        }

        if (setting->feedforward_flag & CURRENT_FEEDFORWARD)
            pid_ref += *motor->current_feedforward_ptr;
        if (setting->close_loop_type & CURRENT_LOOP)
        {
            pid_ref = PIDCalculate(&motor->current_PID, measure->real_current, pid_ref);
        }

        set = pid_ref;
        if (setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)
            set *= -1;

        LIMIT_MIN_MAX(set, T_MIN, T_MAX);
        tmp = float_to_uint(set, T_MIN, T_MAX, 12);
        if (motor->stop_flag == MOTOR_STOP)
            tmp = float_to_uint(0, T_MIN, T_MAX, 12);
        motor_can->tx_buff[6] = (tmp >> 8);
        motor_can->tx_buff[7] = tmp & 0xff;

        CANTransmit(motor_can, 0.5);

        osDelay(1);
    }
}

void HTMotorControlInit()
{
    char ht_task_name[5] = "ht";
    // �������е��ʵ��,��������
    if (!idx)
        return;
    for (size_t i = 0; i < idx; i++)
    {
        char ht_id_buff[2] = {0};
        __itoa(i, ht_id_buff, 10);
        strcat(ht_task_name, ht_id_buff); // �ƺ�ûʲô����,osthreaddef��ѵ�һ�������������ַ�������,��Ϊ������
        // @todo ����Ҫһ�������ŵķ��������ֲ�ͬ�ĵ������
        osThreadDef(ht_task_name, HTMotorTask, osPriorityNormal, 0, 128);
        ht_task_handle[i] = osThreadCreate(osThread(ht_task_name), ht_motor_instance[i]);
    }
}

void HTMotorStop(HTMotorInstance *motor)
{
    motor->stop_flag = MOTOR_STOP;
}

void HTMotorEnable(HTMotorInstance *motor)
{
    motor->stop_flag = MOTOR_ENALBED;
}

void HTMotorOuterLoop(HTMotorInstance *motor, Closeloop_Type_e type)
{
    motor->motor_settings.outer_loop_type = type;
}