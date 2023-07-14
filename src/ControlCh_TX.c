#include "ControlCh.h"
#include "Zing.h"
#include "ZingHw.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "macro.h"
#include "dma.h"
#include "queue.h"

CyU3PThread ControlCh_TX_ThreadHandle;

void ControlCh_TX_Thread(uint32_t Value)
{
	uint32_t rt_len;
	uint8_t *buf = (uint8_t *)CyU3PDmaBufferAlloc (512);
	CyU3PReturnStatus_t Status;

	uint8_t* zing_frame = (uint8_t*)CyU3PDmaBufferAlloc(512);

	while(1)
	{
		if (Dma.Mode_ == DMA_NORMAL)
		{
			if ((Status = Zing_Transfer_Recv(&Dma.ZingCtrlIn, buf, &rt_len, CYU3P_WAIT_FOREVER)) == CY_U3P_SUCCESS)
			{
				Zing_Header2(zing_frame,
							ZING_HDR_DIR_EGRESS, 	// Dir
							0,						// Interrupt
							ZING_HDR_TARGET_BUFFER,	// Target
							ZING_HDR_ACTION_WRITE, 	// Type
							0,						// Request Response
							1,						// Frame type
							0,						// Interrupt flag
							0,						// Address
							rt_len);				// Length
				memcpy(zing_frame + ZING_HDR_SIZE, buf, rt_len);

				queue_enqueue(zing_frame, ZING_HDR_SIZE + rt_len);
			}
		}
		else {
			CyU3PThreadSleep(10);
		}
	}
	CyU3PDmaBufferFree(buf);
}

CyU3PReturnStatus_t ControlCh_TX_Thread_Create(void)
{
	void *StackPtr = NULL;
	CyU3PReturnStatus_t Status;

	StackPtr = CyU3PMemAlloc(CONTROLCH_TX_THREAD_STACK);
	Status = CyU3PThreadCreate(&ControlCh_TX_ThreadHandle,	// Handle to my Application Thread
				"23:tmp2",								// Thread ID and name
				ControlCh_TX_Thread,						// Thread entry function
				0,										// Parameter passed to Thread
				StackPtr,								// Pointer to the allocated thread stack
				CONTROLCH_TX_THREAD_STACK,					// Allocated thread stack size
				CONTROLCH_TX_THREAD_PRIORITY,				// Thread priority
				CONTROLCH_TX_THREAD_PRIORITY,				// = Thread priority so no preemption
				CYU3P_NO_TIME_SLICE,					// Time slice no supported
				CYU3P_AUTO_START						// Start the thread immediately
	);
	return Status;
}
