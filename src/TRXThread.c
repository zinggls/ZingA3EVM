#include "TRxThread.h"
#include "Zing.h"
#include "ZingHw.h"
#include "dma.h"
#include "Zing.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "macro.h"

CyU3PThread TRxThreadHandle;

void TRxThread(uint32_t Value)
{

}

CyU3PReturnStatus_t TRxThread_Create(void)
{
	void *StackPtr = NULL;
	CyU3PReturnStatus_t Status;

	StackPtr = CyU3PMemAlloc(TRX_THREAD_STACK);
	Status = CyU3PThreadCreate(&TRxThreadHandle,	// Handle to my Application Thread
				"42:TRX",								// Thread ID and name
				TRxThread,						// Thread entry function
				0,										// Parameter passed to Thread
				StackPtr,								// Pointer to the allocated thread stack
				TRX_THREAD_STACK,					// Allocated thread stack size
				TRX_THREAD_PRIORITY,				// Thread priority
				TRX_THREAD_PRIORITY,				// = Thread priority so no preemption
				CYU3P_NO_TIME_SLICE,					// Time slice no supported
				CYU3P_AUTO_START						// Start the thread immediately
	);
	return Status;
}
