#include "ControlCh.h"
#include "Zing.h"
#include "ZingHw.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "macro.h"
#include "dma.h"
#include "queue.h"

CyU3PThread ControlCh_ZingRX_ThreadHandle;

void ControlCh_ZingRX_Thread(uint32_t Value)
{
	uint32_t rt_len;
	uint8_t *buf = (uint8_t *)CyU3PDmaBufferAlloc (512);
	CyU3PReturnStatus_t Status;

	ZingHdr_t zing_hdr;

	while(1)
	{
		if (Dma.Mode_ == DMA_NORMAL)
		{
			if ((Status = Zing_Transfer_Recv(&Dma.ControlIn_.Channel_, buf, &rt_len, CYU3P_WAIT_FOREVER)) == CY_U3P_SUCCESS)
			{
				memcpy(&zing_hdr, buf, ZING_HDR_SIZE);

				CyU3PDebugPrint(4, "Receive from ZING\r\n");
				CyU3PDebugPrint(4, "dir=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "interrupt=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "target=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "type=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "req_resp=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "fr_type=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "length=%d\r\n", zing_hdr.dir);
				CyU3PDebugPrint(4, "Print=%s\r\n", buf + ZING_HDR_SIZE);
			}
		}
		else {
			CyU3PThreadSleep(10);
		}
	}
	CyU3PDmaBufferFree(buf);
}

CyU3PReturnStatus_t ControlCh_ZingRX_Thread_Create(void)
{
	void *StackPtr = NULL;
	CyU3PReturnStatus_t Status;

	StackPtr = CyU3PMemAlloc(CONTROLCH_ZingRX_THREAD_STACK);
	Status = CyU3PThreadCreate(&ControlCh_ZingRX_ThreadHandle,	// Handle to my Application Thread
				"26:tmp2",								// Thread ID and name
				ControlCh_ZingRX_Thread,						// Thread entry function
				0,										// Parameter passed to Thread
				StackPtr,								// Pointer to the allocated thread stack
				CONTROLCH_ZingRX_THREAD_STACK,					// Allocated thread stack size
				CONTROLCH_ZingRX_THREAD_PRIORITY,				// Thread priority
				CONTROLCH_ZingRX_THREAD_PRIORITY,				// = Thread priority so no preemption
				CYU3P_NO_TIME_SLICE,					// Time slice no supported
				CYU3P_AUTO_START						// Start the thread immediately
	);
	return Status;
}
