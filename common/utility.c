/*
 * utility.c
 *
 *  Created on: 2022. 11. 8.
 *      Author: rossi
 */
#include "utility.h"
#include "cyu3system.h"
#include "Zing.h"
#include "macro.h"
#include "ZingHw.h"

void shrinkPrint(uint8_t* buffer, uint32_t bufferSize)
{
    uint32_t shrink = bufferSize;
    if(bufferSize>16) shrink = 16;

    for(uint32_t i=0;i<shrink;i++) CyU3PDebugPrint(4,"%x ",buffer[i]);
    if(bufferSize>16) {
    	CyU3PDebugPrint(4,"...");
        for(uint32_t i=12;i>0;i--) CyU3PDebugPrint(4,"%x ",buffer[bufferSize-i]);
    }
    CyU3PDebugPrint(4,"\n");
}

int IsSync(uint8_t byte0,uint8_t byte1)
{
	if(byte0==0x0C && ((byte1&0xfc)==0x8c)) return 1;
    return 0;
}

int Loc(uint8_t* buffer, uint32_t bufferSize, uint32_t* location)
{
    for(uint32_t i=0;i<bufferSize-1;i++) {
        if(IsSync(buffer[i],buffer[i+1])) {
            *location = i;
            return 1;
        }
    }
    return 0;
}

int Locs(uint8_t* buffer, uint32_t bufferSize, uint32_t location[])
{
    int count = 0;
    for(uint32_t i=0;i<bufferSize-1;i++) {
        if(IsSync(buffer[i],buffer[i+1])) {
            location[count] = i;
            count++;
        }
    }
    location[count] = bufferSize;
    return count;
}

int SyncCount(uint8_t* buffer, uint32_t bufferSize)
{
    int count = 0;
    for(uint32_t i=0;i<bufferSize-1;i++) {
        if(IsSync(buffer[i],buffer[i+1])) count++;
    }
    return count;
}

void
Set1Byte(
		uint8_t* buf,
		uint8_t val)
{
	*buf = val;
}

void
Set2Byte(
		uint8_t* buf,
		uint16_t val)
{
	buf[0] = val&0xff;
	buf[1] = (val&0xff00)>>8;
}

void
Set4Byte(
		uint8_t* buf,
		uint32_t val)
{
	buf[0] = val&0xff;
	buf[1] = (val&0xff00)>>8;
	buf[2] = (val&0xff0000)>>16;
	buf[3] = (val&0xff000000)>>24;
}

void
SetBuffer(
		uint8_t* buf,
		uint16_t bmHint,
		uint8_t bFormatIndex,
		uint8_t bFrameIndex,
		uint32_t dwFrameInterval,
		uint16_t wKeyFrameRate,
		uint16_t wPFrameRate,
		uint16_t wCompQuality,
		uint16_t wCompWindowSize,
		uint16_t wDelay,
		uint32_t dwMaxVideoFrameSize,
		uint32_t dwMaxPayloadTransferSize)
{
	Set2Byte(buf,bmHint);
	Set1Byte(buf+2 ,bFormatIndex);
	Set1Byte(buf+3 ,bFrameIndex);
	Set4Byte(buf+4 ,dwFrameInterval);
	Set2Byte(buf+8 ,wKeyFrameRate);
	Set2Byte(buf+10,wPFrameRate);
	Set2Byte(buf+12,wCompQuality);
	Set2Byte(buf+14,wCompWindowSize);
	Set2Byte(buf+16,wDelay);
	Set4Byte(buf+18,dwMaxVideoFrameSize);
	Set4Byte(buf+22,dwMaxPayloadTransferSize);
}

char
TransferType()
{
#ifdef ISOC
    return 'I';   //Isochronous
#else
    return 'B';   //Bulk
#endif
}

char
AckMode()
{
#ifdef NO_ACK
    return 'N';   //No Ack
#else
    return 'Y';   //Ack
#endif
}

char
ppcMode()
{
#ifdef HRCP_PPC
    return 'P';   //PPC
#else
    return 'D';   //DEV
#endif
}

uint32_t ppid()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8003, (uint8_t*)&reg_val, 4));
    return reg_val;
}

uint16_t deviceID()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8004, (uint8_t*)&reg_val, 4));
    return reg_val&0xffff;
}

uint32_t tx_id()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8042, (uint8_t*)&reg_val, 4));
    return reg_val;
}

uint32_t tx_id_rb(uint8_t* regBuf)
{
    uint32_t* pVal = (uint32_t*)(regBuf+ (sizeof(uint32_t)*0x42));
    return *pVal;
}

uint32_t rx_id()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8047, (uint8_t*)&reg_val, 4));
    return reg_val;
}

uint32_t rx_id_rb(uint8_t* regBuf)
{
    uint32_t* pVal = (uint32_t*)(regBuf+ (sizeof(uint32_t)*0x47));
    return *pVal;
}

char band()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8027, (uint8_t*)&reg_val, 4));
    if(reg_val==0x00FE011F)
        return 'L';     //57~60GHz Low Band
    else if(reg_val==0x00FE8010)
        return 'H';     //63~66GHz High Band
    else
        return 'U';     //Undefined
}

uint32_t destIdErrCounter()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x8039, (uint8_t*)&reg_val, 4));
    return reg_val;
}

uint32_t destIdErrCounter_rb(uint8_t* regBuf)
{
    uint32_t* pVal = (uint32_t*)(regBuf+ (sizeof(uint32_t)*0x39));
    return *pVal;
}

uint32_t phyRxFrameCounter()
{
    uint32_t reg_val;
    CHECK(Zing_RegRead(0x801d, (uint8_t*)&reg_val, 4));
    return reg_val;
}

uint32_t phyRxFrameCounter_rb(uint8_t* regBuf)
{
    uint32_t* pVal = (uint32_t*)(regBuf+ (sizeof(uint32_t)*0x1d));
    return *pVal;
}

char setBand(char choice)
{
    uint32_t reg_val;
    if(choice=='L')
        reg_val = 0x00FE011F;
    else if (choice=='H')
        reg_val = 0x00FE8010;
    else
        return 'U';

    CHECK(Zing_RegWrite(0x8027, (uint8_t*)&reg_val, 4));
    return choice;
}

char format(uint8_t index)
{
    if(index==1)
        return 'M';    //MJPEG
    else if (index==2)
        return 'Y';    //YUY2
    else
        return 'U';    //Undefined
}

char interference(uint32_t ppid, uint16_t deviceId, uint32_t rxid)
{
    uint32_t combinedID = (((deviceId&0xff00)>>8)<<16)|ppid;
    if(rxid != combinedID)
        return 'Y';
    else
        return 'N';
}

char interferenceByDestIdDiff(uint32_t destIdDiff)
{
    if(destIdDiff>0) {
        return 'Y';
    }else{
        return 'N';
    }
}

uint32_t reg_rb(uint8_t* regBuf, uint32_t address)
{
    uint32_t offset = address&0xfff;
    uint32_t* pVal = (uint32_t*)(regBuf+ sizeof(uint32_t)*offset);
    return *pVal;
}

void printRegisters(uint8_t* regBuf)
{
    //0x8000  REG_HW_CFG
    uint32_t HW_CFG = reg_rb(regBuf,REG_HW_CFG);
    CyU3PDebugPrint (4, "REG_HW_CFG:0x%x\r\n",HW_CFG);

    //0x8001  REG_IFS
    uint32_t IFS = reg_rb(regBuf,REG_IFS);
    CyU3PDebugPrint (4, "REG_IFS:0x%x\r\n",IFS);

    //0x8002  REG_SUPERFRAME_CFG
    uint32_t SUPERFRAME_CFG = reg_rb(regBuf,REG_SUPERFRAME_CFG);
    CyU3PDebugPrint (4, "REG_SUPERFRAME_CFG:0x%x\r\n",SUPERFRAME_CFG);

    //0x8003  REG_PPID
    uint32_t PPID = reg_rb(regBuf,REG_PPID);
    CyU3PDebugPrint (4, "REG_PPID:0x%x\r\n",PPID);

    //0x8004  REG_DEVICE_ID
    uint32_t DEVICE_ID = reg_rb(regBuf,REG_DEVICE_ID);
    CyU3PDebugPrint (4, "RED_DEVICE_ID:0x%x\r\n",DEVICE_ID);

    //0x8007  REG_PHY_CTRL
    uint32_t PHY_CTRL = reg_rb(regBuf,REG_PHY_CTRL);
    CyU3PDebugPrint (4, "REG_PHY_CTRL:0x%x\r\n",PHY_CTRL);

    //0x8008  REG_PLL_CTRL_SERDES
    uint32_t PLL_CTRL_SERDES = reg_rb(regBuf,REG_PLL_CTRL_SERDES);
    CyU3PDebugPrint (4, "REG_PLL_CTRL_SERDES:0x%x\r\n",PLL_CTRL_SERDES);

    //0x8009  REG_SERDES_TEST_PATTERN
    uint32_t SERDES_TEST_PATTERN = reg_rb(regBuf,REG_SERDES_TEST_PATTERN);
    CyU3PDebugPrint (4, "REG_SERDES_TEST_PATTERN:0x%x\r\n",SERDES_TEST_PATTERN);

    //0x800a  REG_MAC_TIMEOUT_CFG
    uint32_t MAC_TIMEOUT_CFG = reg_rb(regBuf,REG_MAC_TIMEOUT_CFG);
    CyU3PDebugPrint (4, "REG_MAC_TIMEOUT_CFG:0x%x\r\n",MAC_TIMEOUT_CFG);

    //0x800b  REG_PHY_TIMEOUT_CFG
    uint32_t PHY_TIMEOUT_CFG = reg_rb(regBuf,REG_PHY_TIMEOUT_CFG);
    CyU3PDebugPrint (4, "REG_PHY_TIMEOUT_CFG:0x%x\r\n",PHY_TIMEOUT_CFG);

    //0x800c  REG_MAC_RETX_LIMIT
    uint32_t MAC_RETX_LIMIT = reg_rb(regBuf,REG_MAC_RETX_LIMIT);
    CyU3PDebugPrint (4, "REG_MAC_RETX_LIMIT:0x%x\r\n",MAC_RETX_LIMIT);

    //0x800e  REG_SERDES_TEST_CONFIG
    uint32_t SERDES_TEST_CONFIG = reg_rb(regBuf,REG_SERDES_TEST_CONFIG);
    CyU3PDebugPrint (4, "REG_SERDES_TEST_CONFIG:0x%x\r\n",SERDES_TEST_CONFIG);

    //0x8024  REG_RF_RX_CONTROL_0
    uint32_t RF_RX_CONTROL_0 = reg_rb(regBuf,REG_RF_RX_CONTROL_0);
    CyU3PDebugPrint (4, "REG_RF_RX_CONTROL_0:0x%x\r\n",RF_RX_CONTROL_0);

    //0x8025  REG_RF_RX_CONTROL_1
    uint32_t RF_RX_CONTROL_1 = reg_rb(regBuf,REG_RF_RX_CONTROL_1);
    CyU3PDebugPrint (4, "REG_RF_RX_CONTROL_1:0x%x\r\n",RF_RX_CONTROL_1);

    //0x8026  REG_RF_TX_CONTROL_0
    uint32_t RF_TX_CONTROL_0 = reg_rb(regBuf,REG_RF_TX_CONTROL_0);
    CyU3PDebugPrint (4, "REG_RF_TX_CONTROL_0:0x%x\r\n",RF_TX_CONTROL_0);

    //0x8027  REG_RF_CONTROL_0
    uint32_t RF_CONTROL_0 = reg_rb(regBuf,REG_RF_CONTROL_0);
    CyU3PDebugPrint (4, "REG_RF_CONTROL_0:0x%x\r\n",RF_CONTROL_0);

    //0x8028  REG_RF_CONTROL_1
    uint32_t RF_CONTROL_1 = reg_rb(regBuf,REG_RF_CONTROL_1);
    CyU3PDebugPrint (4, "REG_RF_CONTROL_1:0x%x\r\n",RF_CONTROL_1);

    //0x802a  REG_PLL_CONTROL_MONITOR
    uint32_t PLL_CONTROL_MONITOR = reg_rb(regBuf,REG_PLL_CONTROL_MONITOR);
    CyU3PDebugPrint (4, "REG_PLL_CONTROL_MONITOR:0x%x\r\n",PLL_CONTROL_MONITOR);

    //0x802b  REG_SERDES_PLL_AFC
    uint32_t SERDES_PLL_AFC = reg_rb(regBuf,REG_SERDES_PLL_AFC);
    CyU3PDebugPrint (4, "REG_SERDES_PLL_AFC:0x%x\r\n",SERDES_PLL_AFC);

    //0x802c  REG_SERDES_TRIM_1
    uint32_t SERDES_TRIM_1 = reg_rb(regBuf,REG_SERDES_TRIM_1);
    CyU3PDebugPrint (4, "REG_SERDES_TRIM_1:0x%x\r\n",SERDES_TRIM_1);

    //0x802d  REG_SERDES_TRIM_2
    uint32_t SERDES_TRIM_2 = reg_rb(regBuf,REG_SERDES_TRIM_2);
    CyU3PDebugPrint (4, "REG_SERDES_TRIM_1:0x%x\r\n",SERDES_TRIM_2);

    //0x802e  REG_SERDES_TRIM_3
    uint32_t SERDES_TRIM_3 = reg_rb(regBuf,REG_SERDES_TRIM_3);
    CyU3PDebugPrint (4, "REG_SERDES_TRIM_3:0x%x\r\n",SERDES_TRIM_3);

    //0x802f  REG_SERDES_TRIM_4
    uint32_t SERDES_TRIM_4 = reg_rb(regBuf,REG_SERDES_TRIM_4);
    CyU3PDebugPrint (4, "REG_SERDES_TRIM_4:0x%x\r\n",SERDES_TRIM_4);
}
