#include "cyu3types.h"
#include "cyu3os.h"
#include "cyu3system.h"
#include "cyu3error.h"
#include "cyu3externcstart.h"

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

//Endpoints
#define CY_FX_EP_PRODUCER			0x01	//EP 1 OUT, ZING Control channel
#define CY_FX_EP_CONSUMER			0x81    //EP 1 IN
#define CY_FX_EP_PRODUCER_2			0x02    //EP 2 OUT, ZING Data channel
#define CY_FX_EP_CONSUMER_2			0x82    //EP 2 IN
#define CY_FX_DATA_BURST_LENGTH		(8)     //Number of Burst for the Data. USB 3.0 only, fix 8 in ZING

void AppStart(void);
void AppStop(void);

CyBool_t IsApplnActive;

//Extern definitions for the USB Descriptors
extern const uint8_t CyFxUSB20DeviceDscr[];
extern const uint8_t CyFxUSB30DeviceDscr[];
extern const uint8_t CyFxUSBDeviceQualDscr[];
extern const uint8_t CyFxUSBFSConfigDscr[];
extern const uint8_t CyFxUSBHSConfigDscr[];
extern const uint8_t CyFxUSBBOSDscr[];
extern const uint8_t CyFxUSBSSConfigDscr[];
extern const uint8_t CyFxUSBStringLangIDDscr[];
extern const uint8_t CyFxUSBManufactureDscr[];
extern const uint8_t CyFxUSBProductDscr[];
extern const uint8_t CyFxUSBSerialNumDscr[];

#include "cyu3externcend.h"
