#include "ControlCh.h"
#include "Zing.h"
#include "ZingHw.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "macro.h"
#include "dma.h"
#include "queue.h"

CyU3PThread ControlCh_ZingTX_ThreadHandle;

void ControlCh_ZingTX_Thread(uint32_t Value)
{
	uint8_t *buf = (uint8_t *)CyU3PDmaBufferAlloc (512);
	CyU3PReturnStatus_t Status;

	ZingHdr_t zing_hdr;

	while(1)
	{
		if (Dma.Mode_ == DMA_NORMAL)
		{
			if (!(queue_dequeue(buf, 512) == 0))
			{
				memcpy(&zing_hdr, buf, ZING_HDR_SIZE);

				if ((Status = Zing_Transfer_Send(&Dma.ControlOut_.Channel_, buf, ZING_HDR_SIZE + zing_hdr.length)) != CY_U3P_SUCCESS)
				{

				}
			}
			else
			{
				CyU3PThreadSleep(10);
			}

		}
		else {
			CyU3PThreadSleep(10);
		}
	}
	CyU3PDmaBufferFree(buf);
}

CyU3PReturnStatus_t ControlCh_ZingTX_Thread_Create(void)
{
	void *StackPtr = NULL;
	CyU3PReturnStatus_t Status;

	StackPtr = CyU3PMemAlloc(CONTROLCH_ZingTX_THREAD_STACK);
	Status = CyU3PThreadCreate(&ControlCh_ZingTX_ThreadHandle,	// Handle to my Application Thread
				"24:tmp2",								// Thread ID and name
				ControlCh_ZingTX_Thread,						// Thread entry function
				0,										// Parameter passed to Thread
				StackPtr,								// Pointer to the allocated thread stack
				CONTROLCH_ZingTX_THREAD_STACK,					// Allocated thread stack size
				CONTROLCH_ZingTX_THREAD_PRIORITY,				// Thread priority
				CONTROLCH_ZingTX_THREAD_PRIORITY,				// = Thread priority so no preemption
				CYU3P_NO_TIME_SLICE,					// Time slice no supported
				CYU3P_AUTO_START						// Start the thread immediately
	);
	return Status;
}
