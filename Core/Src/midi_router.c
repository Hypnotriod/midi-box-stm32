
#include "midi_router.h"
#include "usart.h"
#include "usb_device.h"
#include "usbd_midi.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t buffUsbReport[MIDI_EPIN_SIZE] = {0};
static uint8_t buffUsbReportNextIndex = 0;

static uint8_t buffUsb[MIDI_BUFFER_LENGTH] = {0};
volatile static uint8_t buffUsbNextIndex = 0;
static uint8_t buffUsbCurrIndex = 0;

static uint8_t buffUart1[3] = {0};
static uint8_t buffUartIndex1 = 0;
static uint8_t msgUart1 = 0;

void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length)
{
  while (usb_rx_buffer_length && *usb_rx_buffer != 0x00)
  {
    buffUsb[buffUsbNextIndex++] = *usb_rx_buffer++;
    buffUsb[buffUsbNextIndex++] = *usb_rx_buffer++;
    buffUsb[buffUsbNextIndex++] = *usb_rx_buffer++;
    buffUsb[buffUsbNextIndex++] = *usb_rx_buffer++;

    usb_rx_buffer_length -= 4;
  }
}

bool MIDI_HasUSBData(void)
{
  return buffUsbCurrIndex != buffUsbNextIndex;
}

void MIDI_ProcessUSBData(void)
{
  static uint8_t lastMessagesBytePerCable[MIDI_CABLES_NUMBER] = {0};
  uint8_t *pLastMessageByte;
  uint8_t cable;
  uint8_t messageByte;
  uint8_t message;
  uint8_t param1;
  uint8_t param2;
  void (*pSend)(uint8_t);

  if (buffUsbCurrIndex == buffUsbNextIndex)
    return;

  cable = (buffUsb[buffUsbCurrIndex] >> 4);
  messageByte = buffUsb[buffUsbCurrIndex + 1];

  if (cable == 0)
  {
    pLastMessageByte = &lastMessagesBytePerCable[0];
    pSend = &UART1_Send;
  }
  else if (cable == 1)
  {
    pLastMessageByte = &lastMessagesBytePerCable[1];
    pSend = &UART2_Send;
  }
  else if (cable == 2)
  {
    pLastMessageByte = &lastMessagesBytePerCable[2];
    pSend = &UART3_Send;
  }
  else
  {
    goto midi_event_packet_processed;
  }

  message = (messageByte >> 4);
  param1 = buffUsb[buffUsbCurrIndex + 2];
  param2 = buffUsb[buffUsbCurrIndex + 3];

  if ((messageByte & MIDI_MASK_REAL_TIME_MESSAGE) == MIDI_MASK_REAL_TIME_MESSAGE)
  {
    pSend(messageByte);
  }
  else if (message == MIDI_MESSAGE_CHANNEL_PRESSURE ||
           message == MIDI_MESSAGE_PROGRAM_CHANGE ||
           messageByte == MIDI_MESSAGE_TIME_CODE_QTR_FRAME ||
           messageByte == MIDI_MESSAGE_SONG_SELECT)
  {
    if (*pLastMessageByte != messageByte)
    {
      pSend(messageByte);
      *pLastMessageByte = messageByte;
    }
    pSend(param1);
  }
  else if (message == MIDI_MESSAGE_NOTE_ON ||
           message == MIDI_MESSAGE_NOTE_OFF ||
           message == MIDI_MESSAGE_KEY_PRESSURE ||
           message == MIDI_MESSAGE_CONTROL_CHANGE ||
           messageByte == MIDI_MESSAGE_SONG_POSITION ||
           message == MIDI_MESSAGE_PITCH_BAND_CHANGE)
  {
    if (*pLastMessageByte != messageByte)
    {
      pSend(messageByte);
      *pLastMessageByte = messageByte;
    }
    pSend(param1);
    pSend(param2);
  }

  midi_event_packet_processed:
  buffUsbCurrIndex += 4;
}

void MIDI_addToUSBReport(uint8_t cable, uint8_t message, uint8_t param1, uint8_t param2)
{
  buffUsbReport[buffUsbReportNextIndex++] = (cable << 4) | (message >> 4);
  buffUsbReport[buffUsbReportNextIndex++] = (message);
  buffUsbReport[buffUsbReportNextIndex++] = (param1);
  buffUsbReport[buffUsbReportNextIndex++] = (param2);

  if (buffUsbReportNextIndex == MIDI_EPIN_SIZE)
  {
    while (USBD_MIDI_GetState(&hUsbDeviceFS) != MIDI_IDLE) {};
    USBD_MIDI_SendReport(&hUsbDeviceFS, buffUsbReport, MIDI_EPIN_SIZE);
    buffUsbReportNextIndex = 0;
  }
}

void MIDI_ProcessUARTData(void)
{
  uint8_t messageByte;
  uint8_t cable;
  uint8_t *pBuff;
  uint8_t *pBuffIndex;
  uint8_t *pMessage;

  if (UART1_Available())
  {
    messageByte = UART1_Get();
    cable = MIDI_UART1_CABLE;
    pBuff = buffUart1;
    pBuffIndex = &buffUartIndex1;
    pMessage = &msgUart1;
  }
  else
  {
    goto try_to_send_usb_midi_report;
  }

  if ((messageByte & MIDI_MASK_REAL_TIME_MESSAGE) == MIDI_MASK_REAL_TIME_MESSAGE)
  {
    MIDI_addToUSBReport(cable, messageByte, 0x00, 0x00);
  }
  else
  {
    if ((messageByte & MIDI_MASK_STATUS_BYTE) == MIDI_MASK_STATUS_BYTE)
    {
      *pBuffIndex = 0;
    }

    pBuff[*pBuffIndex] = messageByte;

    if (*pBuffIndex == 0)
    {
      *pMessage = messageByte >> 4;

      if (*pMessage == MIDI_MESSAGE_NOTE_OFF ||
          *pMessage == MIDI_MESSAGE_NOTE_ON ||
          *pMessage == MIDI_MESSAGE_KEY_PRESSURE ||
          *pMessage == MIDI_MESSAGE_CONTROL_CHANGE ||
          *pMessage == MIDI_MESSAGE_PROGRAM_CHANGE ||
          *pMessage == MIDI_MESSAGE_CHANNEL_PRESSURE ||
          *pMessage == MIDI_MESSAGE_PITCH_BAND_CHANGE)
      {
        *pBuffIndex = 1;
      }
      else if (messageByte == MIDI_MESSAGE_SONG_SELECT ||
               messageByte == MIDI_MESSAGE_SONG_POSITION ||
               messageByte == MIDI_MESSAGE_TIME_CODE_QTR_FRAME)
      {
        *pMessage = messageByte;
        *pBuffIndex = 1;
      }
    }
    else if (*pBuffIndex == 1)
    {
      if (*pMessage == MIDI_MESSAGE_CHANNEL_PRESSURE ||
          *pMessage == MIDI_MESSAGE_PROGRAM_CHANGE ||
          *pMessage == MIDI_MESSAGE_TIME_CODE_QTR_FRAME ||
          *pMessage == MIDI_MESSAGE_SONG_SELECT)
      {
        MIDI_addToUSBReport(cable, pBuff[0], pBuff[1], 0x00);
        *pBuffIndex = 1;
      }
      else
      {
        *pBuffIndex = 2;
      }
    }
    else if (*pBuffIndex == 2)
    {
      if (*pMessage == MIDI_MESSAGE_NOTE_ON ||
          *pMessage == MIDI_MESSAGE_NOTE_OFF ||
          *pMessage == MIDI_MESSAGE_KEY_PRESSURE ||
          *pMessage == MIDI_MESSAGE_CONTROL_CHANGE ||
          *pMessage == MIDI_MESSAGE_SONG_POSITION ||
          *pMessage == MIDI_MESSAGE_PITCH_BAND_CHANGE)
      {
        MIDI_addToUSBReport(cable, pBuff[0], pBuff[1], pBuff[2]);
      }

      *pBuffIndex = 1;
    }
  }

  try_to_send_usb_midi_report:
  if (buffUsbReportNextIndex != 0 && USBD_MIDI_GetState(&hUsbDeviceFS) == MIDI_IDLE)
  {
    USBD_MIDI_SendReport(&hUsbDeviceFS, buffUsbReport, buffUsbReportNextIndex);
    buffUsbReportNextIndex = 0;
  }
}
