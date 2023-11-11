#include "referee.h"

uint8_t uart6_rx_buff[200];								//�������ݻ���
uint8_t uart6_tx_buff[200];								//�������ݻ���
uint16_t u6_cmd_data;									//������ID
/*�ܽṹ��(����ϵͳ��������)*/
Referee_system_data_t Referee_system_data;
/*��ǰ���������ID*/
The_robot_ID_data Robot_ID_Data;



void Referee_Init(void)
{
	HAL_UART_Receive_IT_IDLE(&huart6, uart6_rx_buff, 100);		//����ϵͳ���ڳ�ʼ��	
}


/*		����ϵͳ���ݸ�ʽ
		������ 115200
		����Ϊ 8
		ֹͣλ 1
		��У��
*/
/*  	ͨѶ��ʽ  	*/
/* 		 frame_header (5-byte) + cmd_id (2-byte) +  data (n-byte) + frame_tail (2-byte,CRC16,����У��) 			*/
		
/*		SOF			data_length			seq			CRC8
			1-byte	2-byte					1-byte	1-byte
			0xA5		data����				�����	CRC8У��			*/


/*��������*/
void cmd_rc_callback_handler(void)
{
	/*���ݽ���*/
	static uint16_t start_pos=0,next_start_pos=0;//start_pos ��ʼλ
	while(1)
	{
		memcpy(&Referee_system_data.frameheader_t, &uart6_rx_buff[start_pos],5);
		/*У����ȷ*/
		if((Referee_system_data.frameheader_t.SOF == (uint16_t)0xA5)													//֡ͷ��һ�ֽ�У��
			&&(Verify_CRC8_Check_Sum(&uart6_rx_buff[start_pos],5) == 1)													//CRC8ͷУ��
			&&(Verify_CRC16_Check_Sum(&uart6_rx_buff[start_pos], Referee_system_data.frameheader_t.DataLength+9)) == 1)		//CRC16����У��
		{
			memcpy(&Referee_system_data.rxCmdId_rx, (&uart6_rx_buff[start_pos]+5), sizeof(Referee_system_data.rxCmdId_rx));		//��������ID���Ƶ���Ӧ�ṹ��
			u6_cmd_data = (uint16_t)Referee_system_data.rxCmdId_rx.High << 8 | Referee_system_data.rxCmdId_rx.Low;								//������ID
			uart6_rx_buff[start_pos]++;																					//��ֹ�ظ���������
			next_start_pos=start_pos+9+Referee_system_data.frameheader_t.DataLength;									//��һ֡������ʼλ = ��ǰ������ʼλ+��ǰ���ݳ���+9-byte(֡ͷ5+������2+У��2)
			judgeCalculate(&uart6_rx_buff[start_pos]+7);																//��n-byte���ݽ��н���
			start_pos=next_start_pos;																					//��һ֡������Ϊ��ʵ����ʼλ
		}
		/*У�����*/
		else
		{
			start_pos=0;
			break;
		}
		if(start_pos>200)	//ͷָ��Խ��
		{
			start_pos=0;
			break;
		}
	}
}



/*��������*/
void judgeCalculate(uint8_t *data)
{
	FormatTrans FT;
	switch(u6_cmd_data)
	{
		case 0x0001:
		{
			Referee_system_data.extgame_status_t.game_type = data[0] & 0x0f;
			Referee_system_data.extgame_status_t.game_progress = data[0]>>4 & 0x0f;
			Referee_system_data.extgame_status_t.stage_remain_time = (uint16_t) data[2]<<8 | data[1];
		}break;
		case 0x0002:
		{
			Referee_system_data.extgame_result_t.winner = data[0];
		}break;
		case 0x0003:
		{
			Referee_system_data.extgame_robot_HP_t.red_1_robot_HP = (uint16_t)data[1]<<8 | data[0];
			Referee_system_data.extgame_robot_HP_t.red_2_robot_HP = (uint16_t)data[3]<<8 | data[2];
			Referee_system_data.extgame_robot_HP_t.red_3_robot_HP = (uint16_t)data[5]<<8 | data[4];
			Referee_system_data.extgame_robot_HP_t.red_4_robot_HP = (uint16_t)data[7]<<8 | data[6];
			Referee_system_data.extgame_robot_HP_t.red_5_robot_HP = (uint16_t)data[9]<<8 | data[8];
			Referee_system_data.extgame_robot_HP_t.red_7_robot_HP = (uint16_t)data[11]<<8 | data[10];
			Referee_system_data.extgame_robot_HP_t.red_outpost_HP = (uint16_t)data[13]<<8 | data[12];
			Referee_system_data.extgame_robot_HP_t.red_base_HP = (uint16_t)data[15]<<8 | data[14];
			Referee_system_data.extgame_robot_HP_t.blue_1_robot_HP = (uint16_t)data[17]<<8 | data[16];
			Referee_system_data.extgame_robot_HP_t.blue_2_robot_HP = (uint16_t)data[19]<<8 | data[18];
			Referee_system_data.extgame_robot_HP_t.blue_3_robot_HP = (uint16_t)data[21]<<8 | data[20];
			Referee_system_data.extgame_robot_HP_t.blue_4_robot_HP = (uint16_t)data[23]<<8 | data[22];
			Referee_system_data.extgame_robot_HP_t.blue_5_robot_HP = (uint16_t)data[25]<<8 | data[24];
			Referee_system_data.extgame_robot_HP_t.blue_7_robot_HP = (uint16_t)data[27]<<8 | data[26];
			Referee_system_data.extgame_robot_HP_t.blue_outpost_HP = (uint16_t)data[29]<<8 | data[28];
			Referee_system_data.extgame_robot_HP_t.blue_base_HP = (uint16_t)data[31]<<8 | data[30];
		}break;
		case 0x0004:
		{
			Referee_system_data.extdart_status_t.dart_belong = data[0];
			Referee_system_data.extdart_status_t.stage_remaining_time = (uint16_t)data[2]<<8 | data[1];
		}break;
		case 0x0101:
		{
			Referee_system_data.extevent_data_t.event_type = (uint32_t)data[3]<<24 | (uint32_t)data[2]<<16 | (uint32_t)data[1]<<8 | data[0];
		}break;
		case 0x0102:
		{
			Referee_system_data.extsupply_projectile_action_t.supply_projectile_id = data[0];
			Referee_system_data.extsupply_projectile_action_t.supply_robot_id    = data[1];      //������������Ϣ
			Referee_system_data.extsupply_projectile_action_t.supply_projectile_step = data[2];  //������״̬
			Referee_system_data.extsupply_projectile_action_t.supply_projectile_num = data[3];   //��������
		}break;
		case 0x0104:
		{
			Referee_system_data.extreferee_warning_t.level = data[0];
			Referee_system_data.extreferee_warning_t.foul_robot_id = data[0];
		}break;
		case 0x0105:
		{
			Referee_system_data.extdart_remaining_time_t.dart_remaining_time = data[0];
		}break;
		case 0x0201:
		{
			Referee_system_data.extgame_robot_status_t.robot_id = data[0];      //robot id
			Referee_system_data.extgame_robot_status_t.robot_level = data[1];   //robot level
			Referee_system_data.extgame_robot_status_t.remain_HP = (uint16_t)data[3]<<8 | data[2];//������ʣ��Ѫ��
			Referee_system_data.extgame_robot_status_t.max_HP = (uint16_t)data[5]<<8 | data[4];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_cooling_rate= (uint16_t)data[7]<<8 | data[6];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_cooling_limit= (uint16_t)data[9]<<8 | data[8];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_speed_limit= (uint16_t)data[11]<<8 | data[10];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_cooling_rate= (uint16_t)data[13]<<8 | data[12];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_cooling_limit= (uint16_t)data[15]<<8 | data[14];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_speed_limit= (uint16_t)data[17]<<8 | data[16];
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_cooling_rate= (uint16_t)data[19]<<8 | data[18];//��ȴ�ٶ�
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_cooling_limit= (uint16_t)data[21]<<8 | data[20];//��������
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_speed_limit= (uint16_t)data[23]<<8 | data[22];//�ٶ�����				 
			Referee_system_data.extgame_robot_status_t.chassis_power_limit = (uint16_t)data[25]<<8 | data[24];//���̹�������
			Referee_system_data.extgame_robot_status_t.mains_power_gimbal_output = (data[26] & 0x01);        //��Դ������         
			Referee_system_data.extgame_robot_status_t.mains_power_chassis_output= (data[27] & 0x02)>>1;
			Referee_system_data.extgame_robot_status_t.mains_power_shooter_output= (data[28] & 0x04)>>2;
		}break;
		case 0x0202:
		{
			Referee_system_data.extpower_heat_data_t.chassis_volt = (uint16_t)data[1]<<8 | data[0];   //mV ���������ѹ
			Referee_system_data.extpower_heat_data_t.chassis_current=(uint16_t)data[3]<<8 | data[2];  //mA	�����������
			FT.U[3] = data[7];       //����С��
			FT.U[2] = data[6];		//
			FT.U[1] = data[5];		//
			FT.U[0] = data[4];      //
			Referee_system_data.extpower_heat_data_t.chassis_power = FT.F;//w(����������ת����������) ����
			Referee_system_data.extpower_heat_data_t.chassis_power_buffer = (uint16_t)data[9]<<8 | data[8];   //j
			Referee_system_data.extpower_heat_data_t.shooter_id1_17mm_cooling_heat= (uint16_t)data[11]<<8 | data[10];
			Referee_system_data.extpower_heat_data_t.shooter_id2_17mm_cooling_heat = (uint16_t)data[13]<<8 | data[12];
			Referee_system_data.extpower_heat_data_t.shooter_id1_42mm_cooling_heat = (uint16_t)data[15]<<8 | data[14];//ʵʱ����
		}break;
		case 0x0203:
		{
			Referee_system_data.extgame_robot_pos_t.x = (float)((uint32_t)data[3]<<24 | (uint32_t)data[2]<<16 | (uint32_t)data[1]<<8 | data[0]);
			Referee_system_data.extgame_robot_pos_t.y = (float)((uint32_t)data[7]<<24 | (uint32_t)data[6]<<16 | (uint32_t)data[5]<<8 | data[4]);
			Referee_system_data.extgame_robot_pos_t.z = (float)((uint32_t)data[11]<<24 | (uint32_t)data[10]<<16 | (uint32_t)data[9]<<8 | data[8]);
			Referee_system_data.extgame_robot_pos_t.yaw = (float)((uint32_t)data[15]<<24 | (uint32_t)data[14]<<16 | (uint32_t)data[13]<<8 | data[12]);
		}break;
		case 0x0204:
		{
			Referee_system_data.extbuff_t.power_rune_buff = data[0];//���������棬δ���
		}break;
		case 0x0205:
		{
			Referee_system_data.aerialrobot_energy_t.attack_time = data[0]; 
		}break;
		case 0x0206:
		{
			Referee_system_data.extrobot_hurt_t.armor_id = data[0] & 0x0f;      //��Ѫװ��id
			Referee_system_data.extrobot_hurt_t.hurt_type = data[0]>>4 & 0x0f;  //��Ѫ����
		}break;
		case 0x0207:
		{
			Referee_system_data.extshoot_data_t.bullet_type = data[0];   //��������
			Referee_system_data.extshoot_data_t.shooter_id = data[1];    //�������ID
			Referee_system_data.extshoot_data_t.bullet_freq = data[2];   //��ƵHz
			FT.U[3] = data[6];
			FT.U[2] = data[5];
			FT.U[1] = data[4];
			FT.U[0] = data[3];
			Referee_system_data.extshoot_data_t.bullet_speed = FT.F;//����
		}break;
		case 0x0208:
		{
			Referee_system_data.extbullet_remaining_t.bullet_remaining_num_17mm = (uint16_t)data[1]<<8 | data[0];  //17mm�ӵ�ʣ�෢����Ŀ
			Referee_system_data.extbullet_remaining_t.bullet_remaining_num_42mm = (uint16_t)data[3]<<8 | data[2];  //42mm�ӵ�ʣ�෢����Ŀ
			Referee_system_data.extbullet_remaining_t.coin_remaining_num = (uint16_t)data[5]<<8 | data[4];         //ʣ��������
		}break;
		case 0x0209:
		{
			Referee_system_data.extrfid_status_t.rfid_status = data[0];
		}break;
		case 0x020A:
		{
			Referee_system_data.extdart_client_cmd_t.dart_launch_opening_status = data[0];
			Referee_system_data.extdart_client_cmd_t.dart_attack_target = data[1];
			Referee_system_data.extdart_client_cmd_t.target_change_time = (uint16_t)data[3]<<8 | data[2];
			Referee_system_data.extdart_client_cmd_t.first_dart_speed = data[4];
			Referee_system_data.extdart_client_cmd_t.second_dart_speed = data[5];
			Referee_system_data.extdart_client_cmd_t.third_dart_speed = data[6];
			Referee_system_data.extdart_client_cmd_t.fourth_dart_speed = data[7];
			Referee_system_data.extdart_client_cmd_t.last_dart_launch_time = (uint16_t)data[9]<<8 | data[8];
			Referee_system_data.extdart_client_cmd_t.operate_launch_cmd_time = (uint16_t)data[11]<<8 | data[10];
		}break;
		case 0x0301:
		{
			Referee_system_data.extstudent_interactive_header_data_t.data_cmd_id = data[6];
			Referee_system_data.extstudent_interactive_header_data_t.sender_ID = data[7];
			Referee_system_data.extstudent_interactive_header_data_t.receiver_ID = data[8];
		}break;
		case 0x0302:
		{
			
		}break;
		case 0x0303:
		{
			
		}break;
		case 0x0304:
		{
			Referee_system_data.extrobot_command_t_1.mouse_x = (uint16_t)data[1]<<8 | data[0]; 
			Referee_system_data.extrobot_command_t_1.mouse_y = (uint16_t)data[3]<<8 | data[2]; 
			Referee_system_data.extrobot_command_t_1.mouse_z = (uint16_t)data[5]<<8 | data[4]; 
			Referee_system_data.extrobot_command_t_1.left_button_down = data[6];
			Referee_system_data.extrobot_command_t_1.right_button_down = data[7];
			Referee_system_data.extrobot_command_t_1.keyboard_value = (uint16_t)data[9]<<8 | data[8];
			Referee_system_data.extrobot_command_t_1.reserved = (uint16_t)data[11]<<8 | data[10];
		}break;
	}
}


/*������Ӫ�ж�*/
int Red_or_Blue_judge(void)
{
	if(Referee_system_data.extgame_robot_status_t.robot_id > 100)  //1---101
		return BLUE;
	else
		return RED;
}


/*�ͻ���ID*/
void Robot_client_ID(void)
{
	Robot_ID_Data.robot_ID = Referee_system_data.extgame_robot_status_t.robot_id;
	Robot_ID_Data.robot_colour = Red_or_Blue_judge();
	if(Robot_ID_Data.robot_colour == BLUE)
		Robot_ID_Data.robot_client_ID = 0x0164 + (Robot_ID_Data.robot_ID-100);
	else if(Robot_ID_Data.robot_colour == RED)
		Robot_ID_Data.robot_client_ID = 0x0100 + Robot_ID_Data.robot_ID;
}


/*��ȡʵʱ���ʡ�ʵʱ��������*/
void get_chassis_power_and_buffer(float *power, float *buffer)
{
   *power = Referee_system_data.extpower_heat_data_t.chassis_power;
   *buffer = Referee_system_data.extpower_heat_data_t.chassis_power_buffer;
}

/*��ȡ������ID��������ƹ��ʡ��ȼ�*/
void get_chassis_id_maxpower_maxbuffer(uint8_t *ID,float *max_power,uint8_t *robot_level)
{
	*ID = Referee_system_data.extgame_robot_status_t.robot_id;
	*max_power = Referee_system_data.extgame_robot_status_t.chassis_power_limit;
	*robot_level = Referee_system_data.extgame_robot_status_t.robot_level;
}






