N.B.: To all those who are citizens of the countries listed: russia, or provide any support or justification for such, my friendly reminder that you are not welcome.

# midi-box-stm32

`Midi Hub Device` implementation (with 1 input and 3 output ports) example based on the STM32F103 dev board.  
  
Contains `MIDI Device Class` Middleware implementation for `STM32 HAL USB` drivers, compatible with the `STM32CubeMX`/`STM32CubeIDE` code generator.  
The MIDI class V1.0 follows the "Universal Serial Bus Device Class Definition for MIDI Devices. Release 1.0 Nov 1, 1999"  
  
The user may specify the number of physical/virtual input type `MIDI_IN_PORTS_NUM` ports and output type `MIDI_OUT_PORTS_NUM` ports of the device.  
The `port` is associated with the `cable` number in the MIDI event packet.  
Up to 8 ports of each type are supported, but only 12 ports in total.  

## Configuring user project with STM32CubeMX code generator to use MIDI Device Class Middleware

In STM32CubeMX / STM32CubeIDE:
* At `USB` -> enable `Device FS`
* At `USB_DEVICE` -> choose `Human Interface Device Class (HID)`
* (Optionally) At `USB_DEVICE` -> `Device Descriptor` -> update the device descriptor information with your device info
* Generate code
  
To use the `MIDI Device Class` middleware, the project requires a few modifications in the generated code:
* Copy the `usbd_midi.c` and `usbd_midi.h` to the `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` `Src` and `Inc` folders respectively.
* In your IDE add those folders to the C/C++ compiler include path, and add the files to the corresponding group.
* Modify the `USB_DEVICE/App/usb_device.c`:
```C
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)  // replace this line
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK) // with this line
```
* Modify `USB_DEVICE/Target/usbd_conf.c`:
```C
#include "usbd_hid.h"  // replace this line
#include "usbd_midi.h" // with this line
...
/* USER CODE BEGIN EndPoint_Configuration_HID */ // this section may be absent if your device has no PMA, so no action required
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x01 , PCD_SNG_BUF, 0xC0); // add this line
HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x81 , PCD_SNG_BUF, 0x100); // leave this line as is
/* USER CODE END EndPoint_Configuration_HID */
...
static uint32_t mem[(sizeof(USBD_HID_HandleTypeDef)/4)+1]; // replace this line
static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef)/4)+1]; // with this line
```
* Modify `Core/Inc/main.h`:
```C
/* USER CODE BEGIN EM */
#define MIDI_IN_PORTS_NUM   0x01 // Specify input ports number of your device
#define MIDI_OUT_PORTS_NUM  0x01 // Specify output ports number of your device
```
In some versions of the ST libraries you may encounter a `MIDI_IN_PORTS_NUM macro is undeclared` error. In this case you need to add `MIDI_IN_PORTS_NUM` and `MIDI_OUT_PORTS_NUM` definitions to the `USB_DEVICE/Target/usbd_conf.h` file.

## Midi event packet structure:
```
|  4 bits  |  4 bits  |  4 bits  |  4 bits  |  8 bits  |  8 bits  |
|          |          |        MIDI_0       |  MIDI_1  |  MIDI_2  |
|   Cable  |   Code   |  Message |  Channel |  Byte 1  |  Byte 2  |
```
Please refer to the `USB-MIDI Event Packets` chapter in the [midi10.pdf](https://github.com/Hypnotriod/midi-box-stm32/blob/master/doc/midi10.pdf) documentation for more information.  
## Send the midi event packets to the host
* The size of the `packetsBuffer` should not exceed the `MIDI_EPIN_SIZE` (64 bytes), and should consist of a maximum of 16 event packets.
* Ensure that the MIDI driver status is IDLE before each transfer initiation by using:
```C
USBD_MIDI_GetState(&hUsbDeviceFS) == MIDI_IDLE
```
* Send midi event packets with:
```C
USBD_MIDI_SendPackets(&hUsbDeviceFS, packetsBuffer, eventPacketsNumber * 4);
```
Example of sending a single event packet to the host:
```C
#include "usbd_midi.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
...
uint8_t packetsBuffer[4] = {
  // cable - represents the physical/virtual input port number (0 - 15) of the device
  // code - in general cases is equal to the midi message
  (cable << 4) | code,
  (message << 4) | channel,
  messageByte1,
  messageByte2,
};
...
while (USBD_MIDI_GetState(&hUsbDeviceFS) != MIDI_IDLE) {};
USBD_MIDI_SendPackets(&hUsbDeviceFS, packetsBuffer, 4);
```
## Receive the midi event packets from the host
Override the weak `USBD_MIDI_OnPacketsReceived` callback function *(which will be called during the USB interrupt routine)* with code similar to the following:
```C
void USBD_MIDI_OnPacketsReceived(uint8_t *data, uint8_t len)
{
  while (len)
  {
    // cable - represents the physical/virtual output port number (0 - 15) of the device
    cable = data[0] >> 4;
    code = data[0] & 0x0F;
    message = data[1] >> 4;
    channel = data[1] & 0x0F;
    messageByte1 = data[2];
    messageByte2 = data[3];
    data += 4;
    len -= 4;
  }
}
```
## Interrupt callbacks and FreeRTOS
* Do not call the `USBD_MIDI_SendPackets` function from the `USBD_MIDI_OnPacketsReceived` or `USBD_MIDI_OnPacketsSent` callback functions, as they are called during the USB interrupt routine.
* In the case of the `FreeRTOS` - consider to use the `xQueueSendToBackFromISR` with the `xQueueReceive` functions, to pass the received MIDI data from the `USBD_MIDI_OnPacketsReceived` interrupt callback to the dedicated `task`. [documentation](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch07.md)  
Also use the `xTaskNotifyFromISR` with the `xTaskNotifyWait` functions, to notify the dedicated `task` from the `USBD_MIDI_OnPacketsSent` interrupt callback that the MIDI packets were sent to the host. [documentation](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch10.md)
