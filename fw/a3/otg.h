#ifndef __OTG_H__
#define __OTG_H__

#include "cyu3types.h"
#include "cyu3usbotg.h"

void CyFxOtgEventCb (CyU3POtgEvent_t event,uint32_t input);
CyU3PReturnStatus_t CyFxApplnInit (void);

#endif
