#include "main.h"
#include "MP3.h"
#include "Hardware.h"
#include "Definition.h"

///////// MP3 
extern	volatile	_Bool				MP3_SetupF,MP3_Power;
extern	volatile 	uint8_t			ActivePlayer,StopMp3F,CMD_MP3File,PreMp3File,GangPulsTime,RandTrack,ReplayCounter;
extern	volatile 	uint8_t     PlayerList[10],Mp3Listcc,Mp3Whilecc,PlayerStatus,Mp3Folder,Mp3Track,LatestPlayedMusics[10],LatestMusicCounter;
extern	volatile	uint16_t		PowerOnDelay,PowerOffDelay,PlayerChecker;  

///////// UARTM For MP3
extern	volatile 	uint8_t			UARTM_StrLen;
extern	char 									MP3_Str[20];

///////// Timer
extern	volatile	uint8_t			UART_MP3_TX_Time;

/////////	Setting
extern	volatile 	uint8_t			GangSelect,FloorTalk,WelcomeF,MusicVolume,TalkVolume;

extern	volatile	_Bool				E_Level;

void MP3TrackPlayer(uint8_t Folder,uint8_t Track)
{
   PlayerList[Mp3Listcc]=Folder;
   Mp3Listcc++;
   PlayerList[Mp3Listcc]=Track;
   Mp3Listcc++;
   
   if(Mp3Listcc>9) Mp3Listcc=0;
}


void  StopMp3Activity(void)
{
   Mp3Listcc = 0;
   Mp3Whilecc = 0;
   ActivePlayer = 0;
   PlayerStatus = 0;
}
void  MP3PlayerOperation(void)
{
   if(MP3_Power == 0)
   {
      MP3_SetupF = 1;
      PowerOnDelay = 0;
			CMD_MP3File = 0;
			PreMp3File = 0;
      StopMp3Activity();         
   }
   
   else if(PreMp3File != CMD_MP3File)
   {
      if(CMD_MP3File>0)
      {
         if(CMD_MP3File==100)  // announce floor level
         {
            char mps=0;
            
            if(GangSelect>0 && GangSelect<4)  //Play Gang
            {
               if(MP3_BUSY == 0) 
               {
                  StopMp3F = 1;
                  StopMp3Activity();
               }
               MP3TrackPlayer(0,GangSelect+5);
               GangPulsTime = 50; // For 1Sec gang pulse
               
               mps = 1;
            }
            if(FloorTalk > 0 && FloorTalk < 55)
            {       
               if(MP3_BUSY == 0 && mps == 0) 
               {
                  StopMp3F = 1;
                  StopMp3Activity();
               }
							 uint8_t Ft = FloorTalk;
							 if(Ft > 8) Ft+=16; //Just 8 Level
               if(Ft <= 34)       MP3TrackPlayer(1,Ft);    //Play floor level
               else if(Ft <= 54)  MP3TrackPlayer(2,Ft-34); //Play floor level
               
               mps = 1;
            }
            
            if(WelcomeF)  //Play welcome
            {
               MP3TrackPlayer(0,3);       
               
               mps = 1;
            }
            
            if(mps == 0)
            {
               MP3TrackPlayer(0,0); //Pause
            }
         }
         else if(CMD_MP3File == 130)
         {
            MP3TrackPlayer(0,0); //Pause
         }
         else if(CMD_MP3File == 114) //Dont disturb closing the door
         {
            if(MP3_BUSY == 0) 
            {
               StopMp3F = 1;
               StopMp3Activity();
            }
            MP3TrackPlayer(0,1); 
            
         }
         else if(CMD_MP3File == 113) //Excess capacity allowed
         {
            if(MP3_BUSY == 0) 
            {
               StopMp3F = 1;
               StopMp3Activity();
            }
            MP3TrackPlayer(0,2); 
         }
         else if(CMD_MP3File == 111) //Emergency system
         {
            if(MP3_BUSY == 0) 
            {
               StopMp3F = 1;
               StopMp3Activity();
            }
            MP3TrackPlayer(0,4); 
         }
         else if(CMD_MP3File == 120) // Play Music
         {
            uint8_t tr = 200,Timeout=0;
            while(tr > 120)
            {
							WDT_RESET;
               tr = RandTrack;
               
               if(Timeout < 15)
               {
                  Timeout ++;
                  for(int i=i;i<10;i++)
                  {
                     if(LatestPlayedMusics[i] == tr) 
                     {
                        tr = 200;
                        break;
                     }
                  }
               }
            }
            
            LatestPlayedMusics[LatestMusicCounter] = tr;
            LatestMusicCounter ++;
            if(LatestMusicCounter > 9) LatestMusicCounter = 0;
            
            uint8_t folder = 3;  //Folder 3-8               
            while(tr > 20) {tr-=20; folder++;}
            
            MP3TrackPlayer(folder,tr);
         }
         
      }
      
      
      PreMp3File = CMD_MP3File;
   }
   
   
   if(Mp3Listcc != Mp3Whilecc && ActivePlayer==0 && UART_MP3_TX_Time>30 && PowerOnDelay>2550)
   {
      Mp3Folder = PlayerList[Mp3Whilecc];
      Mp3Whilecc++;
      Mp3Track = PlayerList[Mp3Whilecc];
      Mp3Whilecc++;
      if(Mp3Whilecc>9) Mp3Whilecc=0;
      
      if(Mp3Track > 0) ActivePlayer = 1;
      PlayerStatus = 0;
      
      UART_MP3_TX_Time=0;
      
      if((StopMp3F || Mp3Track == 0) && MP3_SetupF == 0)//if(MP3_Power && MP3_BUSY == 0 && MP3_SetupF == 0)//if(StopMp3F || Mp3Track == 0)
      {
         MP3_Str[0]=0x7E; //Start Byte
         MP3_Str[1]=0x03; //Len
         MP3_Str[2]=0x19; //Opcode (0x19)Reset
         MP3_Str[3]=(MP3_Str[1] ^ MP3_Str[2]); //Checksum ((MP3_Str[1] ^ MP3_Str[2])
         MP3_Str[4]=0xEF; //End            
         UARTM_StrLen = 5;
      }
      else
      {
         MP3_SetupF = 0;
         
         MP3_Str[0] = 0x7E; //Start Byte
         MP3_Str[1] = 0x04; //Len
         MP3_Str[2] = 0x38; //Opcode (0x38) Bussy Pin Level
         MP3_Str[3] = 0x00; //Param
         MP3_Str[4] = 0x3C; //Checksum ((MP3_Str[1] ^ MP3_Str[2]) ^ MP3_Str[3])
         MP3_Str[5] = 0xEF; //End
         UARTM_StrLen = 6;
      }
   }
   
   else if(ActivePlayer && UART_MP3_TX_Time>35 && (MP3_BUSY || StopMp3F || PlayerStatus==3) && PowerOnDelay>2550)
   {
      if(PlayerStatus==0)
      {
         UART_MP3_TX_Time=0;
         PlayerStatus ++;
      }
      else if(PlayerStatus==1)
      {
         uint8_t Volume=0;
         if(Mp3Folder<3) {Volume = TalkVolume * 3;}
         else            {Volume = MusicVolume * 3;}
         if(Volume > 30)  Volume = 30;
         
         MP3_Str[0]=0x7E; //Start Byte
         MP3_Str[1]=0x04; //Len
         MP3_Str[2]=0x31; //Opcode (Volume)
         MP3_Str[3]=Volume; //Param (0-30)
         MP3_Str[4]=(MP3_Str[1] ^ MP3_Str[2]) ^ MP3_Str[3]; //Checksum
         MP3_Str[5]=0xEF; //End
         UART_MP3_TX_Time=0;
         UARTM_StrLen = 6;
         PlayerStatus++;
      }
      else if(PlayerStatus==2)
      {
         MP3_Str[0]=0x7E; //Start Byte
         MP3_Str[1]=0x05; //Len
         MP3_Str[2]=0x42; //Opcode (Spec Floder & Trackl)
         MP3_Str[3]=Mp3Folder; //Param1 (Floder)
         MP3_Str[4]=Mp3Track; //Param2 (Track)
         MP3_Str[5]=((MP3_Str[1] ^ MP3_Str[2]) ^ MP3_Str[3]) ^ MP3_Str[4]; //Checksum
         MP3_Str[6]=0xEF; //End
         UART_MP3_TX_Time=0;
         UARTM_StrLen = 7;
         PlayerChecker=0;
         PlayerStatus++;            
      }
      else if(PlayerStatus==3)
      {
         if(MP3_BUSY == 0 || PlayerChecker > 600) 
         {
            PlayerStatus = 0;
            ActivePlayer=0;

            if(StopMp3F) StopMp3F=0;
            
            if(MP3_BUSY && PlayerChecker > 600) 
            {
               PreMp3File = 255;
               ReplayCounter ++;
               if(ReplayCounter > 3) {PowerOffDelay = 0; ReplayCounter = 0;}
            }
            else ReplayCounter = 0;
         }
      }
      
   
   }
}
