#include "gpio.h"
#include "cyu3error.h"
#include "cyfx3_api.h"
#include "cyu3system.h"

int main (void)
{
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
    CyU3PSysClockConfig_t clkCfg;

	/* setSysClk400 clock configurations */
	clkCfg.setSysClk400 = CyTrue;   /* FX3 device's master clock is set to a frequency > 400 MHz */
	clkCfg.cpuClkDiv = 2;           /* CPU clock divider */
	clkCfg.dmaClkDiv = 2;           /* DMA clock divider */
	clkCfg.mmioClkDiv = 2;          /* MMIO clock divider */
	clkCfg.useStandbyClk = CyFalse; /* device has no 32KHz clock supplied */
	clkCfg.clkSrc = CY_U3P_SYS_CLK; /* Clock source for a peripheral block  */
    Status = CyU3PDeviceInit(&clkCfg);

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
			io_cfg.gpioSimpleEn[1]  = 1<<(GPIO57-32); // TP2 in schematic
			Status = CyU3PDeviceConfigureIOMatrix(&io_cfg);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;	// Won't get here but compiler wants this!
}


