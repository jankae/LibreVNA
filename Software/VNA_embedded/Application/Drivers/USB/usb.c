#include "usb.h"
#include "usbd_desc.h"
#include "usbd_core.h"

USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

#define EP_DATA_IN_ADDRESS		0x81
#define EP_DATA_OUT_ADDRESS		0x01
#define EP_LOG_IN_ADDRESS		0x82

static uint8_t  USBD_Class_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_Class_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_Class_Setup(USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req);
static uint8_t  USBD_Class_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_Class_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  *USBD_Class_GetFSCfgDesc (uint16_t *length);
static uint8_t  *USBD_Class_GetDeviceQualifierDescriptor (uint16_t *length);

static usbd_recv_callback_t cb;
static uint8_t usb_receive_buffer[1024];
static uint8_t usb_transmit_fifo[8192];
static uint16_t usb_transmit_read_index = 0;
static uint16_t usb_transmit_fifo_level = 0;
static bool data_transmission_active = false;
static bool log_transmission_active = true;

USBD_ClassTypeDef  USBD_ClassDriver =
{
  USBD_Class_Init,
  USBD_Class_DeInit,
  USBD_Class_Setup,
  NULL,
  NULL,
  USBD_Class_DataIn,
  USBD_Class_DataOut,
  NULL,
  NULL,
  NULL,
  NULL,
  USBD_Class_GetFSCfgDesc,
  NULL,
  USBD_Class_GetDeviceQualifierDescriptor,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

#define USB_CONFIG_DESC_SIZ		39

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CfgFSDesc[USB_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,                          /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,   /* bDescriptorType: Configuration */
  USB_CONFIG_DESC_SIZ,  /* wTotalLength: Bytes returned */
  0x00,
  0x01,                          /*bNumInterfaces: 1 interface*/
  0x01,                          /*bConfigurationValue: Configuration value*/
  0x00,                          /*iConfiguration */
  0xC0,                          /*bmAttributes: bus powered and Supports Remote Wakeup */
  0x32,                          /*MaxPower 100 mA: this current is used for detecting Vbus*/
  /* Interface */
  0x09,                     /* bLength */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x00,                     /* bInterfaceNumber */
  0x00,                     /* bAlternateSetting */
  0x03,                     /* bNumEndpoints */
  0xFF,                     /* bInterfaceClass */
  0x00,                     /* bInterfaceSubClass */
  0x00,                     /* bInterfaceProtocol */
  0x00,                     /* iInterface */
  /* Endpoint Data OUT */
  0x07,                            /* bLength */
  USB_DESC_TYPE_ENDPOINT,          /* bDescriptorType */
  EP_DATA_OUT_ADDRESS,			           /* bEndpointAddress */
  0x02,                            /* bmAttributes */
  LOBYTE(USB_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize */
  HIBYTE(USB_FS_MAX_PACKET_SIZE),
  0x00,                            /* bInterval */
  /* Endpoint Data IN */
  0x07,                             /* bLength */
  USB_DESC_TYPE_ENDPOINT,           /* bDescriptorType */
  EP_DATA_IN_ADDRESS,               /* bEndpointAddress */
  0x02,                             /* bmAttributes */
  LOBYTE(USB_FS_MAX_PACKET_SIZE),   /* wMaxPacketSize */
  HIBYTE(USB_FS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval */
  /* Endpoint logging IN */
  0x07,                             /* bLength */
  USB_DESC_TYPE_ENDPOINT,           /* bDescriptorType */
  EP_LOG_IN_ADDRESS,               	/* bEndpointAddress */
  0x02,                             /* bmAttributes */
  LOBYTE(USB_FS_MAX_PACKET_SIZE),   /* wMaxPacketSize */
  HIBYTE(USB_FS_MAX_PACKET_SIZE),
  0x00                              /* bInterval */
};

// See https://github.com/pbatard/libwdi/wiki/WCID-Devices for descriptor data
// This requests to load the WinUSB driver for this device
__ALIGN_BEGIN const char USBD_MicrosoftCompatibleID[40] __ALIGN_END =
{
	0x28, 0x00, 0x00, 0x00,
	0x00, 0x01,
	0x04, 0x00,
	0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	0x01,
	'W','I','N','U','S','B','\0','\0',
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t  USBD_Class_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	// Open endpoints and start reception
	USBD_LL_OpenEP(pdev, EP_DATA_IN_ADDRESS, USBD_EP_TYPE_BULK, USB_FS_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, EP_DATA_OUT_ADDRESS, USBD_EP_TYPE_BULK, USB_FS_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, EP_LOG_IN_ADDRESS, USBD_EP_TYPE_BULK, USB_FS_MAX_PACKET_SIZE);
	USBD_LL_PrepareReceive(pdev, EP_DATA_OUT_ADDRESS, usb_receive_buffer,	USB_FS_MAX_PACKET_SIZE);
	return USBD_OK;
}
static uint8_t  USBD_Class_DeInit(USBD_HandleTypeDef *pdev,
                                     uint8_t cfgidx)
{
  USBD_LL_CloseEP(pdev, EP_DATA_IN_ADDRESS);
  USBD_LL_CloseEP(pdev, EP_DATA_OUT_ADDRESS);
  USBD_LL_CloseEP(pdev, EP_LOG_IN_ADDRESS);
  return USBD_OK;
}
static uint8_t USBD_Class_Setup(USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req) {
	if(req->wIndex == 0x0004 && req->bRequest == USB_WCID_VENDOR_CODE && req->bmRequest == 0xC0) {
		// This is a request for the Microsoft Compatible IF Feature Descriptor
		USBD_CtlSendData (pdev, (uint8_t *)(void *)USBD_MicrosoftCompatibleID, req->wLength);
	}
	return USBD_OK;
}

static bool trigger_next_fifo_transmission() {
	data_transmission_active = true;
	uint16_t continous_length = sizeof(usb_transmit_fifo) - usb_transmit_read_index;
	if(continous_length > usb_transmit_fifo_level) {
		continous_length = usb_transmit_fifo_level;
	}
	if(continous_length > sizeof(usb_transmit_fifo)/ 2) {
		continous_length = sizeof(usb_transmit_fifo) / 2;
	}
	hUsbDeviceFS.ep_in[EP_DATA_IN_ADDRESS & 0x7F].total_length = continous_length;
	return USBD_LL_Transmit(&hUsbDeviceFS, EP_DATA_IN_ADDRESS, &usb_transmit_fifo[usb_transmit_read_index], continous_length) == USBD_OK;
}

static uint8_t USBD_Class_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	// A bulk transfer is complete when the endpoint does on of the following:
	// - Has transferred exactly the amount of data expected
	// - Transfers a packet with a payload size less than wMaxPacketSize or transfers a zero-length packet
	if(epnum == (EP_DATA_IN_ADDRESS & 0x7F)) {
		// transmission of fifo data, mark as empty
		__disable_irq();
		usb_transmit_fifo_level -= pdev->ep_in[epnum].total_length;
		usb_transmit_read_index += pdev->ep_in[epnum].total_length;
		usb_transmit_read_index %= sizeof(usb_transmit_fifo);
		__enable_irq();
	}
	if (pdev->ep_in[epnum].total_length
			&& !(pdev->ep_in[epnum].total_length % USB_FS_MAX_PACKET_SIZE)) {
		pdev->ep_in[epnum].total_length = 0;
		USBD_LL_Transmit(pdev, epnum, NULL, 0);
	} else {
		if(epnum == (EP_DATA_IN_ADDRESS & 0x7F)) {
			if(usb_transmit_fifo_level > 0) {
				trigger_next_fifo_transmission();
			} else {
				data_transmission_active = false;
			}
		} else {
			log_transmission_active = false;
		}
	}
	return USBD_OK;
}
static uint8_t  USBD_Class_DataOut(USBD_HandleTypeDef *pdev,
                                      uint8_t epnum)
{
  if(cb) {
	  cb(usb_receive_buffer, USBD_LL_GetRxDataSize (pdev, epnum));
  }
  USBD_LL_PrepareReceive(pdev, EP_DATA_OUT_ADDRESS, usb_receive_buffer, USB_FS_MAX_PACKET_SIZE);
  return USBD_OK;
}
static uint8_t  *USBD_Class_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CfgFSDesc);
  return USBD_CfgFSDesc;
}
static uint8_t  *USBD_Class_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = sizeof(USBD_DeviceQualifierDesc);
  return USBD_DeviceQualifierDesc;
}

void usb_init(usbd_recv_callback_t receive_callback) {
	cb = receive_callback;
	USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_ClassDriver);
	USBD_Start(&hUsbDeviceFS);
    HAL_NVIC_SetPriority(USB_HP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_IRQn);
    HAL_NVIC_SetPriority(USB_LP_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USB_LP_IRQn);
}
bool usb_transmit(const uint8_t *data, uint16_t length) {
	// attempt to add data to fifo
	if(length > usb_available_buffer()) {
		// data won't fit, abort
		return false;
	}
	// grab pointer to write position
	__disable_irq();
	uint16_t write_index = usb_transmit_read_index + usb_transmit_fifo_level;
	__enable_irq();
	write_index %= sizeof(usb_transmit_fifo);
	// copy the data to the fifo
	uint16_t continous_length = sizeof(usb_transmit_fifo) - write_index;
	if(continous_length > length) {
		// can copy all data at once
		memcpy(&usb_transmit_fifo[write_index], data, length);
	} else {
		// needs to copy two data segments
		memcpy(&usb_transmit_fifo[write_index], data, continous_length);
		memcpy(&usb_transmit_fifo[0], data + continous_length, length - continous_length);
	}
	// increment fifo level
	__disable_irq();
	usb_transmit_fifo_level += length;
	__enable_irq();

	static bool first = true;
	if(first) {
		log_transmission_active = false;
		first = false;
	}
	if(!data_transmission_active) {
		return trigger_next_fifo_transmission();
	} else {
		// still transmitting, no need to trigger
		return true;
	}
}

void usb_log(const char *log, uint16_t length) {
	if(!log_transmission_active) {
		static uint8_t buffer[256];
		memcpy(buffer, log, length);
		log_transmission_active = true;
		hUsbDeviceFS.ep_in[EP_LOG_IN_ADDRESS & 0x7F].total_length = length;
		USBD_LL_Transmit(&hUsbDeviceFS, EP_LOG_IN_ADDRESS, buffer, length);
	} else {
		// still busy, unable to send log
	}
}

void USB_HP_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
void USB_LP_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

uint16_t usb_available_buffer() {
	return sizeof(usb_transmit_fifo) - usb_transmit_fifo_level;
}
