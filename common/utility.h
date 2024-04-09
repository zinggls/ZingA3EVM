/*
 * utility.h
 *
 *  Created on: 2022. 11. 8.
 *      Author: rossi
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "cyu3types.h"
#include "cyu3system.h"

void shrinkPrint(uint8_t* buffer, uint32_t bufferSize);
int IsSync(uint8_t byte0,uint8_t byte1);
int Loc(uint8_t* buffer, uint32_t bufferSize, uint32_t* location);
int Locs(uint8_t* buffer, uint32_t bufferSize, uint32_t location[]);
int SyncCount(uint8_t* buffer, uint32_t bufferSize);
void Set1Byte(uint8_t* buf,uint8_t val);
void Set2Byte(uint8_t* buf,uint16_t val);
void Set4Byte(uint8_t* buf,uint32_t val);
void SetBuffer(uint8_t* buf,uint16_t bmHint,uint8_t bFormatIndex,uint8_t bFrameIndex,uint32_t dwFrameInterval,uint16_t wKeyFrameRate,uint16_t wPFrameRate,
		uint16_t wCompQuality,uint16_t wCompWindowSize,uint16_t wDelay,uint32_t dwMaxVideoFrameSize,uint32_t dwMaxPayloadTransferSize);

char TransferType();
char AckMode();
char ppcMode();
uint32_t ppid();
uint16_t deviceID();
uint32_t tx_id();
uint32_t tx_id_rb(uint8_t* regBuf);
uint32_t rx_id();
uint32_t rx_id_rb(uint8_t* regBuf);
char band();
uint32_t destIdErrCounter();
uint32_t destIdErrCounter_rb(uint8_t* regBuf);
uint32_t phyRxFrameCounter();
uint32_t phyRxFrameCounter_rb(uint8_t* regBuf);
char format(uint8_t index);
char interference(uint32_t ppid, uint16_t deviceId, uint32_t rxid);
char interferenceByDestIdDiff(uint32_t destIdDiff);
char setBand(char choice);

#endif /* UTIL_H_ */
