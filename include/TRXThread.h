#ifndef __TRX_THREAD_H__
#define __TRX_THREAD_H__

#include "cyu3types.h"
#include "cyu3os.h"

#define TRX_THREAD_STACK		(0x1000)
#define TRX_THREAD_PRIORITY	(25)

void TRxThread(uint32_t Value);
CyU3PReturnStatus_t TRxThread_Create(void);

#endif
