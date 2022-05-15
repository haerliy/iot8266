#include "sys.h"
#include "usart.h"	
#include <string.h>
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 


#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART1_RXDATA_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART2_RXDATA_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
static uint8_t uart2_rx_len = 0;

static u8 Uart1RxBuffer[Uart1RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���
static u8 Uart2RxBuffer[Uart2RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���

UART_HandleTypeDef UART1_Handler; //UART���
UART_HandleTypeDef UART2_Handler;
//��ʼ��IO ����1 
//bound:������
void uart_init()
{	
	//UART ��ʼ������
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=115200;  				//������
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()��ʹ��UART1
	
	UART2_Handler.Instance = USART2;
	UART2_Handler.Init.BaudRate = 115200;
	UART2_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART2_Handler.Init.StopBits = UART_STOPBITS_1;
	UART2_Handler.Init.Parity = UART_PARITY_NONE;
	UART2_Handler.Init.Mode = UART_MODE_TX_RX;
	UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&UART2_Handler);
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)Uart1RxBuffer, Uart1RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
	HAL_UART_Receive_IT(&UART2_Handler, (u8 *)Uart2RxBuffer, Uart2RXBUFFERSIZE);
}

//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
		__HAL_RCC_AFIO_CLK_ENABLE();
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		GPIO_Initure.Mode=GPIO_MODE_AF_INPUT;	//ģʽҪ����Ϊ��������ģʽ��	
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA10
		
#if EN_USART1_RX
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//��ռ���ȼ�3�������ȼ�3
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
#endif	
	}else if(huart->Instance==USART2){
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_Initure.Pin = GPIO_PIN_2;
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
		GPIO_Initure.Pin = GPIO_PIN_3;
		GPIO_Initure.Mode = GPIO_MODE_INPUT;
		GPIO_Initure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
		HAL_NVIC_SetPriority(USART2_IRQn, 3, 2);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Uart1RxBuffer[0]!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Uart1RxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART1_RXDATA_BUF[USART_RX_STA&0X3FFF]=Uart1RxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
	}else if(huart->Instance==USART2){
		USART2_RXDATA_BUF[uart2_rx_len%200]= Uart2RxBuffer[0];
		uart2_rx_len++;
		HAL_UART_Receive_IT(&UART2_Handler, (uint8_t*)&Uart2RxBuffer[0], 1);
	}
}
 
//����1�жϷ������
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntEnter();    
#endif
	
	HAL_UART_IRQHandler(&UART1_Handler);	//����HAL���жϴ����ú���
	
	timeout=0;
    while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)//�ȴ�����
	{
	 timeout++;////��ʱ����
     if(timeout>HAL_MAX_DELAY) break;		
	
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (u8 *)Uart1RxBuffer, Uart1RXBUFFERSIZE) != HAL_OK)//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
	{
	 timeout++; //��ʱ����
	 if(timeout>HAL_MAX_DELAY) break;	
	}
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntExit();  											 
#endif
} 

void USART2_StartRx(void)
{
	HAL_UART_Receive_IT(&UART2_Handler, (uint8_t*)Uart2RxBuffer, 1);
}

uint8_t USART2_Receive(uint8_t *rx_data)
{
	memcpy(rx_data, (uint8_t*)USART2_RXDATA_BUF, uart2_rx_len);
	return uart2_rx_len;
}

void USART2_Transmit(uint8_t *pdata, uint16_t len, uint16_t tt)
{
    HAL_UART_Transmit(&UART2_Handler, pdata, len, tt);
}

void USART2_ClearBuf(void)
{
    uart2_rx_len = 0;
    memset((uint8_t*)USART2_RXDATA_BUF, 0, 200);
}

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UART2_Handler);
}

#endif	

