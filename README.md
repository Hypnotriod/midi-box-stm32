# midi-box-stm32

`Midi Hub Device` implementation example based on STM32F103 dev board.  
  
Contains `MIDI Device Class` Middleware implementation for `STM32 HAL USB` drivers, compatible with `STM32CubeMX` / `STM32CubeIDE` code generator.  
MIDI class V1.0 following the "Universal Serial BusDevice Class Definition for MIDI Devices. Release 1.0 Nov 1, 1999"  

## Configuring project using STM32CubeMX code generator

In STM32CubeMX / STM32CubeMX:
* At `USB` -> enable `Device FS`
* At `USB_DEVICE` -> choose `Human Interface Device Class (HID)`
  
To use `MIDI Device Class` middleware, project requires few modifications in generated code:
* Copy `usbd_midi.c` and `usbd_midi.h` to `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` `Src` and `Inc` folders respectively
* In your IDE add those folders to C/C++ compiler include path, and files to corresponding group
* Modify `USB_DEVICE/App/usb_device.c`:
```
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)  // replace this line
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK) // with this line
```
* Modify `USB_DEVICE/App/usb_device.c`:
```
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
/* USER CODE BEGIN EndPoint_Configuration_HID */ // You may replace HID with MIDI for more convenience, but code generator will not keep it on next code generate
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x01 , PCD_SNG_BUF, 0xC0); // add this line
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x81 , PCD_SNG_BUF, 0x100); // leave this line as is
/* USER CODE END EndPoint_Configuration_HID */
```
