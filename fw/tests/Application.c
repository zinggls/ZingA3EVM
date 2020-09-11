#include "Application.h"
#include "DebugConsole.h"
#include "i2c_test.h"

CyU3PThread ApplicationThreadHandle;

void ApplicationThread(uint32_t Value)
{
	CyU3PReturnStatus_t Status = InitializeDebugConsole(6);
	CheckStatus("[App] InitializeDebugConsole", Status);

	test_i2c_init();

    CyU3PDebugPrint(4, "All tests are done\n");
}

void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ApplicationThreadHandle,	// Handle to my Application Thread
            "01:tests",										// Thread ID and name
            ApplicationThread,								// Thread entry function
            1,												// Parameter passed to Thread
            StackPtr,										// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,						// Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,					// Thread priority
            APPLICATION_THREAD_PRIORITY,					// Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,							// Time slice no supported
            CYU3P_AUTO_START								// Start the thread immediately
            );
    if (Status != CY_U3P_SUCCESS) while(1);
}

int main (void)
{
	CyU3PIoMatrixConfig_t io_cfg;
	CyU3PReturnStatus_t Status;

    Status = CyU3PDeviceInit(0);

    if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyFalse, CyFalse);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&io_cfg, 0, sizeof(io_cfg));
			io_cfg.isDQ32Bit = CyTrue;
			io_cfg.useUart   = CyTrue;
			io_cfg.useI2C    = CyTrue;
			io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_DEFAULT;
			io_cfg.gpioSimpleEn[0]  = 0;
			io_cfg.gpioSimpleEn[1]  = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&io_cfg);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;	// Won't get here but compiler wants this!
}
