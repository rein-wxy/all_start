#ifndef __BSP_USB_H
#define __BSP_USB_H	 


#include "usbd_cdc_if.h"





/** 
  * 
  * @brief USB���⴮��ʹ��
  * @param	
  * @retval none
  * @attention ������һ��usb�շ�����
			   �жϽ���һ�����ݣ�Ȼ�󽲴�����ԭ·����
	*** ��Ҫ�� MX_USB_DEVICE_Init();��ӽ���ʼ����
	*** ����ͨ���жϽ��մ���
	*** USB���ݽ��մ�����CAN��ͬһ���Ĵ�����ͬʱʹ�û������⣨û�⣩��
*/

/*
	  if(USB_RX_STA != 0)
	  {
		USB_printf("USB_RX");
		  CDC_Transmit_FS(USB_RX_BUF,USB_RX_STA);
		  USB_printf("\r\n");
		  memset(USB_RX_BUF,0,sizeof(USB_RX_BUF));//���������ݼĴ�������
	      USB_RX_STA = 0;
	  }

*/

#endif