#ifndef __OTG_H__
#define __OTG_H__

#include "cyu3types.h"
#include "cyu3usbotg.h"

#define CY_FX_HOST_VBUS_ENABLE_VALUE    (CyTrue)        /* GPIO value for driving VBUS. */
#define CY_FX_HOST_VBUS_DISABLE_VALUE   (!CY_FX_HOST_VBUS_ENABLE_VALUE)
#define CY_FX_USB_CHANGE_EVENT          (1 << 0)        /* Event signaling that a peripheral change is detected. */
#define CY_FX_HOST_EP0_SETUP_SIZE       (32)            /* EP0 setup packet buffer size made multiple of 32 bytes. */
#define CY_FX_HOST_EP0_WAIT_TIMEOUT     (5000)          /* EP0 request timeout in clock ticks. */

void CyFxOtgEventCb (CyU3POtgEvent_t event,uint32_t input);
CyU3PReturnStatus_t CyFxApplnInit (void);
void CyFxApplnStart ();
void CyFxApplnStop ();

#endif
