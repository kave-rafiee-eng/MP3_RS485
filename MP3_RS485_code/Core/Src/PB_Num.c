#include "main.h"
#include "Hardware.h"
#include "Definition.h"
#include "PB_Num.h"


///////// PB & Num
extern	volatile 	_Bool				CPB[MAX_NOS],L_CPB[MAX_NOS],ForCancelStatus[MAX_NOS],CPB_Cancel[MAX_NOS],BlinkPBSt;
extern	volatile 	uint8_t 		PB_Time[MAX_NOS];
extern	volatile 	uint8_t			Shitf_PB_Cycle,Numrator;

///////// Hardware
extern	volatile  _Bool				HwOut_PB[MAX_NOS],HwIn_PB[MAX_NOS];
extern	volatile 	_Bool 			Hw_SegA,Hw_SegB,Hw_SegC,Hw_SegD,Hw_SegE,Hw_SegF,Hw_SegG,Hw_SegUP,Hw_SegDN,Hw_SegL;

/////////	Program
extern	volatile	_Bool				RunOrStop;
extern	volatile	uint8_t 		CPB_Clear,CPB_Set,CPB_Blinker,PB_BlinkStatus;
	

const char SegmentMap[] = 
{
   0x00, // 0: NULL
   0x3f, // 1: 0 
   0x06, // 2: 1
   0x5b, // 3: 2
   0x4f, // 4: 3 
   0x66, // 5: 4
   0x6d, // 6: 5
   0x7d, // 7: 6
   0x07, // 8: 7
   0x7f, // 9: 8 
   0x6f, // 10:9
   0x3D, // 11:G
   0x73, // 12:P
   0x50, // 13:r
   0x7C, // 14:b
   0x76, // 15:H
   0x79, // 16:E
   0x71, // 17:F   
   0x5E, // 18:d
   0x38, // 19:L
   0x77, // 20:A
   0x39, // 21:C
   0x6E, // 22:y   
   0x49, // 23:Day Counter Mark
   0x54, // 24:n for CF3 & 1CF Error
   0x1B  // 25:Start Mode Mark
};


void	Parallel_HallStation(void)
{
	for(uint8_t iw=0;iw<MAX_NOS;iw++)
	{
		if(CPB_Set == iw) 	
		{
			CPB[iw] = 1;
			CPB_Cancel[iw] = 0;
			ForCancelStatus[iw] = 0;
		}
		if(CPB_Clear == iw) 
		{
			CPB[iw] = 0;
			CPB_Cancel[iw] = 0;
			ForCancelStatus[iw] = 0;
		}
		if(CPB[iw] == 0) 
		{
			CPB_Cancel[iw] = 0;
			ForCancelStatus[iw] = 0;
		}
		
		if(HwIn_PB[iw])
		{
			 if(PB_Time[iw] < 60) PB_Time[iw]++;
			 
			 if(CPB[iw]==0 && L_CPB[iw]==0) 
			 {
					if(PB_Time[iw] > 10)
					{
						 CPB[iw] = 1;
						 L_CPB[iw] = 1;
					}
			 }
			 else if(CPB_Cancel[iw]==0 && CPB[iw] && ForCancelStatus[iw])  //&& L_CPB[i]
			 {
					if(PB_Time[iw] > CANCEL_PUSHB_TIME) CPB_Cancel[iw] = 1;
			 }
		}
		//else if(CPB_Cancel[iw])  {CPB[iw]=0; CPB_Cancel[iw]=0;}
		else
		{
			if(PB_Time[iw]>1) PB_Time[iw]-=2;

			else
			{          
				if(CPB[iw]) {ForCancelStatus[iw] = 1;}
				
				L_CPB[iw] = CPB[iw];
			}
		}
		
		_Bool v = CPB[iw];
		if(CPB_Blinker == iw && RunOrStop == RUN)
		{
			v = PB_BlinkStatus;
		}
		HwOut_PB[iw] = v;
	}
	
	Hw_SegA = (Numrator & 0x01)&&1;
	Hw_SegB = (Numrator & 0x02)&&1;
	Hw_SegC = (Numrator & 0x04)&&1;
	Hw_SegD = (Numrator & 0x08)&&1;
	Hw_SegE = (Numrator & 0x10)&&1;
	Hw_SegF = (Numrator & 0x20)&&1;
	Hw_SegG = (Numrator & 0x40)&&1;
}



