/**
  * BSD 2-Clause License
  *
  * Copyright (c) 2025, Illia Pikin a.k.a Hypnotriod
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, this
  *    list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  */

#ifndef MIDI_H
#define MIDI_H

#include "stm32f1xx_hal.h"
#include "stdbool.h"

#define MIDI_BUFFER_LENGTH           256
#define MIDI_MAX_CHANNELS_NUM        16
#define MIDI_MAX_CABLES_NUM          16

#define MIDI_MESSAGE_NOTE_OFF          0x08
#define MIDI_MESSAGE_NOTE_ON           0x09
#define MIDI_MESSAGE_KEY_PRESSURE      0x0A
#define MIDI_MESSAGE_CONTROL_CHANGE    0x0B
#define MIDI_MESSAGE_PROGRAM_CHANGE    0x0C
#define MIDI_MESSAGE_CHANNEL_PRESSURE  0x0D
#define MIDI_MESSAGE_PITCH_BAND_CHANGE 0x0E

#define MIDI_MESSAGE_TIMING_CLOCK   0xF8
#define MIDI_MESSAGE_START          0xFA
#define MIDI_MESSAGE_CONTINUE       0xFB
#define MIDI_MESSAGE_STOP           0xFC
#define MIDI_MESSAGE_ACTIVE_SENSING 0xFE
#define MIDI_MESSAGE_SYSTEM_RESET   0xFF

#define MIDI_MESSAGE_TIME_CODE_QTR_FRAME 0xF1
#define MIDI_MESSAGE_SONG_POSITION       0xF2
#define MIDI_MESSAGE_SONG_SELECT         0xF3

#define MIDI_MASK_STATUS_BYTE       0x80
#define MIDI_MASK_REAL_TIME_MESSAGE 0xF8

#define MIDI_MESSAGE_CONTROL_ALL_SOUNDS_OFF        120
#define MIDI_MESSAGE_CONTROL_RESET_ALL_CONTROLLERS 121
#define MIDI_MESSAGE_CONTROL_ALL_NOTES_OFF         123

#define MIDI_MESSAGE_PITCH_BAND_MIDDLE 8192
#define MIDI_MESSAGE_PITCH_BAND_MAX    16383
#define MIDI_MESSAGE_PITCH_BAND_MIN    0

#define MIDI_MESSAGE_NOTE_MAX             127
#define MIDI_MESSAGE_NOTE_VELOCITY_MAX    127
#define MIDI_MESSAGE_CONTROL_VALUE_MAX    127
#define MIDI_MESSAGE_CONTROL_VALUE_MIN    0
#define MIDI_MESSAGE_CONTROL_VALUE_MIDDLE 64

void MIDI_ProcessUSBData(void);
void MIDI_ProcessUARTData(void);

#endif
