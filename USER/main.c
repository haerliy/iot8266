#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "data_management.h"

int main(void)
{  
	uint8_t tx_data[100] = "AT\r\n"; 
    HAL_Init();                    	 	//��ʼ��HAL��    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//����ʱ��,72M
	delay_init(72);               		//��ʼ����ʱ����
	uart_init();						//��ʼ������
	LED_Init();							//��ʼ��LED	
	//KEY_Init();						//��ʼ������
	printf("Please enter data, end with Enter \r\n");  
	  
	USART2_StartRx();   
	USART2_Transmit(tx_data, sizeof(tx_data), 500);
    while(1)
    {
		esp8266_cmd_deal();
		pc_cmd_deal();
    }
}
