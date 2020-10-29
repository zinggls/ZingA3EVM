#include "otg.h"
#include "cyu3error.h"
#include "cyu3system.h"
#include "cyu3usb.h"

/* OTG event handler. */
void CyFxOtgEventCb (CyU3POtgEvent_t event,uint32_t input)
{
	CyU3PDebugPrint (4, "OTG Event: %d, Input: %d.\r\n", event, input);
	switch (event)
	{
		case CY_U3P_OTG_PERIPHERAL_CHANGE:
			if (input == CY_U3P_OTG_TYPE_A_CABLE)
			{
				CyU3PDebugPrint (4, "OTG Event = CY_U3P_OTG_PERIPHERAL_CHANGE, Input = CY_U3P_OTG_TYPE_A_CABLE\r\n");
			}
			else
			{
				CyU3PDebugPrint (4, "OTG Event = CY_U3P_OTG_PERIPHERAL_CHANGE, Input != CY_U3P_OTG_TYPE_A_CABLE\r\n");
			}
			break;

		case CY_U3P_OTG_VBUS_VALID_CHANGE:
			if (input)
			{
				CyU3PDebugPrint (4, "OTG Event = CY_U3P_OTG_VBUS_VALID_CHANGE, Input: true\r\n");
			}
			else
			{
				CyU3PDebugPrint (4, "OTG Event = CY_U3P_OTG_VBUS_VALID_CHANGE, Input: !true\r\n");
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

	/* Initialize the OTG module. */
	otgCfg.otgMode = CY_U3P_OTG_MODE_OTG;
	otgCfg.chargerMode = CY_U3P_OTG_CHARGER_DETECT_ACA_MODE;
	otgCfg.cb = CyFxOtgEventCb;
	status = CyU3POtgStart (&otgCfg);
	if (status != CY_U3P_SUCCESS) return status;

	/* Since VBATT or VBUS is required for OTG operation enable it. */
	return CyU3PUsbVBattEnable (CyTrue);
}
