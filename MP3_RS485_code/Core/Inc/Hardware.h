#include "main.h"


#define	LED_LINK(x)          		(x ? (GPIOC ->BSRR =GPIO_PIN_14) : (GPIOC ->BRR =GPIO_PIN_14) ) 

#define	MP3_PWR(x)          		(x ?   (GPIOA ->BSRR =GPIO_PIN_0) : (GPIOA ->BRR =GPIO_PIN_0) ) 
#define MP3_BUSY     	  				((GPIOA->IDR & GPIO_PIN_1) && 1) 

#define UARTM_TX(x) 		      	(x ? (GPIOA ->BSRR =GPIO_PIN_2) : (GPIOA ->BRR =GPIO_PIN_2) ) 

#define URA_RELAY(x) 		      	(x ? (GPIOA ->BSRR =GPIO_PIN_12) : (GPIOA ->BRR =GPIO_PIN_12) ) 
#define DOOR1_RELAY(x) 		      	(x ? (GPIOA ->BSRR =GPIO_PIN_11) : (GPIOA ->BRR =GPIO_PIN_11) ) 

#define WDT_RESET 	IWDG->KR = 0x0000AAAA


void SystemClock_Config(void);
void MX_GPIO_Init(void);

void TIM17_Init(void);
void TIM14_Init(void);

void USART1_Init(void);
void DMA1_CH1_Init(void);
void SerialPrint(int length);

void USART2_Init(void);
void DMA1_CH3_Init(void);
void SerialSendData(int length);
void DMA1_CH4_Init(void);
	
void ADC_Init(void);
void ShiftRegister_PB_NUM_REL(void);
void Inputs_Debouncer(void);



