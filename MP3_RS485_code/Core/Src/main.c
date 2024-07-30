#include "main.h"
#include "stdio.h"
#include "Definition.h"
#include "Hardware.h"
#include "Serial.h"
#include "MP3.h"
#include "PB_Num.h"

///////// UARTM For MP3
volatile 	uint8_t			UARTM_SendBitStatus=0,UARTM_Dtr=0,UARTM_BitShifter=0,UARTM_StrByte=0,UARTM_StrLen=0;
char 									MP3_Str[20];

///////// MP3 
volatile	_Bool				MP3_SetupF=0,MP3_Power=0;
volatile 	uint8_t			ActivePlayer=0,StopMp3F=0,CMD_MP3File=0,PreMp3File=0,GangPulsTime=0,RandTrack=1,ReplayCounter=0;
volatile 	uint8_t     PlayerList[10],Mp3Listcc=0,Mp3Whilecc=0,PlayerStatus=0,Mp3Folder=0,Mp3Track=0,LatestPlayedMusics[10],LatestMusicCounter=0;
volatile	uint16_t		PowerOnDelay=0,PowerOffDelay=0,PlayerChecker=0;       


///////// Timer
volatile	uint8_t			UART_MP3_TX_Time=0;
volatile 	uint16_t 		Pr=0,Ms5=0,Ms20=0,Task5MsTimer=0,Task20MsTimer=0,Task100MsTimer=0,Task1SecTimer=0,Task1MinuteTimer=0,ChargerStatusTimer=0;
volatile	uint8_t			InDebTimer[8]={0,0,0,0,0,0,0,0},CAB_TX_AfterLastByteTimer=0;
volatile	uint8_t			PHC_Timer=0,DO_Timer=0,PB_BlinkTimer=0;

///////// PB & Num
volatile 	_Bool				CPB[MAX_NOS],L_CPB[MAX_NOS],ForCancelStatus[MAX_NOS],CPB_Cancel[MAX_NOS],PB_BlinkStatus=0;
volatile 	uint8_t 		PB_Time[MAX_NOS],CPB_Clear=NO_AIM,CPB_Set=NO_AIM,CPB_Blinker=NO_AIM;
volatile 	uint8_t			Shitf_PB_Cycle=0,Numrator=0;

///////// Battery & Charger
volatile	_Bool				ChargingF=1;
volatile	uint8_t			ChargerStatus=0,LowBatteryTimer=0,FullBatteryTimer=0;
float 								BatteryVoltage=4.2;

///////// Hardware
volatile 	_Bool 			ReadInputsShiftF=0;
volatile  _Bool       HwIn_Rev=0,HwIn_RevUP=0,HwIn_RevDN=0,HwIn_STP=0,HwIn_OVL=0,HwIn_DO=0,HwIn_PHC=0,HwIn_IN=0;
volatile 	_Bool 			Hw_SegA=0,Hw_SegB=0,Hw_SegC=0,Hw_SegD=0,Hw_SegE=0,Hw_SegF=0,Hw_SegG=0,Hw_SegUP=0,Hw_SegDN=0,Hw_SegL=0;
volatile  _Bool				HwOut_PB[MAX_NOS]={0,0,0,0,0,0},HwIn_PB[MAX_NOS],OE_PB_State=0;
volatile  _Bool				Relay_L5D=0,Relay_L6=0,Relay_URA=0,Relay_D1=0,Relay_D2=0,BUZZ=0,OUT=0;
volatile 	uint8_t  		InputsShiftStatus=0,PowerVoltage=12;
volatile 	uint16_t    adcv[2];


///////// Serial 
char                 	Print_Str[100],CAB_TX_Buffer[CAB_TX_BUFFER_SIZE];
volatile	char				CAB_RX_Buffer[CAB_RX_BUFFER_SIZE];
volatile	_Bool				NewPacketF=0;
volatile	uint8_t			CRXDataCNT=0,PacketCounter=0,SyncStatus=0,PacketLen=0,ReceivedPacket[30],CAB_DataCounter=0,CAB_DataCount=0,LinkLED_Status=0;
volatile	uint16_t		PacketError=0,PreCabTxDmaCnt=0,GetDataTimer=0;


/////////	Program
volatile	_Bool				In_IN=0,In_PHC=0,In_DO=0,In_OVL=0,In_REV=0,In_UP=0,In_DN=0,In_STP=0,T_PHC=0,T_DO=0,SegUp=0,SegDn=0;
volatile	_Bool				Safety_90=0,Safety_71=0,Safety_66=0,Safety_69=0,Safety_68=0,OutStatus=0;
volatile	_Bool				StandbyF=0,CMD_Door1=0,CMD_Door2=0,CMD_URA=0,RunOrStop=0,SegL=0,LandingF=0,WelcomeF=0;
volatile	uint8_t 		E_Level=0,FloorTalk=0,GangSelect=0,MusicVolume=7,TalkVolume=8;

char i=0;

int main(void)
{

  HAL_Init();

  SystemClock_Config();

	MP3_PWR(0);
	MP3_Power = 1;
	UARTM_TX(0);
	
  MX_GPIO_Init();
	TIM17_Init();  		//100us Interrupter For Reading Inputs Shift Registers 
	TIM14_Init();  		//1ms Interrupter
	USART1_Init();		//UART2 For Data Transmition with Main Board
	DMA1_CH3_Init();	//DMA For UART2 TX 
	DMA1_CH4_Init();	//DMA For UART2 RX 
		
	PowerOffDelay=500;
	PowerOnDelay=1000;
	StopMp3F=1;
	MP3_Power = 1;
  MP3TrackPlayer(0,7); //Welcome
	MP3TrackPlayer(0,3); //Welcome

  while (1)
  {
	
		WDT_RESET;
		
		//////////////////////////////////////////////////////////////////////////
		///////////////////////////   Call Task 5 Ms   ///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		if(Task5MsTimer > 4)
		{
			Task5MsTimer -= 5;
			
			GetDataTimer ++;
			if(GetDataTimer > 400)
			{
				USART1_Init();		//UART2 For Data Transmition with Main Board
				CRXDataCNT = CAB_RX_DMA_CNT;
				SyncStatus = 0;
				PacketLen = 0;
				DMA1_CH3_Init();	//DMA For UART2 TX 
				DMA1_CH4_Init();	//DMA For UART2 RX 
			}
			
			////////////////////////////
			///// Inputs Debouncer /////
			////////////////////////////
			Inputs_Debouncer(); //Hardware.c
			
			
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		///////////////////////////  Call Task 100 Ms  ///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		if(Task100MsTimer > 99) 
		{
			Task100MsTimer -= 100;
			
			if(RunOrStop == STOP) 
			{
				PB_BlinkStatus = 1;
				PB_BlinkTimer = 0;
			}
			else
			{
				if(LandingF == 1 && PB_BlinkTimer < 2) 	PB_BlinkTimer++;
				if(LandingF == 0 && PB_BlinkTimer < 4)	PB_BlinkTimer++;
				else 
				{
					PB_BlinkTimer = 0;
					PB_BlinkStatus = !PB_BlinkStatus;
				}
			}
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		////////////////////////////  Call Task 1Sec  ////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		if(Task1SecTimer > 999)
		{
			Task1SecTimer -= 1000;
			
			Task1MinuteTimer++;
			
			CAB_DataCount = CAB_DataCounter;
			CAB_DataCounter = 0;
		}
			
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////  Call Task 1Minute   //////////////////////////
		//////////////////////////////////////////////////////////////////////////
		if(Task1MinuteTimer > 59) //59
		{
			Task1MinuteTimer -= 60; //60	
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		///////////////////////////    Serial Cabin    ///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CAB_SerialDataTransmition(); //Serial.c		
		
		MP3PlayerOperation(); //MP3.c

  }

}







void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}

#endif 
