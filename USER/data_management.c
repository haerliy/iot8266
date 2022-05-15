#include "data_management.h"
#include "delay.h"
#include "led.h"
#include "usart.h"	
#include <string.h>
void pc_cmd_deal(void){
	u8 len;	
	u16 times=0;
	if(USART_RX_STA&0x8000)
	{					   
		len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
		printf("\r\n The message you send is:\r\n");
		HAL_UART_Transmit(&UART1_Handler,(uint8_t*)USART1_RXDATA_BUF,len,1000);	//发送接收到的数据
		while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);			//等待发送结束
		printf("\r\n\r\n");//插入换行
		if(USART1_RXDATA_BUF[0] == 'A' && USART1_RXDATA_BUF[1] == 'T'){
			USART2_StartRx();   
			USART1_RXDATA_BUF[len] = '\r';
			USART1_RXDATA_BUF[len + 1] = '\n';
			USART2_Transmit(USART1_RXDATA_BUF, len + 2, 500);
		}
		memset((uint8_t*)USART1_RXDATA_BUF, 0, 200);
		USART_RX_STA=0;
	}else
	{
		times++;
		if(times%1000==0)
		{
			printf("hello\r\n");
		}
		if(times%30==0)LED0=!LED0;//闪烁LED,提示系统正在运行.
		delay_ms(10);   
	} 
}

void esp8266_cmd_deal(void){
	uint8_t rx_len = 0;                
	uint8_t rx_data[200] = {0};        
	
	rx_len = USART2_Receive(rx_data);      
	if (rx_len >0)                         
	{
		printf("Receive: %s\n\r", rx_data);
		USART2_ClearBuf();                 
		rx_len = 0;                        
	}
}
