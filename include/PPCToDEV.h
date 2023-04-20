#ifndef __PPC_To_DEV_H__
#define __PPC_To_DEV_H__

#include "cyu3types.h"
#include "cyu3os.h"

#define PPCTODEV_THREAD_STACK		(0x1000)
#define PPCTODEV_THREAD_PRIORITY	(12)
#define EVT_PPCTODEV0				(1 << 1)

typedef struct P2DCtx_t{
	CyU3PEvent Event_;
	uint8_t Data_[512];
	uint32_t Data_idx_;
	uint32_t MngtData_;
}P2DCtx_t;

P2DCtx_t P2DCtx;

void PPCToDEVThread(uint32_t Value);
CyU3PReturnStatus_t PPCToDEVThread_Create(void);

#endif
