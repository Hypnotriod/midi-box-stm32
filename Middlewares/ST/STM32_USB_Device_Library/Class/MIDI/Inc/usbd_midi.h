/**
  ******************************************************************************
  * @file    usbd_midi_core.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    18-February-2014
  * @brief   header file for the usbd_MIDI_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  *
  *                      Modified by Ilya Pikin 09.09.14
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_MIDI_CORE_H_
#define __USB_MIDI_CORE_H_

#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MIDI_Exported_Defines
  * @{
  */ 
#define MIDI_IN_PORTS_NUM              0x01
#define MIDI_OUT_PORTS_NUM             0x03
  
#define MIDI_EPIN_ADDR                 0x81
#define MIDI_EPIN_SIZE                 0x40

#define MIDI_EPOUT_ADDR                0x01
#define MIDI_EPOUT_SIZE                0x40

#define USB_MIDI_CLASS_DESC_SHIFT      18
#define USB_MIDI_DESC_SIZE             7
#define USB_MIDI_REPORT_DESC_SIZE      (MIDI_IN_PORTS_NUM * 16 + MIDI_OUT_PORTS_NUM * 16 + 33)
#define USB_MIDI_CONFIG_DESC_SIZE      (USB_MIDI_REPORT_DESC_SIZE + USB_MIDI_CLASS_DESC_SHIFT)

#define MIDI_DESCRIPTOR_TYPE           0x21
  
#define MIDI_REQ_SET_PROTOCOL          0x0B
#define MIDI_REQ_GET_PROTOCOL          0x03

#define MIDI_REQ_SET_IDLE              0x0A
#define MIDI_REQ_GET_IDLE              0x02

#define MIDI_REQ_SET_REPORT            0x09
#define MIDI_REQ_GET_REPORT            0x01

#define MIDI_JACK_1    0x01
#define MIDI_JACK_2    0x02
#define MIDI_JACK_3    0x03
#define MIDI_JACK_4    0x04
#define MIDI_JACK_5    0x05
#define MIDI_JACK_6    0x06
#define MIDI_JACK_7    0x07
#define MIDI_JACK_8    0x08
#define MIDI_JACK_9    0x09
#define MIDI_JACK_10    0x0a
#define MIDI_JACK_11    0x0b
#define MIDI_JACK_12    0x0c
#define MIDI_JACK_13    0x0d
#define MIDI_JACK_14    0x0e
#define MIDI_JACK_15    0x0f
#define MIDI_JACK_16    0x10
#define MIDI_JACK_17    0x11
#define MIDI_JACK_18    0x12
#define MIDI_JACK_19    0x13
#define MIDI_JACK_20    0x14
#define MIDI_JACK_21    0x15
#define MIDI_JACK_22    0x16
#define MIDI_JACK_23    0x17
#define MIDI_JACK_24    0x18
#define MIDI_JACK_25    0x19
#define MIDI_JACK_26    0x1a
#define MIDI_JACK_27    0x1b
#define MIDI_JACK_28    0x1c
#define MIDI_JACK_29    0x1d
#define MIDI_JACK_30    0x1e
#define MIDI_JACK_31    0x1f
#define MIDI_JACK_32    0x20
#define MIDI_JACK_33    0x21
#define MIDI_JACK_34    0x22
#define MIDI_JACK_35    0x23
#define MIDI_JACK_36    0x24
#define MIDI_JACK_37    0x25
#define MIDI_JACK_38    0x26
#define MIDI_JACK_39    0x27
#define MIDI_JACK_40    0x28
#define MIDI_JACK_41    0x29
#define MIDI_JACK_42    0x2a
#define MIDI_JACK_43    0x2b
#define MIDI_JACK_44    0x2c
#define MIDI_JACK_45    0x2d
#define MIDI_JACK_46    0x2e
#define MIDI_JACK_47    0x2f
#define MIDI_JACK_48    0x30
#define MIDI_JACK_49    0x31
#define MIDI_JACK_50    0x32
#define MIDI_JACK_51    0x33
#define MIDI_JACK_52    0x34
#define MIDI_JACK_53    0x35
#define MIDI_JACK_54    0x36
#define MIDI_JACK_55    0x37
#define MIDI_JACK_56    0x38
#define MIDI_JACK_57    0x39
#define MIDI_JACK_58    0x3a
#define MIDI_JACK_59    0x3b
#define MIDI_JACK_60    0x3c
#define MIDI_JACK_61    0x3d
#define MIDI_JACK_62    0x3e
#define MIDI_JACK_63    0x3f
#define MIDI_JACK_64    0x40

/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef enum
{
  MIDI_IDLE = 0,
  MIDI_BUSY,
}
MIDI_StateTypeDef; 


typedef struct
{
  uint32_t             Protocol;   
  uint32_t             IdleState;  
  uint32_t             AltSetting;
  MIDI_StateTypeDef     state;  
}
USBD_MIDI_HandleTypeDef; 
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_MIDI;
extern void USBD_MIDI_DataInHandler(uint8_t * usb_rx_buffer, uint8_t usb_rx_buffer_length);

/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t USBD_MIDI_SendReport (USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
uint8_t *USBD_MIDI_DeviceQualifierDescriptor (uint16_t *length);
uint8_t USBD_MIDI_GetState(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MIDI_GetDeviceState(USBD_HandleTypeDef  *pdev);

/**
  * @}
  */ 

#endif  // __USB_MIDI_CORE_H_
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
