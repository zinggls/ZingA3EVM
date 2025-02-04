#include "Application.h"
#include "gitcommit.h"
#include "git_describe.h"
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
extern CyU3PDmaChannel glChHandleUARTtoCPU;
static uint8_t regBuf[512];

void uartIntrCb(CyU3PUartEvt_t evt, CyU3PUartError_t error)
{
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
    CyU3PDmaBuffer_t inBuf;
    uint8_t *buf;

    if (evt == CY_U3P_UART_EVENT_RX_DONE)
    {
        Status = CyU3PDmaChannelSetWrapUp (&glChHandleUARTtoCPU);
        if(Status != CY_U3P_SUCCESS) CyU3PDebugPrint(4, "CyU3PDmaChannelSetWrapUp error=0x%x \n\r",Status);

        Status = CyU3PDmaChannelGetBuffer (&glChHandleUARTtoCPU, &inBuf, CYU3P_NO_WAIT);
        if(Status != CY_U3P_SUCCESS) CyU3PDebugPrint(4, "CyU3PDmaChannelGetBuffer error=0x%x \n\r",Status);

        buf = inBuf.buffer;
        CyU3PDebugPrint(4, "[%d] ",inBuf.count);
        for(int i=0;i<inBuf.count;i++) CyU3PDebugPrint(4,"%x ",buf[i]);
        CyU3PDebugPrint(4,"\n\r");

        if(buf[0]==0x4 && buf[1]==0x66 && buf[3]==0x0) {    //ex. 4f10  4byte f:formatIndex 1:index 0:padding
            CyU3PDebugPrint(4,"gFormatIndex = \n\r");
        }

        if(buf[0]==0x4 && buf[1]==0x72 && buf[3]==0x0) {    //ex. 4r10  4byte r:frameIndex 1:index 0:padding
            CyU3PDebugPrint(4,"gFrameIndex = \n\r");
        }

        if(buf[0]==0x4 && buf[1]==0x62 && buf[3]==0x0) {    //ex. 4b10  4byte b:band (0:low or 1:high) 0:padding
            CyU3PDebugPrint(4,"band = %d\n\r",buf[2]);
            if(buf[2]==0) {
                //low band
                char b = setBand('L');
                if(b=='L') CyU3PDebugPrint(4,"Low band set\n\r");
            }else if(buf[2]==1) {
                //High band
                char b = setBand('H');
                if(b=='H') CyU3PDebugPrint(4,"High band set\n\r");
            }else{
                //Undefined
                CyU3PDebugPrint(4,"Undefined band value=0x%x\n\r",buf[2]);
            }
        }

        if(buf[0]==0x4 && buf[1]==0x72 && buf[2]==0x73 && buf[3]==0x74) {
            //Device Reset
            CyU3PDeviceReset(CyFalse);
        }

        if(buf[0]==0x4 && buf[1]==0x70) {   //ex. 4pABCD  4byte p:PPID ABCD:PPID 2bytes
            uint32_t reg_val = (buf[2]<<8)|buf[3];
            Status=Zing_RegWrite(REG_PPID,(uint8_t*)&reg_val,4);
            if(Status==CY_U3P_SUCCESS)
                CyU3PDebugPrint(4,"PPID set(0x%x)\n\r",reg_val);
            else
                CyU3PDebugPrint(4,"PPID set(0x%x) error(0x%x)\n\r",reg_val,Status);
        }

        CyU3PDmaChannelDiscardBuffer (&glChHandleUARTtoCPU);
        CyU3PUartRxSetBlockXfer (UART_RX_UNIT);
    }
}

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
	CheckStatus("[App] InitializeDebugConsole", InitializeDebugConsole(8,uartIntrCb));

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

    CheckStatus("[App] Zing_RegRead",Zing_RegRead(0x8000, (uint8_t*)regBuf, sizeof(regBuf)));
    printRegisters(regBuf);

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
	char transType,ack,ppc,bnd;
	uint32_t ppidVal,deviceIdVal,rxidVal,txidVal;
	uint16_t loopCount = 0;
	while (1)
	{
        ppidVal = ppid();
        deviceIdVal = deviceID();
        transType = TransferType(); //Transfer Type (1: Isochronous 2: Bulk)
        ack = AckMode();        //Ack mode (0:No Ack 1:Ack)
        ppc = ppcMode();        //PPC or DEV (0: DEV 1:PPC)
        runStatusVal = RunStatus();
        rxidVal = rx_id();
        txidVal = tx_id();
        bnd = band();

        CyU3PDebugPrint (4, "ZED USB:%d BND:%c PPID:0x%x DeviceID:0x%x TRT:%c ACK:%c PPC:%c TXID:0x%x RXID:0x%x RUN:%c CNT:%d \r\n",
                gUsbSpeed,bnd,ppidVal,deviceIdVal,transType,ack,ppc,txidVal,rxidVal,runStatusVal,Dma.DataOut_.Count_);

		CyU3PThreadSleep(100);

        if(loopCount%10==0) CyU3PDebugPrint(4,"GIT DESCRIBE:%s GIT_INFO:%s \r\n",GIT_DESCRIBE,GIT_INFO);
        loopCount++;
	}

	while (1);	// Hang here
}
