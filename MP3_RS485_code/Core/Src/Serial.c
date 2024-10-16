#include "main.h"
#include "Serial.h"
#include "Hardware.h"
#include "Definition.h"

///////// PB & Num
extern	volatile 	_Bool				CPB[MAX_NOS],CPB_Cancel[MAX_NOS],PB_BlinkStatus;
extern	volatile 	uint8_t 		PB_Time[MAX_NOS],CPB_Clear,CPB_Set,CPB_Blinker;
extern	volatile 	uint8_t			Numrator;

///////// Serial 
extern	char                 	Print_Str[100],CAB_TX_Buffer[CAB_TX_BUFFER_SIZE];
extern	volatile	char				CAB_RX_Buffer[CAB_RX_BUFFER_SIZE];
extern	volatile	_Bool				NewPacketF;
extern	volatile	uint8_t			CRXDataCNT,PacketCounter,SyncStatus,PacketLen,ReceivedPacket[30],CAB_DataCounter,CAB_DataCount,LinkLED_Status;
extern	volatile	uint16_t		PacketError,PreCabTxDmaCnt,GetDataTimer;


/////////	Program
extern	volatile	_Bool				In_IN,In_PHC,In_DO,In_OVL,In_REV,In_UP,In_DN,In_STP,T_PHC,T_DO,SegUp,SegDn;
extern	volatile	_Bool				Safety_90,Safety_71,Safety_66,Safety_69,Safety_68,OutStatus;
extern	volatile	_Bool				StandbyF,CMD_Door1,CMD_Door2,CMD_URA,RunOrStop,SegL,LandingF,WelcomeF;
extern	volatile	uint8_t 		E_Level,CMD_MP3File,FloorTalk,GangSelect,MusicVolume,TalkVolume,PowerVoltage;

extern	float 								BatteryVoltage;

void CAB_SerialDataTransmition(void)
{
	while(CAB_RX_DMA_CNT != CRXDataCNT)
	{
		uint8_t DataIn = CAB_RX_Buffer[CRXDataCNT];
		
		if(DataIn == 0xFF && SyncStatus == 0) 			{PacketLen = 0;	SyncStatus++;}
		else if(SyncStatus == 0)										{PacketLen = 0;}
		else if(DataIn == 0xFE && SyncStatus == 1) 	{PacketLen = 0;	SyncStatus++;}
		else if(SyncStatus == 2)
		{
			if(DataIn == CAB_SERIAL_ID) 							{PacketLen = 0; SyncStatus++;}
			else {SyncStatus = 0;} //ID doesn't match
		}
		else if(SyncStatus == 3) {PacketLen = DataIn+4;	SyncStatus++; PacketCounter = 0;}
		else if(SyncStatus < PacketLen && SyncStatus > 3)
		{
			ReceivedPacket[PacketCounter] = DataIn;
			PacketCounter++;
			SyncStatus++;
		}
		else 
		{
			if(SyncStatus == PacketLen && PacketLen > 0 && PacketCounter == 9)
			{
				uint8_t	Cheksum = ReceivedPacket[0];
				for(uint16_t iw=1;iw < PacketLen;iw++) 
				{
					Cheksum ^= ReceivedPacket[iw];
				}
				
				if(DataIn == Cheksum ) 
				{
					NewPacketF = 1; 
					CAB_DataCounter++; 
					LinkLED_Status++;
					if(LinkLED_Status > 2) LinkLED_Status = 0;
					GetDataTimer = 0;
				}
				
			}
			
			if(NewPacketF == 0) {PacketError ++;}
			SyncStatus = 0;
			PacketLen = 0;
		}

		if(NewPacketF)
		{
			//////////////////////////////
			///// Get Data From Main /////
			//////////////////////////////
			
			Numrator 	= ReceivedPacket[0];
			
			SegL 			= ReceivedPacket[1] & 0x80;
			StandbyF 	= ReceivedPacket[1] & 0x40;
			LandingF 	= ReceivedPacket[1] & 0x20;
			Safety_90 = ReceivedPacket[1] & 0x10;
			Safety_71 = ReceivedPacket[1] & 0x08;
			Safety_66 = ReceivedPacket[1] & 0x04;
			Safety_69 = ReceivedPacket[1] & 0x02;
			Safety_68 = ReceivedPacket[1] & 0x01;
			
			CMD_Door1	= ReceivedPacket[2] & 0x80;
			CMD_Door2	= ReceivedPacket[2] & 0x40;
			CMD_URA		= ReceivedPacket[2] & 0x20;
			RunOrStop	= ReceivedPacket[2] & 0x10;
			WelcomeF	= ReceivedPacket[2] & 0x08;
			GangSelect	= ReceivedPacket[2] & 0x07;

			CMD_MP3File	=	ReceivedPacket[3];
			
			CPB_Blinker = ReceivedPacket[4] & 0x0F;
			E_Level 		=(ReceivedPacket[4] >> 4) & 0x0F;
			
			CPB_Set 		= ReceivedPacket[5] & 0x0F;
			CPB_Clear 	=(ReceivedPacket[5] >> 4) & 0x0F;
			
			FloorTalk 	= ReceivedPacket[6];
			
			TalkVolume 	= ReceivedPacket[7] & 0x0F;
			MusicVolume	=(ReceivedPacket[7] >> 4) & 0x0F;
			
			OutStatus 	= ReceivedPacket[8] & 0x80;
			SegUp  			=	ReceivedPacket[8] & 0x40;
			SegDn  			=	ReceivedPacket[8] & 0x20;
			
			
			/////////////////////////////
			///// Send Data To Main /////
			/////////////////////////////
			uint8_t i=0;
			
			for(i=0;i<10;i++) CAB_TX_Buffer[i] = 0;
			
			CAB_TX_Buffer[0] = 0xFF;
			CAB_TX_Buffer[1] = 0xFE;
			CAB_TX_Buffer[2] = CAB_SERIAL_ID;
			CAB_TX_Buffer[3] = 8; //Packet Len
			
			//Start Packet
			for(i=MAX_NOS;i<=MAX_NOS;i--)
			{
				CAB_TX_Buffer[4] |= CPB[i];
				CAB_TX_Buffer[6] |= CPB_Cancel[i];
				if(i > 0)
				{
					CAB_TX_Buffer[4] <<=1;
					CAB_TX_Buffer[6] <<=1;
				}
			}
			CAB_TX_Buffer[5] = CAB_TX_Buffer[4];
			CAB_TX_Buffer[7] = CAB_TX_Buffer[6];
			
			CAB_TX_Buffer[8] = 	((uint8_t)In_IN << 7)  | ((uint8_t)T_PHC << 6) | ((uint8_t)T_DO << 5)  | ((uint8_t)In_OVL << 4) | 
													((uint8_t)In_REV << 3) | ((uint8_t)In_UP << 2) | ((uint8_t)In_DN << 1) | In_STP;
			CAB_TX_Buffer[9] = CAB_TX_Buffer[8];
			
			CAB_TX_Buffer[10] = BatteryVoltage * 40; //Accuracy 0.033
			CAB_TX_Buffer[11] = 120;
			CAB_TX_Buffer[12] = 0;
			SerialSendData(13);
			NewPacketF = 0;
		}
		
		CRXDataCNT++;
		if(CRXDataCNT >= CAB_RX_BUFFER_SIZE) CRXDataCNT = 0;
	}
	LED_LINK(LinkLED_Status>1);
	
	
}



