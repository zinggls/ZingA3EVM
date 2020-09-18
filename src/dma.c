#include "dma.h"
#include "cyu3error.h"
#include "cyu3usb.h"
#include "DebugConsole.h"

static void initDmaCount()
{
	Dma.Mode_= DMA_UNINITIALIZED;
	Dma.DataOutCount_ = 0;
	Dma.DataInCount_ = 0;
	Dma.ControlOutCount_ = 0;
	Dma.ControlInCount_ = 0;
}

void initDma(uint8_t controlInEP,uint8_t controlOutEP,uint8_t dataInEP,uint8_t dataOutEP,uint16_t dataBurstLength)
{
	initDmaCount();
	Dma.ControlInEP_ = controlInEP;
	Dma.ControlOutEP_ = controlOutEP;
	Dma.DataInEP_ = dataInEP;
	Dma.DataOutEP_ = dataOutEP;
	Dma.DataBurstLength_ = dataBurstLength;
}

void setDmaChannelCfg(CyU3PDmaChannelConfig_t *pDmaCfg, uint16_t size, uint16_t count, CyU3PDmaSocketId_t prodSckId,
		CyU3PDmaSocketId_t consSckId, uint32_t notification, CyU3PDmaCallback_t cb)
{
	pDmaCfg->size  = size;
	pDmaCfg->count = count;
	pDmaCfg->prodSckId = prodSckId;
	pDmaCfg->consSckId = consSckId;
	pDmaCfg->dmaMode = CY_U3P_DMA_MODE_BYTE;
	pDmaCfg->notification = notification;
	pDmaCfg->cb = cb;
	pDmaCfg->prodHeader = 0;
	pDmaCfg->prodFooter = 0;
	pDmaCfg->consHeader = 0;
	pDmaCfg->prodAvailCount = 0;
}

CyU3PReturnStatus_t createChannel(const char* name,
		CyU3PDmaChannelConfig_t *pDmaCfg,uint16_t size,uint16_t count,CyU3PDmaSocketId_t prodSckId,
		CyU3PDmaSocketId_t consSckId,uint32_t notification,CyU3PDmaCallback_t cb,
		CyU3PDmaChannel *handle,CyU3PDmaType_t type)
{
	char msg[256]={0,};
	memcpy(msg,name,strlen(name));

	if(strcmp(name,"DmaNormal.DataIn")==0) {
#if PACKET_SUSPEND == 1
		notification |= CY_U3P_DMA_CB_PROD_SUSP | CY_U3P_DMA_CB_CONS_SUSP;
#endif
	}

	setDmaChannelCfg(pDmaCfg, size, count, prodSckId, consSckId, notification, cb);
	CHECK(CyU3PDmaChannelCreate(handle, type, pDmaCfg));

	if(strcmp(name,"DmaNormal.DataIn")==0) {
#if PACKET_SUSPEND == 1
		CyU3PDmaChannelSetSuspend(&handle,CY_U3P_DMA_SCK_SUSP_EOP,CY_U3P_DMA_SCK_SUSP_EOP);
#endif
	}

	CHECK(CyU3PDmaChannelSetXfer(handle, 0));	//Set DMA Channel transfer size to INFINITE
	return CY_U3P_SUCCESS;
}

void channelReset(uint8_t controlIn,uint8_t controlOut,uint8_t dataIn,uint8_t dataOut)
{
	//Abort & destroy DMAs
	CyU3PDmaChannelAbort(&Dma.ControlOut_);
	CyU3PDmaChannelAbort(&Dma.ControlIn_);
	CyU3PDmaChannelAbort(&Dma.DataOut_);
	CyU3PDmaChannelAbort(&Dma.DataIn_);

	CyU3PDmaChannelDestroy(&Dma.ControlOut_);
	CyU3PDmaChannelDestroy(&Dma.ControlIn_);
	CyU3PDmaChannelDestroy(&Dma.DataOut_);
	CyU3PDmaChannelDestroy(&Dma.DataIn_);

	//Flush the Endpoint memory
	CyU3PUsbFlushEp(controlIn);
	CyU3PUsbFlushEp(controlOut);
	CyU3PUsbFlushEp(dataIn);
	CyU3PUsbFlushEp(dataOut);
}

static CyU3PReturnStatus_t DMA_Sync_mode(uint8_t controlIn,uint8_t controlOut,uint8_t dataIn,uint8_t dataOut,uint16_t dataBurstLength)
{
	CyU3PDmaChannelConfig_t dmaCfg;
	uint16_t size = 1024; // super speed <- assumed condition , temporary code

	channelReset(controlIn,controlOut,dataIn,dataOut);

	CHECK(createChannel("DmaSync.ControlOut",
						&dmaCfg,
						size,
						8,
						CY_U3P_CPU_SOCKET_PROD,
						CY_U3P_PIB_SOCKET_0,
						CY_U3P_DMA_CB_PROD_EVENT,
						0,
						&Dma.ControlOut_,
						CY_U3P_DMA_TYPE_MANUAL_OUT));

	CHECK(createChannel("DmaSync.ControlIn",
						&dmaCfg,
						size,
						8,
						CY_U3P_PIB_SOCKET_1,
						CY_U3P_CPU_SOCKET_CONS,
						CY_U3P_DMA_CB_PROD_EVENT,
						0,
						&Dma.ControlIn_,
						CY_U3P_DMA_TYPE_MANUAL_IN));

	CHECK(createChannel("DmaSync.DataOut",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_CPU_SOCKET_PROD,
						CY_U3P_PIB_SOCKET_2,
						CY_U3P_DMA_CB_PROD_EVENT,
						0,
						&Dma.DataOut_,
						CY_U3P_DMA_TYPE_MANUAL_OUT));

	CHECK(createChannel("DmaSync.DataIn",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_PIB_SOCKET_3,
						CY_U3P_CPU_SOCKET_CONS,
						CY_U3P_DMA_CB_PROD_EVENT,
						0,
						&Dma.DataIn_,
						CY_U3P_DMA_TYPE_MANUAL_IN));

	Dma.Mode_ = DMA_SYNC;
#ifdef DEBUG
	CyU3PDebugPrint(4,"DMA_Sync_mode(%d) done\n", Dma.Mode_);
#endif
	return CY_U3P_SUCCESS;
}

void DMA_Normal_CtrlOut_Cb(CyU3PDmaChannel *handle,CyU3PDmaCbType_t evtype,CyU3PDmaCBInput_t *input)
{
	switch (evtype)
	{
	case CY_U3P_DMA_CB_PROD_EVENT:
		Dma.ControlOutCount_++;
		break;
	case CY_U3P_DMA_CB_SEND_CPLT:	//override mode
		break;
	case CY_U3P_DMA_CB_CONS_EVENT:	//normal mode
		break;
	default:
		break;
	}
}

void DMA_Normal_CtrlIn_Cb(CyU3PDmaChannel *handle,CyU3PDmaCbType_t evtype,CyU3PDmaCBInput_t *input)
{
	switch (evtype)
	{
	case CY_U3P_DMA_CB_PROD_EVENT:
		Dma.ControlInCount_++;
		break;
	case CY_U3P_DMA_CB_SEND_CPLT:	//override mode
		break;
	case CY_U3P_DMA_CB_CONS_EVENT:	//normal mode
		break;
	default:
		break;
	}
}

void DMA_Normal_DataOut_Cb(CyU3PDmaChannel *handle,CyU3PDmaCbType_t evtype,CyU3PDmaCBInput_t *input)
{
	switch (evtype)
	{
	case CY_U3P_DMA_CB_PROD_EVENT:
		Dma.DataOutCount_++;
		break;
	case CY_U3P_DMA_CB_SEND_CPLT:	//override mode
		break;
	case CY_U3P_DMA_CB_CONS_EVENT:	//normal mode
		break;
	default:
		break;
	}
}

void DMA_Normal_DataIn_Cb(CyU3PDmaChannel *handle,CyU3PDmaCbType_t evtype,CyU3PDmaCBInput_t *input)
{
	switch (evtype)
	{
	case CY_U3P_DMA_CB_PROD_EVENT:
		Dma.DataInCount_++;
		break;
	case CY_U3P_DMA_CB_SEND_CPLT:	//override mode
		break;
	case CY_U3P_DMA_CB_CONS_EVENT:	//normal mode
		break;
#if PACKET_SUSPEND == 1
	case CY_U3P_DMA_CB_PROD_SUSP:
		CyU3PDmaChannelResume(handle,CyTrue,CyTrue);
		break;
	case CY_U3P_DMA_CB_CONS_SUSP:
		CyU3PDmaChannelResume(handle,CyTrue,CyTrue);
		break;
#endif
	default:
		break;
	}
}

static CyU3PReturnStatus_t DMA_Normal_mode(uint8_t controlIn,uint8_t controlOut,uint8_t dataIn,uint8_t dataOut,uint16_t dataBurstLength)
{
	CyU3PDmaChannelConfig_t dmaCfg;
	uint16_t size = 1024; // super speed <- assumed condition , temporary code

	channelReset(controlIn,controlOut,dataIn,dataOut);

	CHECK(createChannel("DmaNormal.ControlOut",
						&dmaCfg,
						size,
						8,
						CY_U3P_UIB_SOCKET_PROD_1,
						CY_U3P_PIB_SOCKET_0,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_Normal_CtrlOut_Cb,
						&Dma.ControlOut_,
						CY_U3P_DMA_TYPE_AUTO_SIGNAL));

	CHECK(createChannel("DmaNormal.ControlIn",
						&dmaCfg,
						size,
						8,
						CY_U3P_PIB_SOCKET_1,
						CY_U3P_UIB_SOCKET_CONS_1,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_Normal_CtrlIn_Cb,
						&Dma.ControlIn_,
						CY_U3P_DMA_TYPE_AUTO_SIGNAL));

	CHECK(createChannel("DmaNormal.DataOut",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_UIB_SOCKET_PROD_2,
						CY_U3P_PIB_SOCKET_2,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_Normal_DataOut_Cb,
						&Dma.DataOut_,
						CY_U3P_DMA_TYPE_AUTO_SIGNAL));

	CHECK(createChannel("DmaNormal.DataIn",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_PIB_SOCKET_3,
						CY_U3P_UIB_SOCKET_CONS_2,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_Normal_DataIn_Cb,
						&Dma.DataIn_,
						CY_U3P_DMA_TYPE_AUTO_SIGNAL));

	Dma.Mode_ = DMA_NORMAL;
#ifdef DEBUG
	CyU3PDebugPrint(4,"DMA_Normal_mode(%d) done\n", Dma.Mode_);
#endif
	return CY_U3P_SUCCESS;
}

static CyU3PReturnStatus_t DMA_LoopBack_mode(uint8_t controlIn,uint8_t controlOut,uint8_t dataIn,uint8_t dataOut,uint16_t dataBurstLength)
{
	CyU3PDmaChannelConfig_t dmaCfg;
	uint16_t size = 1024; // super speed <- assumed condition , temporary code

	channelReset(controlIn,controlOut,dataIn,dataOut);

	CHECK(createChannel("DmaLoopback.ControlOut",
						&dmaCfg,
						size,
						8,
						CY_U3P_UIB_SOCKET_PROD_1,
						CY_U3P_UIB_SOCKET_CONS_1,
						0,
						0,
						&Dma.ControlOut_,
						CY_U3P_DMA_TYPE_AUTO));

	CHECK(createChannel("DmaLoopback.DataOut",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_UIB_SOCKET_PROD_2,
						CY_U3P_UIB_SOCKET_CONS_2,
						0,
						0,
						&Dma.DataOut_,
						CY_U3P_DMA_TYPE_AUTO));

	Dma.Mode_ = DMA_LP;
#ifdef DEBUG
	CyU3PDebugPrint(4,"DMA_LoopBack_mode(%d) done\n", Dma.Mode_);
#endif
	return CY_U3P_SUCCESS;
}

void DMA_SinkSource_Cb(CyU3PDmaChannel *chHandle,CyU3PDmaCbType_t type,CyU3PDmaCBInput_t *input)
{
	CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
	CyU3PDmaBuffer_t buf_p;

	Dma.DataOutCount_++;

	if (type == CY_U3P_DMA_CB_PROD_EVENT)
	{
		status = CyU3PDmaChannelDiscardBuffer(chHandle);
		if (status != CY_U3P_SUCCESS)
		{
			CyU3PDebugPrint(4, "DMA_SinkSource_Cb,CyU3PDmaChannelDiscardBuffer failed(0x%x)\n", status);
		}
	}
	if (type == CY_U3P_DMA_CB_CONS_EVENT)
	{
		status = CyU3PDmaChannelGetBuffer(chHandle, &buf_p, CYU3P_NO_WAIT);
		if (status == CY_U3P_SUCCESS)
		{
			status = CyU3PDmaChannelCommitBuffer(chHandle, buf_p.size, 0);	//Commit the full buffer with default status
			if (status != CY_U3P_SUCCESS)
			{
				CyU3PDebugPrint(4, "DMA_SinkSource_Cb,CyU3PDmaChannelCommitBuffer failed(0x%x)\n", status);
			}
		}
		else
		{
			CyU3PDebugPrint(4, "DMA_SinkSource_Cb,CyU3PDmaChannelGetBuffer failed(0x%x)\n", status);
		}
	}
}

CyU3PReturnStatus_t DMASrcSinkFillInBuffers(void)
{
	CyU3PDmaBuffer_t    buf_p;
	uint16_t            index = 0;

	/* Now preload all buffers in the MANUAL_OUT pipe with the required data. */
	for (index = 0; index < 8; index++)
	{
		CHECK(CyU3PDmaChannelGetBuffer(&Dma.ControlIn_, &buf_p, CYU3P_NO_WAIT));

		CyU3PMemSet(buf_p.buffer, 0xA5, buf_p.size);
		CHECK(CyU3PDmaChannelCommitBuffer(&Dma.ControlIn_, buf_p.size, 0));
	}

	/* Now preload all buffers in the MANUAL_OUT pipe with the required data. */
	for (index = 0; index < 4; index++)
	{
		CHECK(CyU3PDmaChannelGetBuffer(&Dma.DataIn_, &buf_p, CYU3P_NO_WAIT));

		CyU3PMemSet(buf_p.buffer, 0xA5, buf_p.size);
		CHECK(CyU3PDmaChannelCommitBuffer(&Dma.DataIn_, buf_p.size, 0));
	}
	return CY_U3P_SUCCESS;
}

static CyU3PReturnStatus_t DMA_SinkSource_mode(uint8_t controlIn,uint8_t controlOut,uint8_t dataIn,uint8_t dataOut,uint16_t dataBurstLength)
{
	CyU3PDmaChannelConfig_t dmaCfg;
	uint16_t size = 1024; // super speed <- assumed condition , temporary code

	channelReset(controlIn,controlOut,dataIn,dataOut);

	CHECK(createChannel("DmaSinkSource.ControlOut",
						&dmaCfg,
						size,
						8,
						CY_U3P_UIB_SOCKET_PROD_1,
						CY_U3P_CPU_SOCKET_CONS,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_SinkSource_Cb,
						&Dma.ControlOut_,
						CY_U3P_DMA_TYPE_MANUAL_IN));

	CHECK(createChannel("DmaSinkSource.ControlIn",
						&dmaCfg,
						size,
						8,
						CY_U3P_CPU_SOCKET_PROD,
						CY_U3P_UIB_SOCKET_CONS_1,
						CY_U3P_DMA_CB_CONS_EVENT,
						DMA_SinkSource_Cb,
						&Dma.ControlIn_,
						CY_U3P_DMA_TYPE_MANUAL_OUT));

	CHECK(createChannel("DmaSinkSource.DataOut",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_UIB_SOCKET_PROD_2,
						CY_U3P_CPU_SOCKET_CONS,
						CY_U3P_DMA_CB_PROD_EVENT,
						DMA_SinkSource_Cb,
						&Dma.DataOut_,
						CY_U3P_DMA_TYPE_MANUAL_IN));

	CHECK(createChannel("DmaSinkSource.DataIn",
						&dmaCfg,
						size*dataBurstLength,
						4,
						CY_U3P_CPU_SOCKET_PROD,
						CY_U3P_UIB_SOCKET_CONS_2,
						CY_U3P_DMA_CB_CONS_EVENT,
						DMA_SinkSource_Cb,
						&Dma.DataIn_,
						CY_U3P_DMA_TYPE_MANUAL_OUT));

	CHECK(DMASrcSinkFillInBuffers());

	Dma.Mode_ = DMA_SINKSOURCE;
#ifdef DEBUG
	CyU3PDebugPrint(4,"DMA_SinkSource_mode(%d) done\n", Dma.Mode_);
#endif
	return CY_U3P_SUCCESS;
}

CyU3PReturnStatus_t DMA_Sync()
{
	return DMA_Sync_mode(Dma.ControlInEP_,Dma.ControlOutEP_,Dma.DataInEP_,Dma.DataOutEP_,Dma.DataBurstLength_);
}

CyU3PReturnStatus_t DMA_Normal()
{
	return DMA_Normal_mode(Dma.ControlInEP_,Dma.ControlOutEP_,Dma.DataInEP_,Dma.DataOutEP_,Dma.DataBurstLength_);
}

CyU3PReturnStatus_t DMA_LoopBack()
{
	return DMA_LoopBack_mode(Dma.ControlInEP_,Dma.ControlOutEP_,Dma.DataInEP_,Dma.DataOutEP_,Dma.DataBurstLength_);
}

CyU3PReturnStatus_t DMA_SinkSource()
{
	return DMA_SinkSource_mode(Dma.ControlInEP_,Dma.ControlOutEP_,Dma.DataInEP_,Dma.DataOutEP_,Dma.DataBurstLength_);
}
