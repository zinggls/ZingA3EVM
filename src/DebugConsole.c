#include "DebugConsole.h"
#include "cyu3error.h"
#include "macro.h"

CyU3PDmaChannel glChHandleUARTtoCPU;
static CyBool_t DebugTxEnabled = CyTrue;	// Set true once I can output messages to the Console

void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status)
{
	if (DebugTxEnabled)				// Need to wait until ConsoleOut is enabled
	{
		if (Status == CY_U3P_SUCCESS) CyU3PDebugPrint(4, "%s OK\n", StringPtr);
		else
		{
			// else hang here
			CyU3PDebugPrint(4, "\r\n%s failed, Status = %d\n", StringPtr, Status);
			while (1)
			{
				CyU3PDebugPrint(4, "?");
				CyU3PThreadSleep (1000);
			}
		}
	}
}

void CyFxAppErrorHandler(char* StringPtr,CyU3PReturnStatus_t Status)
{
	for (;;)
	{
		if(DebugTxEnabled) CyU3PDebugPrint(4,"Error in %s(0x%x)\n",StringPtr,Status);
		CyU3PThreadSleep(100);
	}
}

CyU3PReturnStatus_t InitializeDebugConsole(uint8_t TraceLevel,CyU3PUartIntrCb_t UartCallback)
{
	CyU3PUartConfig_t uartConfig;
	CyU3PDmaChannelConfig_t dmaCfg;

	CHECK(CyU3PUartInit());											// Start the UART driver

	CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
	uartConfig.parity   = CY_U3P_UART_NO_PARITY;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyTrue;
	uartConfig.flowCtrl = CyFalse;
	uartConfig.isDma    = CyTrue;

	CHECK(CyU3PUartSetConfig(&uartConfig, UartCallback));			// Configure the UART hardware
	CHECK(CyU3PUartTxSetBlockXfer(0xFFFFFFFF));						// Send as much data as I need to
	CHECK(CyU3PUartRxSetBlockXfer(UART_RX_UNIT));
	CHECK(CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, TraceLevel));	// Attach the Debug driver above the UART driver

    CyU3PDebugPreamble (CyFalse);

    CyU3PMemSet ((uint8_t *)&dmaCfg, 0, sizeof(dmaCfg));
    dmaCfg.size = 16;
    dmaCfg.count = 1;
    dmaCfg.prodSckId = CY_U3P_LPP_SOCKET_UART_PROD;
    dmaCfg.consSckId = CY_U3P_CPU_SOCKET_CONS;
    dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;
    /* Enabling the callback for produce event. */
    dmaCfg.notification = CY_U3P_DMA_CB_PROD_EVENT;
    dmaCfg.cb = 0;
    CHECK(CyU3PDmaChannelCreate (&glChHandleUARTtoCPU, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaCfg));
    CHECK(CyU3PDmaChannelSetXfer (&glChHandleUARTtoCPU, 0));
	return CY_U3P_SUCCESS;
}
