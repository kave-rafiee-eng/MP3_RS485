
#ifndef __SERIAL_H
#define __SERIAL_H

	#include "main.h"

	
	#define	CAB_TX_BUFFER_SIZE			50
	#define	CAB_RX_BUFFER_SIZE			200

	#define	CAB_TX_DMA_CNT  				DMA1_Channel3->CNDTR
	#define	CAB_RX_DMA_CNT  				(CAB_RX_BUFFER_SIZE - DMA1_Channel4->CNDTR)

	#define	CAB_SERIAL_ID						0xAA

	void CAB_SerialDataTransmition(void);

#endif