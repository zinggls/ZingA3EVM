#ifndef __CONTROL_CH_H__
#define __CONTROL_CH_H__

#include "cyu3types.h"
#include "cyu3os.h"

#define CONTROLCH_THREAD_STACK		(0x1000)
#define CONTROLCH_THREAD_PRIORITY	(8)
#define EVT_CTLCH0					(1 << 1)

#define CONTROLCH_TX_THREAD_STACK		(0x1000)
#define CONTROLCH_TX_THREAD_PRIORITY	(8)

#define CONTROLCH_ZingTX_THREAD_STACK		(0x1000)
#define CONTROLCH_ZingTX_THREAD_PRIORITY	(8)

#define CONTROLCH_ZingRX_THREAD_STACK		(0x1000)
#define CONTROLCH_ZingRX_THREAD_PRIORITY	(8)

typedef struct ConChCtx_t{
	CyU3PEvent Event_;
	uint8_t Data_[512];
	uint32_t Data_idx_;
	uint32_t MngtData_;
}ConChCtx_t;

ConChCtx_t CcCtx;

void ControlChThread(uint32_t Value);
CyU3PReturnStatus_t ControlChThread_Create(void);

void ControlCh_TX_Thread(uint32_t Value);
CyU3PReturnStatus_t ControlCh_TX_Thread_Create(void);

void ControlCh_ZingTX_Thread(uint32_t Value);
void ControlCh_ZingRX_Thread(uint32_t Value);
CyU3PReturnStatus_t ControlCh_ZingTX_Thread_Create(void);
CyU3PReturnStatus_t ControlCh_ZingRX_Thread_Create(void);

#endif
