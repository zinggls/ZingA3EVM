#include "DEVToPPC.h"
#include "Zing.h"
#include "ZingHw.h"
#include "dma.h"
#include "Zing.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "macro.h"
#include "Flag0Event.h"

CyU3PThread DEVToPPCThreadHandle;

void DEVToPPCThread(uint32_t Value)
{
	uint8_t* buf = (uint8_t*)CyU3PDmaBufferAlloc(256);
	uint32_t len;
	CyU3PReturnStatus_t Status;
	ZingHdr_t header;
	uint8_t* transmit_data = (uint8_t*)CyU3PDmaBufferAlloc(256 + ZING_HDR_SIZE);
	uint32_t evStat;

	while (1)
	{
#ifdef HRCP_DEV
		memset(buf, 0, 256);
		if ((Status = Zing_Transfer_Recv(&Dma.ControlIn_.Channel_, buf, &len, CYU3P_WAIT_FOREVER)) == CY_U3P_SUCCESS)
		{
			//CyU3PDebugPrint(4, "[D2P] Transmit to PPC: %s (%d)\r\n", buf, len);

			memset(&header, 0, ZING_HDR_SIZE);
			header.dir = 0;
			header.interrupt = 0;
			header.target = 0;
			header.type = 1;
			header.req_resp = 0;
			//header.rsvd0 = 0;
			header.fr_type = 1;
			//header.rsvd1 = 0;
			//header.intr_flags = 0;
			//header.addr = 0;
			header.length = len;

			memcpy(transmit_data, &header, ZING_HDR_SIZE);
			memcpy(transmit_data + ZING_HDR_SIZE, buf, len);

			if ((Status = Zing_Transfer_Send(&Dma.CPUPIB_Out, transmit_data, ZING_HDR_SIZE + len)) != CY_U3P_SUCCESS)
			{
				//CyU3PDebugPrint(4, "[D2P] Transmit error %d\r\n", Status);
			}

			//Status = CyU3PEventGet(&ZingFlag0Event, EVT_FLAG0, CYU3P_EVENT_OR_CLEAR, &evStat, CYU3P_WAIT_FOREVER);

		}
#elif defined(HRCP_PPC)
		memset(transmit_data, 0, 256 + ZING_HDR_SIZE);
		if ((Status = Zing_Transfer_Recv(&Dma.CPUPIB_In, transmit_data, &len, CYU3P_WAIT_FOREVER)) == CY_U3P_SUCCESS)
		{
			memset(buf, 0, 256);
			memcpy(buf, transmit_data + ZING_HDR_SIZE, len - ZING_HDR_SIZE);
			memcpy(&header, transmit_data, ZING_HDR_SIZE);

			/*
			CyU3PDebugPrint(4, "[D2P] Receive from DEV: %s(%d)\r\n", buf, len - ZING_HDR_SIZE);

			CyU3PDebugPrint(4, "[D2P] DEV HDR(addr): %d\r\n", header.addr);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(dir): %d\r\n", header.dir);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(fr_type): %d\r\n", header.fr_type);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(interrupt): %d\r\n", header.interrupt);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(intr_flags): %d\r\n", header.intr_flags);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(length): %d\r\n", header.length);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(req_resp): %d\r\n", header.req_resp);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(target): %d\r\n", header.target);
			CyU3PDebugPrint(4, "[D2P] DEV HDR(type): %d\r\n", header.type);
			*/

			if (len != 0)
			{
				CyU3PEventSet(&ZingFlag0Event, EVT_FLAG0, CYU3P_EVENT_OR);
			}
			else
			{
				continue;
			}

			if ((Status = Zing_Transfer_Send(&Dma.ControlOut_.Channel_, buf, len - ZING_HDR_SIZE)) != CY_U3P_SUCCESS)
			{
				//CyU3PDebugPrint(4, "[D2P] Transmit error\r\n");
			}
		}
#endif
	}
}

CyU3PReturnStatus_t DEVToPPCThread_Create(void)
{
	void *StackPtr = NULL;
	CyU3PReturnStatus_t Status;

	//CHECK(CyU3PEventCreate(&P2DCtx.Event_));

	StackPtr = CyU3PMemAlloc(DEVTOPPC_THREAD_STACK);
	Status = CyU3PThreadCreate(&DEVToPPCThreadHandle,	// Handle to my Application Thread
				"32:D2P",								// Thread ID and name
				DEVToPPCThread,						// Thread entry function
				0,										// Parameter passed to Thread
				StackPtr,								// Pointer to the allocated thread stack
				DEVTOPPC_THREAD_STACK,					// Allocated thread stack size
				DEVTOPPC_THREAD_PRIORITY,				// Thread priority
				DEVTOPPC_THREAD_PRIORITY,				// = Thread priority so no preemption
				CYU3P_NO_TIME_SLICE,					// Time slice no supported
				CYU3P_AUTO_START						// Start the thread immediately
	);
	return Status;
}
