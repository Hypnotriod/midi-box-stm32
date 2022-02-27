
#include "midi_router.h"
#include "usart.h"
#include "usb_device.h"
#include "usbd_midi.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t buffUsbReport[MIDI_EPIN_SIZE] = {0};
volatile static uint8_t buffUsbReportNextIndex = 0;

volatile static uint8_t buffUsb[MIDI_BUFFER_LENGTH] = {0};
volatile static uint8_t buffUsbNextIndex = 0;
volatile static uint8_t buffUsbCurrIndex = 0;

static uint8_t buffUart1[3] = {0};
static uint8_t buffUartIndex1 = 0;
static uint8_t msgUart1 = 0;

void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length)
{
	while (usb_rx_buffer_length && *usb_rx_buffer != 0x00)
	{
		buffUsb[buffUsbNextIndex++] = *usb_rx_buffer;
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
	static uint8_t lastMessagesBytePerWire[MIDI_WIRES_NUMBER] = {0};
	uint8_t *pLastMessageByte;
	uint8_t wire;
	uint8_t messageByte;
	uint8_t message;
	uint8_t param1;
	uint8_t param2;
	void (*pSend)(uint8_t);

	if (buffUsbCurrIndex == buffUsbNextIndex)
		return;

	wire = (buffUsb[buffUsbCurrIndex] >> 4);
	messageByte = buffUsb[buffUsbCurrIndex + 1];

	if (wire == 0)
	{
		pLastMessageByte = &lastMessagesBytePerWire[0];
		pSend = &UART1_Send;
	}
	else if (wire == 1)
	{
		pLastMessageByte = &lastMessagesBytePerWire[1];
		pSend = &UART2_Send;
	}
	else if (wire == 2)
	{
		pLastMessageByte = &lastMessagesBytePerWire[2];
		pSend = &UART3_Send;
	}
	else
	{
		pSend = NULL;
	}

	if (pSend != NULL)
	{
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
				pSend(messageByte),
						*pLastMessageByte = messageByte;
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
				pSend(messageByte),
						*pLastMessageByte = messageByte;
			pSend(param1);
			pSend(param2);
		}
	}

	buffUsbCurrIndex += 4;
}

void MIDI_addUSBReport(uint8_t wire, uint8_t message, uint8_t param1, uint8_t param2)
{
	buffUsbReport[buffUsbReportNextIndex++] = (wire << 4) | (message >> 4);
	buffUsbReport[buffUsbReportNextIndex++] = (message);
	buffUsbReport[buffUsbReportNextIndex++] = (param1);
	buffUsbReport[buffUsbReportNextIndex++] = (param2);

	if (buffUsbReportNextIndex == MIDI_EPIN_SIZE)
	{
		while (USBD_MIDI_GetState(&hUsbDeviceFS) != MIDI_IDLE)
		{
		};
		USBD_MIDI_SendReport(&hUsbDeviceFS, buffUsbReport, MIDI_EPIN_SIZE);
		buffUsbReportNextIndex = 0;
	}
}

void MIDI_ProcessUARTData(void)
{
	uint8_t messageByte;
	uint8_t *buffUart;
	uint8_t *buffUartIndex;
	uint8_t *msgUart;

	if (UART1_Available())
	{
		messageByte = UART1_Get();
		buffUart = buffUart1;
		buffUartIndex = &buffUartIndex1;
		msgUart = &msgUart1;
	}
	else
	{
		return;
	}

	if ((messageByte & MIDI_MASK_REAL_TIME_MESSAGE) == MIDI_MASK_REAL_TIME_MESSAGE)
	{ // Realtime messages
		MIDI_addUSBReport(MIDI_UART1_WIRE, messageByte, 0x00, 0x00);
	}
	else
	{
		if ((messageByte & MIDI_MASK_STATUS_BYTE) == MIDI_MASK_STATUS_BYTE)
		{
			*buffUartIndex = 0; // First byte of MIDI message received
		}

		buffUart[*buffUartIndex] = messageByte;

		// Handle first MIDI byte
		if (*buffUartIndex == 0)
		{
			*msgUart = messageByte >> 4; // get midi message

			if (*msgUart == MIDI_MESSAGE_NOTE_OFF ||
					*msgUart == MIDI_MESSAGE_NOTE_ON ||
					*msgUart == MIDI_MESSAGE_KEY_PRESSURE ||
					*msgUart == MIDI_MESSAGE_CONTROL_CHANGE ||
					*msgUart == MIDI_MESSAGE_PROGRAM_CHANGE ||
					*msgUart == MIDI_MESSAGE_CHANNEL_PRESSURE ||
					*msgUart == MIDI_MESSAGE_PITCH_BAND_CHANGE)
			{
				*buffUartIndex = 1;
			}
			else if (messageByte == MIDI_MESSAGE_SONG_SELECT ||
							 messageByte == MIDI_MESSAGE_SONG_POSITION ||
							 messageByte == MIDI_MESSAGE_TIME_CODE_QTR_FRAME)
			{
				*msgUart = messageByte;
				*buffUartIndex = 1;
			}
		}
		// Handle second MIDI byte
		else if (*buffUartIndex == 1)
		{
			if (*msgUart == MIDI_MESSAGE_CHANNEL_PRESSURE ||
					*msgUart == MIDI_MESSAGE_PROGRAM_CHANGE ||
					*msgUart == MIDI_MESSAGE_TIME_CODE_QTR_FRAME ||
					*msgUart == MIDI_MESSAGE_SONG_SELECT)
			{
				MIDI_addUSBReport(MIDI_UART1_WIRE, buffUart[0], buffUart[1], 0x00);
				*buffUartIndex = 1;
			}
			else
			{
				*buffUartIndex = 2;
			}
		}
		// Handle third MIDI byte
		else if (*buffUartIndex == 2)
		{
			if (*msgUart == MIDI_MESSAGE_NOTE_ON ||
					*msgUart == MIDI_MESSAGE_NOTE_OFF ||
					*msgUart == MIDI_MESSAGE_KEY_PRESSURE ||
					*msgUart == MIDI_MESSAGE_CONTROL_CHANGE ||
					*msgUart == MIDI_MESSAGE_SONG_POSITION ||
					*msgUart == MIDI_MESSAGE_PITCH_BAND_CHANGE)
			{
				MIDI_addUSBReport(MIDI_UART1_WIRE, buffUart[0], buffUart[1], buffUart[2]);
			}

			*buffUartIndex = 1;
		}
	}

	if (buffUsbReportNextIndex != 0 && USBD_MIDI_GetState(&hUsbDeviceFS) == MIDI_IDLE)
	{
		if (buffUsbReportNextIndex != MIDI_EPIN_SIZE)
		{
			memset(&buffUsbReport[buffUsbReportNextIndex], 0x00, (MIDI_EPIN_SIZE - buffUsbReportNextIndex));
		}
		USBD_MIDI_SendReport(&hUsbDeviceFS, buffUsbReport, MIDI_EPIN_SIZE);
		buffUsbReportNextIndex = 0;
	}
}
