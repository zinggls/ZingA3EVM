#include "Application.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "cyu3usb.h"
#include "cyu3usbhost.h"
#include "cyu3gpio.h"
#include "cyu3utils.h"

/* Setup packet buffer for host mode. */
uint8_t glSetupPkt[CY_FX_HOST_EP0_SETUP_SIZE] __attribute__ ((aligned (32)));

/* USB host stack event callback function. */
void CyFxHostEventCb (CyU3PUsbHostEventType_t evType, uint32_t evData)
{
    /* This is connect / disconnect event. Log it so that the
     * application thread can handle it. */
    if (evType == CY_U3P_USB_HOST_EVENT_CONNECT)
    {
        CyU3PDebugPrint (4, "Host connect event received\r\n");
        glIsPeripheralPresent = CyTrue;
    }
    else
    {
        CyU3PDebugPrint (4, "Host disconnect event received\r\n");
        glIsPeripheralPresent = CyFalse;
    }

    /* Signal the thread that a peripheral change has been detected. */
    CyU3PEventSet (&applnEvent, CY_FX_USB_CHANGE_EVENT, CYU3P_EVENT_OR);
}

/* Helper function to send EP0 setup packet. */
CyU3PReturnStatus_t
CyFxSendSetupRqt (
        uint8_t type,
        uint8_t request,
        uint16_t value,
        uint16_t index,
        uint16_t length,
        uint8_t *buffer_p)
{
    CyU3PUsbHostEpStatus_t epStatus;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    glSetupPkt[0] = type;
    glSetupPkt[1] = request;
    glSetupPkt[2] = CY_U3P_GET_LSB(value);
    glSetupPkt[3] = CY_U3P_GET_MSB(value);
    glSetupPkt[4] = CY_U3P_GET_LSB(index);
    glSetupPkt[5] = CY_U3P_GET_MSB(index);
    glSetupPkt[6] = CY_U3P_GET_LSB(length);
    glSetupPkt[7] = CY_U3P_GET_MSB(length);

    status = CyU3PUsbHostSendSetupRqt (glSetupPkt, buffer_p);
    if (status != CY_U3P_SUCCESS)
    {
        return status;
    }
    status = CyU3PUsbHostEpWaitForCompletion (0, &epStatus,
            CY_FX_HOST_EP0_WAIT_TIMEOUT);

    return status;
}

/* This function initializes the mouse driver application. */
void CyFxApplnStart ()
{
	CyU3PDebugPrint (2, "CyFxApplnStart()\r\n");
}

/* This function disables the mouse driver application. */
void CyFxApplnStop ()
{
	CyU3PDebugPrint (2, "CyFxApplnStop()\r\n");
}

/* This function initializes the USB host stack. */
void CyFxUsbHostStart ()
{
    CyU3PUsbHostConfig_t hostCfg;
    CyU3PReturnStatus_t status;

    hostCfg.ep0LowLevelControl = CyFalse;
    hostCfg.eventCb = CyFxHostEventCb;
    hostCfg.xferCb = NULL;
    status = CyU3PUsbHostStart (&hostCfg);
    if (status != CY_U3P_SUCCESS)
    {
        return;
    }
}

/* This function disables the USB host stack. */
void CyFxUsbHostStop ()
{
    /* Stop host mode application if running. */
    if (glIsApplnActive)
    {
        CyFxApplnStop ();
    }

    /* Stop the host module. */
    CyU3PUsbHostStop ();
}

/* This function enables / disables driving VBUS. */
void CyFxUsbVBusControl (CyBool_t isEnable)
{
    if (isEnable)
    {
        /* Drive VBUS only if no-one else is driving. */
        if (!CyU3POtgIsVBusValid ())
        {
            CyU3PGpioSimpleSetValue (21, CY_FX_HOST_VBUS_ENABLE_VALUE);
        }
    }
    else
    {
        CyU3PGpioSimpleSetValue (21, CY_FX_HOST_VBUS_DISABLE_VALUE);
    }

    CyU3PDebugPrint (2, "VBUS enable:%d\r\n", isEnable);
}

/* OTG event handler. */
void CyFxOtgEventCb (CyU3POtgEvent_t event,uint32_t input)
{
	CyU3PDebugPrint (4, "OTG Event: %d, Input: %d.\r\n", event, input);
	switch (event)
	{
		case CY_U3P_OTG_PERIPHERAL_CHANGE:
			if (input == CY_U3P_OTG_TYPE_A_CABLE)
			{
				CyFxUsbHostStart ();
				/* Enable the VBUS supply. */
				CyFxUsbVBusControl (CyTrue);
			}
			else
			{
				/* Make sure that the VBUS supply is disabled. */
				CyFxUsbVBusControl (CyFalse);

				/* Stop the previously started host stack. */
				if ((!CyU3POtgIsHostMode ()) && (CyU3PUsbHostIsStarted ()))
				{
					CyFxUsbHostStop ();
				}
			}
			break;

		case CY_U3P_OTG_VBUS_VALID_CHANGE:
			if (input)
			{
				/* Start the host mode stack. */
				if (!CyU3PUsbHostIsStarted ())
				{
					CyFxUsbHostStart ();
				}
			}
			else
			{
				/* If the OTG mode has changed, stop the previous stack. */
				if ((!CyU3POtgIsHostMode ()) && (CyU3PUsbHostIsStarted ()))
				{
					/* Stop the previously started host stack. */
					CyFxUsbHostStop ();
				}
			}
			break;

		default:
			/* do nothing */
			break;
	}
}

CyU3PReturnStatus_t CyFxApplnInit (void)
{
	CyU3POtgConfig_t otgCfg;
	CyU3PReturnStatus_t status;

	/* Wait until VBUS is stabilized. */
	CyU3PThreadSleep (100);

	glIsApplnActive = CyFalse;			/* Whether the application is active or not. */
	glIsPeripheralPresent = CyFalse;	/* Whether a remote peripheral is present or not. */

	/* Initialize the OTG module. */
	otgCfg.otgMode = CY_U3P_OTG_MODE_OTG;
	otgCfg.chargerMode = CY_U3P_OTG_CHARGER_DETECT_ACA_MODE;
	otgCfg.cb = CyFxOtgEventCb;
	status = CyU3POtgStart (&otgCfg);
	if (status != CY_U3P_SUCCESS) {
		CyU3PDebugPrint (4, "CyU3POtgStart failed with status: %d\r\n",status);
		return status;
	}

	/* Since VBATT or VBUS is required for OTG operation enable it. */
	return CyU3PUsbVBattEnable (CyTrue);
}
