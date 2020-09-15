#ifndef __ZING_INTERNAL_H__
#define __ZING_INTERNAL_H__

CyU3PReturnStatus_t Zing_PLLConfig(void);
void Zing_AllocBuffer(void);
void Zing_Header(
		uint8_t *pt,            /* pt : buffer pointer */
        uint16_t payload_size,  /* payload_size : memory size in bytes */
        uint16_t addr,          /* addr : zing internal address , address unit = 32bit */
        uint16_t type           /* type : Read/write */);

void Zing_DataReadFlush(void);
void Zing_SetGPIFBusWidth(uint8_t width);

void Zing_Header2(
        uint8_t *pt,            /* pt : buffer pointer */
        uint16_t dir,
        uint16_t interrupt,
        uint16_t target,
        uint16_t type,
        uint16_t req_resp,
        uint16_t fr_type,
        uint16_t intr_flags,
        uint16_t addr,
        uint16_t payload_size);

#endif
