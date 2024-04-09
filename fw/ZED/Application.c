#include "Application.h"
#include "gitcommit.h"
#include "DebugConsole.h"
#include "USB_Handler.h"
#include "gpif/PIB.h"
#include "i2c.h"
#include <gpio.h>
#include "ZingHw.h"
#include "cyu3error.h"
#include "Zing.h"
#include "dma.h"
#include "ControlCh.h"
#include "USB_EP0.h"
#include "utility.h"

CyBool_t IsApplnActive = CyFalse;		//Whether the application is active or not
uint16_t gDataInCountPrev = 0;

/* This function starts the application. This is called
 * when a SET_CONF event is received from the USB host. The endpoints
 * are configured and the DMA pipe is setup in this function. */
void AppStart(void)
{
	uint16_t size = 0;
	CyU3PEpConfig_t epCfg;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
	CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();

    gUsbSpeed = usbSpeed;

    /* First identify the usb speed. Once that is identified,
     * create a DMA channel and start the transfer on this. */

    /* Based on the Bus Speed configure the endpoint packet size */
	switch(usbSpeed)
	{
	case CY_U3P_FULL_SPEED:
		size = 64;
		break;
	case CY_U3P_HIGH_SPEED:
		size = 512;
		break;
	case  CY_U3P_SUPER_SPEED:
		size = 1024;
		break;
	default:
		CyFxAppErrorHandler("Invalid USB speed",CY_U3P_ERROR_FAILURE);
		break;
	}
	CyU3PDebugPrint (4, "CyU3PUsbGetSpeed = %d\n",usbSpeed);

	CyU3PMemSet((uint8_t *)&epCfg, 0, sizeof (epCfg));
	epCfg.enable = CyTrue;
	epCfg.epType = CY_U3P_USB_EP_BULK;
	epCfg.burstLen = 1;
	epCfg.streams = 0;
	epCfg.pcktSize = size;

	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONTROL_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_CONTROL_OUT)",apiRetStatus);
	}

	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONTROL_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_CONTROL_IN)",apiRetStatus);
    }

	CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
	epCfg.enable = CyTrue;
	epCfg.epType = CY_U3P_USB_EP_BULK;
	epCfg.burstLen = (usbSpeed == CY_U3P_SUPER_SPEED) ? (CY_FX_DATA_BURST_LENGTH) : 1;
	epCfg.streams = 0;
	epCfg.pcktSize = size;

	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_DATA_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_DATA_OUT)",apiRetStatus);
	}

	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_DATA_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_DATA_IN)",apiRetStatus);
	}

	/* Update the status flag. */
	IsApplnActive = CyTrue;
}

/* This function stops the application. This shall be called whenever
 * a RESET or DISCONNECT event is received from the USB host. The endpoints are
 * disabled and the DMA pipe is destroyed by this function. */
void AppStop(void)
{
	CyU3PEpConfig_t epCfg;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	/* Update the flag. */
	IsApplnActive = CyFalse;

	/* Flush the endpoint memory */
	CyU3PUsbFlushEp(CY_FX_EP_CONTROL_OUT);
	CyU3PUsbFlushEp(CY_FX_EP_CONTROL_IN);
	CyU3PUsbFlushEp(CY_FX_EP_DATA_OUT);
	CyU3PUsbFlushEp(CY_FX_EP_DATA_IN);

	/* Disable endpoints. */
	CyU3PMemSet((uint8_t *)&epCfg, 0, sizeof (epCfg));
	epCfg.enable = CyFalse;

	/* Control OUT: Producer endpoint configuration. */
	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONTROL_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_CONTROL_OUT)",apiRetStatus);
	}

    /* Control IN: Consumer endpoint configuration. */
	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONTROL_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_CONTROL_IN)",apiRetStatus);
    }

	/* Data OUT: Producer endpoint configuration */
	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_DATA_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
    {
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_DATA_OUT)",apiRetStatus);
    }

	/* Data IN: Consumer endpoint configuration */
	apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_DATA_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		CyFxAppErrorHandler("CyU3PSetEpConfig(CY_FX_EP_DATA_IN)",apiRetStatus);
	}
}

static char RunStatus()
{
    if(Dma.DataOut_.Count_ == gDataInCountPrev)
        return 'N';
    else{
        gDataInCountPrev = Dma.DataOut_.Count_;
        return 'Y';
    }
}

void ApplicationThread(uint32_t Value)
{
	CheckStatus("[App] InitializeDebugConsole", InitializeDebugConsole(6,NULL));

	CyU3PDebugPrint(4,"[App] Git:%s\n",GIT_INFO);

	CheckStatus("[App] USBEP0RxThread_Create", USBEP0RxThread_Create());
	CheckStatus("[App] SetupGPIO", SetupGPIO());
	CheckStatus("[App] I2C_Init", I2C_Init());
	CheckStatus("[App] USB_Init", USB_Init());
	CheckStatus("[App] PIB_Init", PIB_Init());

	initDma(CY_FX_EP_CONTROL_IN,CY_FX_EP_CONTROL_OUT,CY_FX_EP_DATA_IN,CY_FX_EP_DATA_OUT,CY_FX_DATA_BURST_LENGTH);
	CheckStatus("[App] DMA_Sync",DMA_Sync());

	CheckStatus("[App] ControlChThread_Create", ControlChThread_Create());
	CheckStatus("[App] Zing_Init", Zing_Init());

#ifdef HRCP_PPC
	CheckStatus("[App] Zing_SetHRCP(PPC)",Zing_SetHRCP(PPC));
#else
	CheckStatus("[App] Zing_SetHRCP(DEV)",Zing_SetHRCP(DEV));
#endif

	CheckStatus("[App] USB_Connect", USB_Connect());

	while(IsApplnActive == 0) {
		CyU3PThreadSleep(100);
	}

	CheckStatus("[App] DMA_Normal_DataOnly",DMA_Normal_DataOnly());
	CyU3PDebugPrint(4,"[App] DMA Normal DataOnly mode uses ");
#ifdef DMA_NORMAL_MANUAL
	CyU3PDebugPrint(4,"Manual mode\n");
#else
	CyU3PDebugPrint(4,"Auto Signal mode\n");
#endif

	char runStatusVal;
	char transType,ack,ppc;
	uint32_t ppidVal,deviceIdVal,rxidVal,txidVal;

	ppidVal = ppid();
	deviceIdVal = deviceID();
	while (1)
	{
        transType = TransferType(); //Transfer Type (1: Isochronous 2: Bulk)
        ack = AckMode();        //Ack mode (0:No Ack 1:Ack)
        ppc = ppcMode();        //PPC or DEV (0: DEV 1:PPC)
        runStatusVal = RunStatus();
        rxidVal = rx_id();
        txidVal = tx_id();

        CyU3PDebugPrint (4, "ZED USB:%d PPID:0x%x DeviceID:0x%x TRT:%c ACK:%c PPC:%c TXID:0x%x RXID:0x%x RUN:%c CNT:%d \r\n",
                gUsbSpeed,ppidVal,deviceIdVal,transType,ack,ppc,txidVal,rxidVal,runStatusVal,Dma.DataOut_.Count_);

		CyU3PThreadSleep(100);
	}

	while (1);	// Hang here
}
