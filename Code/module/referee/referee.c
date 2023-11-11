#include "referee.h"

uint8_t uart6_rx_buff[200];								//接收数据缓冲
uint8_t uart6_tx_buff[200];								//发送数据缓冲
uint16_t u6_cmd_data;									//命令码ID
/*总结构体(裁判系统接收数据)*/
Referee_system_data_t Referee_system_data;
/*当前机器人相关ID*/
The_robot_ID_data Robot_ID_Data;



void Referee_Init(void)
{
	HAL_UART_Receive_IT_IDLE(&huart6, uart6_rx_buff, 100);		//裁判系统串口初始化	
}


/*		裁判系统数据格式
		波特率 115200
		数据为 8
		停止位 1
		无校验
*/
/*  	通讯格式  	*/
/* 		 frame_header (5-byte) + cmd_id (2-byte) +  data (n-byte) + frame_tail (2-byte,CRC16,整包校验) 			*/
		
/*		SOF			data_length			seq			CRC8
			1-byte	2-byte					1-byte	1-byte
			0xA5		data长度				包序号	CRC8校验			*/


/*接收数据*/
void cmd_rc_callback_handler(void)
{
	/*数据接收*/
	static uint16_t start_pos=0,next_start_pos=0;//start_pos 起始位
	while(1)
	{
		memcpy(&Referee_system_data.frameheader_t, &uart6_rx_buff[start_pos],5);
		/*校验正确*/
		if((Referee_system_data.frameheader_t.SOF == (uint16_t)0xA5)													//帧头第一字节校验
			&&(Verify_CRC8_Check_Sum(&uart6_rx_buff[start_pos],5) == 1)													//CRC8头校验
			&&(Verify_CRC16_Check_Sum(&uart6_rx_buff[start_pos], Referee_system_data.frameheader_t.DataLength+9)) == 1)		//CRC16整包校验
		{
			memcpy(&Referee_system_data.rxCmdId_rx, (&uart6_rx_buff[start_pos]+5), sizeof(Referee_system_data.rxCmdId_rx));		//将命令码ID复制到相应结构体
			u6_cmd_data = (uint16_t)Referee_system_data.rxCmdId_rx.High << 8 | Referee_system_data.rxCmdId_rx.Low;								//命令码ID
			uart6_rx_buff[start_pos]++;																					//防止重复处理数据
			next_start_pos=start_pos+9+Referee_system_data.frameheader_t.DataLength;									//下一帧数据起始位 = 当前数据起始位+当前数据长度+9-byte(帧头5+命令码2+校验2)
			judgeCalculate(&uart6_rx_buff[start_pos]+7);																//将n-byte数据进行解析
			start_pos=next_start_pos;																					//下一帧数据作为其实新起始位
		}
		/*校验错误*/
		else
		{
			start_pos=0;
			break;
		}
		if(start_pos>200)	//头指针越界
		{
			start_pos=0;
			break;
		}
	}
}



/*解析数据*/
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
			Referee_system_data.extsupply_projectile_action_t.supply_robot_id    = data[1];      //补弹机器人信息
			Referee_system_data.extsupply_projectile_action_t.supply_projectile_step = data[2];  //出弹口状态
			Referee_system_data.extsupply_projectile_action_t.supply_projectile_num = data[3];   //补弹数量
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
			Referee_system_data.extgame_robot_status_t.remain_HP = (uint16_t)data[3]<<8 | data[2];//机器人剩余血量
			Referee_system_data.extgame_robot_status_t.max_HP = (uint16_t)data[5]<<8 | data[4];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_cooling_rate= (uint16_t)data[7]<<8 | data[6];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_cooling_limit= (uint16_t)data[9]<<8 | data[8];
			Referee_system_data.extgame_robot_status_t.shooter_id1_17mm_speed_limit= (uint16_t)data[11]<<8 | data[10];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_cooling_rate= (uint16_t)data[13]<<8 | data[12];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_cooling_limit= (uint16_t)data[15]<<8 | data[14];
			Referee_system_data.extgame_robot_status_t.shooter_id2_17mm_speed_limit= (uint16_t)data[17]<<8 | data[16];
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_cooling_rate= (uint16_t)data[19]<<8 | data[18];//冷却速度
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_cooling_limit= (uint16_t)data[21]<<8 | data[20];//热量上限
			Referee_system_data.extgame_robot_status_t.shooter_id1_42mm_speed_limit= (uint16_t)data[23]<<8 | data[22];//速度上限				 
			Referee_system_data.extgame_robot_status_t.chassis_power_limit = (uint16_t)data[25]<<8 | data[24];//底盘功率上限
			Referee_system_data.extgame_robot_status_t.mains_power_gimbal_output = (data[26] & 0x01);        //电源输出情况         
			Referee_system_data.extgame_robot_status_t.mains_power_chassis_output= (data[27] & 0x02)>>1;
			Referee_system_data.extgame_robot_status_t.mains_power_shooter_output= (data[28] & 0x04)>>2;
		}break;
		case 0x0202:
		{
			Referee_system_data.extpower_heat_data_t.chassis_volt = (uint16_t)data[1]<<8 | data[0];   //mV 底盘输出电压
			Referee_system_data.extpower_heat_data_t.chassis_current=(uint16_t)data[3]<<8 | data[2];  //mA	底盘输出电流
			FT.U[3] = data[7];       //保留小数
			FT.U[2] = data[6];		//
			FT.U[1] = data[5];		//
			FT.U[0] = data[4];      //
			Referee_system_data.extpower_heat_data_t.chassis_power = FT.F;//w(采用联合体转换数据类型) 功率
			Referee_system_data.extpower_heat_data_t.chassis_power_buffer = (uint16_t)data[9]<<8 | data[8];   //j
			Referee_system_data.extpower_heat_data_t.shooter_id1_17mm_cooling_heat= (uint16_t)data[11]<<8 | data[10];
			Referee_system_data.extpower_heat_data_t.shooter_id2_17mm_cooling_heat = (uint16_t)data[13]<<8 | data[12];
			Referee_system_data.extpower_heat_data_t.shooter_id1_42mm_cooling_heat = (uint16_t)data[15]<<8 | data[14];//实时热量
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
			Referee_system_data.extbuff_t.power_rune_buff = data[0];//机器人增益，未拆解
		}break;
		case 0x0205:
		{
			Referee_system_data.aerialrobot_energy_t.attack_time = data[0]; 
		}break;
		case 0x0206:
		{
			Referee_system_data.extrobot_hurt_t.armor_id = data[0] & 0x0f;      //扣血装甲id
			Referee_system_data.extrobot_hurt_t.hurt_type = data[0]>>4 & 0x0f;  //扣血类型
		}break;
		case 0x0207:
		{
			Referee_system_data.extshoot_data_t.bullet_type = data[0];   //弹丸类型
			Referee_system_data.extshoot_data_t.shooter_id = data[1];    //发射机构ID
			Referee_system_data.extshoot_data_t.bullet_freq = data[2];   //射频Hz
			FT.U[3] = data[6];
			FT.U[2] = data[5];
			FT.U[1] = data[4];
			FT.U[0] = data[3];
			Referee_system_data.extshoot_data_t.bullet_speed = FT.F;//弹速
		}break;
		case 0x0208:
		{
			Referee_system_data.extbullet_remaining_t.bullet_remaining_num_17mm = (uint16_t)data[1]<<8 | data[0];  //17mm子弹剩余发射数目
			Referee_system_data.extbullet_remaining_t.bullet_remaining_num_42mm = (uint16_t)data[3]<<8 | data[2];  //42mm子弹剩余发射数目
			Referee_system_data.extbullet_remaining_t.coin_remaining_num = (uint16_t)data[5]<<8 | data[4];         //剩余金币数量
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


/*红蓝阵营判断*/
int Red_or_Blue_judge(void)
{
	if(Referee_system_data.extgame_robot_status_t.robot_id > 100)  //1---101
		return BLUE;
	else
		return RED;
}


/*客户端ID*/
void Robot_client_ID(void)
{
	Robot_ID_Data.robot_ID = Referee_system_data.extgame_robot_status_t.robot_id;
	Robot_ID_Data.robot_colour = Red_or_Blue_judge();
	if(Robot_ID_Data.robot_colour == BLUE)
		Robot_ID_Data.robot_client_ID = 0x0164 + (Robot_ID_Data.robot_ID-100);
	else if(Robot_ID_Data.robot_colour == RED)
		Robot_ID_Data.robot_client_ID = 0x0100 + Robot_ID_Data.robot_ID;
}


/*读取实时功率、实时缓冲能量*/
void get_chassis_power_and_buffer(float *power, float *buffer)
{
   *power = Referee_system_data.extpower_heat_data_t.chassis_power;
   *buffer = Referee_system_data.extpower_heat_data_t.chassis_power_buffer;
}

/*读取机器人ID、最大限制功率、等级*/
void get_chassis_id_maxpower_maxbuffer(uint8_t *ID,float *max_power,uint8_t *robot_level)
{
	*ID = Referee_system_data.extgame_robot_status_t.robot_id;
	*max_power = Referee_system_data.extgame_robot_status_t.chassis_power_limit;
	*robot_level = Referee_system_data.extgame_robot_status_t.robot_level;
}






