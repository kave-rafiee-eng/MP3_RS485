#include "main.h"
#include "stm32g0xx_it.h"

#include "Definition.h"
#include "Hardware.h"
#include "Serial.h"


///////// MP3 
extern	volatile	_Bool				MP3_Power;
extern	volatile 	uint8_t			RandTrack,PlayerStatus;
extern	volatile	uint16_t		PowerOnDelay,PowerOffDelay,PlayerChecker;

///////// UARTM For MP3
extern	volatile 	uint8_t			UARTM_SendBitStatus,UARTM_Dtr,UARTM_BitShifter,UARTM_StrByte,UARTM_StrLen;
extern	char 									MP3_Str[20];

///////// Timer
extern	volatile	uint8_t			UART_MP3_TX_Time;
extern	volatile 	uint16_t 		Pr,Ms5,Ms20,Task5MsTimer,Task20MsTimer,Task100MsTimer,Task1SecTimer,ChargerStatusTimer;
extern 	volatile	uint8_t			CAB_TX_AfterLastByteTimer;

///////// Serial 
extern 	volatile	uint16_t		PreCabTxDmaCnt;

///////// Hardware
extern	volatile 	_Bool 			ReadInputsShiftF;
extern	volatile  _Bool       HwIn_Rev,HwIn_RevUP,HwIn_RevDN,HwIn_STP,HwIn_OVL,HwIn_DO,HwIn_PHC,HwIn_IN;
extern	volatile 	uint8_t  		InputsShiftStatus;

///////// Battery & Charger
extern	volatile	uint8_t			ChargerStatus;

/////////	Program
extern	volatile	_Bool				StandbyF;


	
void 	TIM14_IRQHandler (void) //Every 1 mS
{
	TIM14->SR =0;

	Pr++;
	Task5MsTimer++;
	Task20MsTimer++;
	Task100MsTimer++;
	Task1SecTimer++;
	
	RandTrack++;
	if(RandTrack > 120) RandTrack = 1;
	
	
	//////////////////////
	////  TX Timers  /////
	//////////////////////
	if(UART_MP3_TX_Time<100) UART_MP3_TX_Time++;
	
	if(CAB_TX_DMA_CNT != PreCabTxDmaCnt) 			{CAB_TX_AfterLastByteTimer = 0;		PreCabTxDmaCnt = CAB_TX_DMA_CNT; }
	else if(CAB_TX_AfterLastByteTimer < 100) 	{CAB_TX_AfterLastByteTimer++;}
	
	
	/////////////////////////////
	////  MP3 Power Control  ////
	/////////////////////////////
	if(StandbyF || PowerOffDelay < 500) 
	{
		if(PowerOffDelay < 500) {PowerOffDelay++;}
		
		MP3_Power = 0; // MP3 Power OFF
	}
	else if(PowerOnDelay < 5000)
	{
		PowerOnDelay ++;
		MP3_Power = 1; // MP3 Power ON
	}
	if(PlayerStatus==3 && PlayerChecker<2000)  PlayerChecker++;
	
	
}

void TIM17_IRQHandler (void) //Every 104 uS
{
  TIM17->SR =0;
	///////////////////////////
	/////  UARTM for MP3  /////
	///////////////////////////	
	if(UARTM_StrLen)
	{
		UART_MP3_TX_Time = 0;
		
		if(UARTM_SendBitStatus > 0)	//Bit Controller
		{
			UARTM_SendBitStatus--;
		} 
		else //String Controller
		{
			if(UARTM_StrLen > UARTM_StrByte)	
			{
				UARTM_Dtr = MP3_Str[UARTM_StrByte];
				UARTM_SendBitStatus = 9;
				
				UARTM_StrByte++;
			}
			else //Sending completed
			{
				UARTM_StrLen = 0;
				UARTM_StrByte = 0;
			}
		}
		
		if(UARTM_SendBitStatus >= 9) //Start Bit
		{
			UARTM_TX(0);
			UARTM_BitShifter = 0x01;
		}
		else if(UARTM_SendBitStatus == 0) //Stop Bit
		{
			UARTM_TX(1);
		}
		else //Data Bits
		{
			UARTM_TX(UARTM_Dtr & UARTM_BitShifter);
			UARTM_BitShifter <<= 1;
		}
	}
	else 
	{
		UARTM_TX(MP3_Power);
	  MP3_PWR(MP3_Power);
	}
	
}

void NMI_Handler(void)
{
   while (1)
  {
  }

}


void HardFault_Handler(void)
{

  while (1)
  {

  }
}


void SVC_Handler(void)
{
}


void PendSV_Handler(void)
{

}


void SysTick_Handler(void)
{

  HAL_IncTick();
}
