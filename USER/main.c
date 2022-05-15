#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"

int main(void)
{
	u8 len;	
	u16 times=0;
	
	uint8_t rx_len = 0;                
	uint8_t rx_data[200] = {0};        
	uint8_t tx_data[100] = "AT\r\n";   
  
    HAL_Init();                    	 	//��ʼ��HAL��    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//����ʱ��,72M
	delay_init(72);               		//��ʼ����ʱ����
	uart_init();					//��ʼ������
	LED_Init();							//��ʼ��LED	
	//KEY_Init();							//��ʼ������
	printf("Please enter data, end with Enter \r\n");  
	  
	USART2_StartRx();   
	USART2_Transmit(tx_data, sizeof(tx_data), 500);
    while(1)
    {
		rx_len = USART2_Receive(rx_data);      
		if (rx_len >0)                         
		{
			printf("Receive: %s\n\r", rx_data);
			USART2_ClearBuf();                 
			rx_len = 0;                        
		}

		
       if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			printf("\r\n The message you send is:\r\n");
			HAL_UART_Transmit(&UART1_Handler,(uint8_t*)USART1_RXDATA_BUF,len,1000);	//���ͽ��յ�������
			while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
			printf("\r\n\r\n");//���뻻��
			USART_RX_STA=0;
		}else
		{
			times++;
			if(times%1000==0)
			{
				printf("hello\r\n");
			}
			if(times%30==0)LED0=!LED0;//��˸LED,��ʾϵͳ��������.
			delay_ms(10);   
		} 
    }
}
