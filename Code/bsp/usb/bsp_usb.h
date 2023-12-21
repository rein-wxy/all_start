#ifndef __BSP_USB_H
#define __BSP_USB_H	 


#include "usbd_cdc_if.h"





/** 
  * 
  * @brief USB虚拟串口使用
  * @param	
  * @retval none
  * @attention 下面是一个usb收发例程
			   中断接收一包数据，然后讲此数据原路发送
	*** 需要将 MX_USB_DEVICE_Init();添加进初始化中
	*** 数据通过中断接收处理
	*** USB数据接收处理与CAN用同一个寄存器（同时使用会有问题（没测））
*/

/*
	  if(USB_RX_STA != 0)
	  {
		USB_printf("USB_RX");
		  CDC_Transmit_FS(USB_RX_BUF,USB_RX_STA);
		  USB_printf("\r\n");
		  memset(USB_RX_BUF,0,sizeof(USB_RX_BUF));//将串口数据寄存器清零
	      USB_RX_STA = 0;
	  }

*/

#endif