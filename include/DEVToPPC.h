#ifndef __DEV_To_PPC_H__
#define __DEV_To_PPC_H__

#include "cyu3types.h"
#include "cyu3os.h"

#define DEVTOPPC_THREAD_STACK		(0x1000)
#define DEVTOPPC_THREAD_PRIORITY	(15)
#define EVT_DEVTOPPC0				(1 << 1)

typedef struct D2PCtx_t{
	CyU3PEvent Event_;
	uint8_t Data_[512];
	uint32_t Data_idx_;
	uint32_t MngtData_;
}D2PCtx_t;

D2PCtx_t D2PCtx;

void DEVToPPCThread(uint32_t Value);
CyU3PReturnStatus_t DEVToPPCThread_Create(void);

#endif
