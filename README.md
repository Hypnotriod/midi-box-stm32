# midi-box-stm32

`Midi Hub Device` implementation (with 1 input and 3 output ports) example based on STM32F103 dev board.  
  
Contains `MIDI Device Class` Middleware implementation for `STM32 HAL USB` drivers, compatible with `STM32CubeMX`/`STM32CubeIDE` code generator.  
MIDI class V1.0 following the "Universal Serial BusDevice Class Definition for MIDI Devices. Release 1.0 Nov 1, 1999"  
  
In file `usbd_midi.h` user can specify number of physical/virtual input type `MIDI_IN_PORTS_NUM` ports and output type `MIDI_OUT_PORTS_NUM` ports.  
Right now up to 8 ports of each type supported, but only 12 ports in total.  

## Configuring user project with STM32CubeMX code generator to use MIDI Device Class Middleware

In STM32CubeMX / STM32CubeIDE:
* At `USB` -> enable `Device FS`
* At `USB_DEVICE` -> choose `Human Interface Device Class (HID)`
* Generate code
  
To use `MIDI Device Class` middleware, project requires few modifications in generated code:
* Copy `usbd_midi.c` and `usbd_midi.h` to `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` `Src` and `Inc` folders respectively.
* In your IDE add those folders to C/C++ compiler include path, and files to corresponding group.
* Modify `USB_DEVICE/App/usb_device.c`:
```
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)  // replace this line
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK) // with this line
```
* Modify `USB_DEVICE/App/usbd_conf.c`:
```
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
/* USER CODE BEGIN EndPoint_Configuration_HID */ // You may replace HID with MIDI for more convenience, but code generator will not keep it on next code generate
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x01 , PCD_SNG_BUF, 0xC0); // add this line
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x81 , PCD_SNG_BUF, 0x100); // leave this line as is
/* USER CODE END EndPoint_Configuration_HID */
```
## Send midi messages packet to host device:
* Enshure that MIDI driver status is IDLE by:
```
USBD_MIDI_GetState(&hUsbDeviceFS) == MIDI_IDLE
```
* Send midi messages packet with:
```
USBD_MIDI_SendReport(&hUsbDeviceFS, messagesBuffer, MIDI_EPIN_SIZE);
```
## Receive midi messages packet from host device:
* Implement this weak function with something like this:
```
void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length)
{
  while (usb_rx_buffer_length && *usb_rx_buffer != 0x00)
  {
    wire = usb_rx_buffer[0] >> 4;
    message = usb_rx_buffer[1] >> 4;
    channel = usb_rx_buffer[1] & 0x0F;
    messageByte1 = usb_rx_buffer[2];
    messageByte2 = usb_rx_buffer[3];
    usb_rx_buffer += 4;
    usb_rx_buffer_length -= 4;
  }
}
```
