#include "main.h"
#include "Hardware.h"
#include "Definition.h"
#include "Serial.h"


///////// Hardware
extern	volatile 	_Bool 			ReadInputsShiftF;
extern	volatile  _Bool       HwIn_Rev,HwIn_RevUP,HwIn_RevDN,HwIn_STP,HwIn_OVL,HwIn_DO,HwIn_PHC,HwIn_IN;
extern	volatile 	_Bool 			Hw_SegA,Hw_SegB,Hw_SegC,Hw_SegD,Hw_SegE,Hw_SegF,Hw_SegG,Hw_SegUP,Hw_SegDN,Hw_SegL;
extern	volatile  _Bool				HwOut_PB[MAX_NOS],HwIn_PB[MAX_NOS],OE_PB_State;
extern	volatile  _Bool				Relay_L5D,Relay_L6,Relay_URA,Relay_D1,Relay_D2,BUZZ,OUT;
extern	volatile 	uint8_t  		InputsShiftStatus,PowerVoltage;
extern	volatile 	uint16_t    adcv[2];


///////// Serial 
extern	char                 	Print_Str[100],CAB_TX_Buffer[CAB_TX_BUFFER_SIZE];
extern	volatile	char				CAB_RX_Buffer[CAB_RX_BUFFER_SIZE];

///////// Timer
extern	volatile	uint8_t			InDebTimer[8];

/////////	Program
extern	volatile	_Bool				In_IN,In_PHC,In_DO,In_OVL,In_REV,In_UP,In_DN,In_STP,T_DO,T_PHC;


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}



void TIM17_Init (void)
{
	RCC->APBENR2 |= RCC_APBENR2_TIM17EN;          /* enable clock for TIM17   */
	TIM17->PSC  = 0;
	TIM17->CR1  = 0;

	TIM17->PSC = 0;
	TIM17->ARR = 6666; //104.1 us for 9600bps AutoReload
	TIM17->DIER = TIM_DIER_UIE;                   /* Update Interrupt enable */
	NVIC_EnableIRQ(TIM17_IRQn);       	  				/* TIM17   Interrupt enable */

	TIM17->CR1  |= TIM_CR1_CEN;                   /* timer enable            */
}

void TIM14_Init(void)
{
  RCC->APBENR2 |= RCC_APBENR2_TIM14EN;          /* enable clock for TIM14   */
  TIM14->PSC  = 0;
  TIM14->CR1  = 0;
  TIM14->PSC   = ( 63 );                        /* set prescaler   = 1MHz   */
  TIM14->ARR   = ( 999 );                       /* set auto-reload = 1ms    */

	TIM14->DIER = TIM_DIER_UIE;                   /* Update Interrupt enable  */
  NVIC_EnableIRQ(TIM14_IRQn);                   /* TIM14   Interrupt enable */

  TIM14->CR1  |= TIM_CR1_CEN;                   /* timer enable             */
}

void MX_GPIO_Init(void)
{
	
	 GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_14; //LED_LINK
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	 
	
	GPIO_InitStruct.Pin = GPIO_PIN_0; //MP3_POWER
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	 
	 
	GPIO_InitStruct.Pin = GPIO_PIN_1; //MP3_BUZI
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	 
	 
	GPIO_InitStruct.Pin = GPIO_PIN_2; //MP3_TX
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	
	GPIO_InitStruct.Pin = GPIO_PIN_4; //HV_REV
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
		
	GPIO_InitStruct.Pin = GPIO_PIN_5; //HV_UP
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
		
	GPIO_InitStruct.Pin = GPIO_PIN_6; //HV_DN
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_7; //HV_DN
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	
	GPIO_InitStruct.Pin = GPIO_PIN_0; //HV_DO
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);		
	
	GPIO_InitStruct.Pin = GPIO_PIN_11; //DOOR 1
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	
		
	GPIO_InitStruct.Pin = GPIO_PIN_12; //URA
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	
}



void USART1_Init(void)
{
	int i;

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	
	GPIOB->MODER &= (uint32_t)~( ((uint32_t)0x03 << 12) | ((uint32_t)0x03 << 14) );  //PB6,PB7 Clear
	GPIOB->MODER |= (uint32_t) ( ((uint32_t)0x02 << 12) | ((uint32_t)0x02 << 14) );  //PB6,PB7 Alternate function mode
	
	GPIOB->AFR[0] &= (uint32_t)~( ((uint32_t)0x0F << 20) ); //PB6 , AF Clear
	GPIOB->AFR[0] |= (uint32_t) ( ((uint32_t)0x00 << 20) ); //PB6 AF0 For USART2
	
	GPIOB->AFR[0] &= (uint32_t)~( ((uint32_t)0x0F << 24) ); //PB7 , AF Clear
	GPIOB->AFR[0] |= (uint32_t) ( ((uint32_t)0x00 << 24) ); //PB7 AF0 For USART2
	
	RCC->APBENR2 |= RCC_APBENR2_USART1EN;    // enable USART1 clock

	USART1->BRR   = 0x1A0B;                  // 9600 baud @ PCLK2 64MHz
	//USART1->BRR   = 6630;
	USART1->CR1   = 0;                       // Clear Control Register 1
	USART1->CR1   = USART_CR1_TE  | USART_CR1_RE;     // enable TX & RX

	
	USART1->CR3  |= USART_CR3_DMAT | USART_CR3_DMAR | USART_CR3_OVRDIS;        // Enable TX DMA Bit & RX DMA Bit
	for (i = 0; i < 0x1000; i++) __NOP();    // avoid unwanted output

	USART1->CR1  |= USART_CR1_UE;// | USART_CR1_RXNEIE_RXFNEIE;   // Enable USART3 & RX Interrupts
	//NVIC_EnableIRQ(USART3_4_IRQn);                   /* USART2  RX Interrupt enable */
}

void DMA1_CH3_Init(void)
{
	DMA1_Channel3->CCR  = 0; 	/* DMA Channel 1 Disable  */

	RCC->AHBENR |= RCC_AHBENR_DMA1EN; 	/* enable peripheral clock for DMA1 */

	DMAMUX1_Channel2->CCR = 0x00000033; //Request MUX Input: USART1_TX
	DMAMUX1_RequestGenStatus->RGCFR = 0;
	//DMAMUX1_ChannelStatus->CSR = 0;
	//DMAMUX1_ChannelStatus->CFR = 0;
	//DMAMUX1_RequestGenStatus->RGSR = 0;

	DMA1_Channel3->CMAR  = (uint32_t) &USART1->TDR;    	/* set chn1 memory address    */
	DMA1_Channel3->CPAR  = (uint32_t) CAB_TX_Buffer;    		/* set chn1 peripheral address*/
	DMA1_Channel3->CNDTR = 0;     		/* transmit size word         */
	
	/* configure DMA channel      */
	DMA1_Channel3->CCR	= 	(0<<14) |       //Memory to memory mode Enable
													(0<<12) |       //Channel priority level 0:Low 1:Medium 2:High 3:Very high
													(0<<10) |       //Memory size 0:8bit 1:16bit 2:32bit
													(0<< 8) |       //Peripheral size 0:8bit 1:16bit 2:32bit
													(0<< 7) |       //Memory increment mode Disable
													(1<< 6) |       //Peripheral increment mode Enable
													(0<< 5) |       //Circular mode Disable
													(0<< 4) |       //Data transfer direction  0: Read from peripheral  1: Read from memory
													(0<< 3) |       //Transfer error interrupt Disable
													(0<< 2) |       //Half transfer interrupt Disable
													(0<< 1) |       //Transfer complete interrupt Enable
													(0<< 0) ;       //Channel Disable


	/* Enable DMA1 Channel1 Transfer Complete interrupt */
	//NVIC_EnableIRQ(DMA1_Channel4_IRQn);     /* enable DMA1 Channel1 Interrupt   */
	DMA1_Channel3->CCR  |= DMA_CCR_EN;     /* DMA Channel 1 enable  */
}

void DMA1_CH4_Init(void)
{
	DMA1_Channel4->CCR  = 0;     /* DMA Channel 2 Disable  */

	RCC->AHBENR |= RCC_AHBENR_DMA1EN; 	/* enable peripheral clock for DMA1 */

	DMAMUX1_Channel3->CCR = 0x00000032; 	//Request MUX Input: USART1_RX
	DMAMUX1_RequestGenStatus->RGCFR = 0;
	//DMAMUX1_ChannelStatus->CSR = 0;
	//DMAMUX1_ChannelStatus->CFR = 0;
	//DMAMUX1_RequestGenStatus->RGSR = 0;

	DMA1_Channel4->CMAR  = (uint32_t) CAB_RX_Buffer;   /* set chn1 memory address    */
	DMA1_Channel4->CPAR  = (uint32_t) &USART1->RDR;    /* set chn1 peripheral address*/
	DMA1_Channel4->CNDTR = CAB_RX_BUFFER_SIZE;         /* transmit size word         */
	
	/* configure DMA channel      */
	DMA1_Channel4->CCR	= 	(0<<14) |       //Memory to memory mode Enable
													(2<<12) |       //Channel priority level 0:Low 1:Medium 2:High 3:Very high
													(0<<10) |       //Memory size 0:8bit 1:16bit 2:32bit
													(0<< 8) |       //Peripheral size 0:8bit 1:16bit 2:32bit
													(1<< 7) |       //Memory increment mode Disable
													(0<< 6) |       //Peripheral increment mode Disable
													(1<< 5) |       //Circular mode Disable
													(0<< 4) |       //Data transfer direction  0: Read from peripheral  1: Read from memory
													(0<< 3) |       //Transfer error interrupt Disable
													(0<< 2) |       //Half transfer interrupt Disable
													(0<< 1) |       //Transfer complete interrupt Disable
													(0<< 0) ;       //Channel Disable


	/* Enable DMA1 Channel1 Transfer Complete interrupt */
	//NVIC_EnableIRQ(DMA1_Channel4_IRQn);     /* enable DMA1 Channel1 Interrupt   */
	DMA1_Channel4->CCR  |= DMA_CCR_EN;     /* DMA Channel 1 enable  */
}

void SerialSendData(int length)
{
	//HAL_UART_Transmit(&huart1, (uint8_t*)buffer, length , 500);
	//HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer, length );
	DMA1_Channel3->CCR  = 0;
	DMA1_Channel3->CNDTR = length;              /* transmit size word          */
	DMA1_Channel3->CCR   = DMA_CCR_PINC | DMA_CCR_EN;       //Peripheral increment mode Enable
}

void Inputs_Debouncer(void)
{
	
	HwIn_Rev = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4);
	HwIn_RevUP = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5);	
	HwIn_RevDN = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6);	
	HwIn_STP = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7);	
	HwIn_DO = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);	
	
	
	if(HwIn_Rev)		{if(InDebTimer[4] < 20) {InDebTimer[4]++;} 	else {In_REV= 1;}}
	else            {if(InDebTimer[4] > 0) 	{InDebTimer[4]--;}	else {In_REV= 0;}}
	
	if(HwIn_RevUP)	{if(InDebTimer[5] < 20) {InDebTimer[5]++;} 	else {In_UP	= 1;}}
	else            {if(InDebTimer[5] > 0) 	{InDebTimer[5]--;}	else {In_UP	= 0;}}
	
	if(HwIn_RevDN)	{if(InDebTimer[6] < 20) {InDebTimer[6]++;} 	else {In_DN = 1;}}
	else            {if(InDebTimer[6] > 0) 	{InDebTimer[6]--;}	else {In_DN	= 0;}}
	
	if(HwIn_STP)	{if(InDebTimer[7] < 20) {InDebTimer[7]++;} 	else {In_STP = 1;}}
	else          {if(InDebTimer[7] > 0) 	{InDebTimer[7]--;}	else {In_STP	= 0;}}
		
	if(HwIn_DO)	{if(InDebTimer[3] < 20) {InDebTimer[3]++;} 	else { T_PHC = 0;}}
	else          {if(InDebTimer[3] > 0) 	{InDebTimer[3]--;}	else {T_PHC	= 1;}}
	
	T_DO = T_PHC;
}






