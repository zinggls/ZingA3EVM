#ifndef __ZING_TEST_H__
#define __ZING_TEST_H__

#include "Application.h"
#include "zing.h"
#include "dma.h"
#include "ControlCh.h"

void test_zing_init()
{
	initDma(CY_FX_EP_CONTROL_IN,CY_FX_EP_CONTROL_OUT,CY_FX_EP_DATA_IN,CY_FX_EP_DATA_OUT,CY_FX_DATA_BURST_LENGTH);
	DMA_Sync();

	assert("Control Channel Thread",ControlChThread_Create()==CY_U3P_SUCCESS);
	assert("1st Zing Init",Zing_Init()==CY_U3P_SUCCESS);
	assert("2nd Zing Init",Zing_Init()==CY_U3P_SUCCESS);

	assert("Zing SetHRCP(PPC)",Zing_SetHRCP(PPC)==CY_U3P_SUCCESS);
	assert("Zing GetHRCP",Zing_GetHRCP()==PPC);

	assert("Zing SetHRCP(DEV)",Zing_SetHRCP(DEV)==CY_U3P_SUCCESS);
	assert("Zing GetHRCP",Zing_GetHRCP()==DEV);
}

#endif
