/*
Copyright (c) 2014-2016 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Include guard
#pragma once

#include <Arduino.h>

// Delay_basic is only for avrs. With ARM sending is currently not possible
// TODO implement sending
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif

//==============================================================================
// Protocol Definitions
//==============================================================================

// NEC
// IRP notation:
// {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ				38000UL
#define NEC_PULSE			564UL
#define NEC_ADDRESS_LENGTH	16
#define NEC_COMMAND_LENGTH	16
#define NEC_DATA_LENGTH		(NEC_ADDRESS_LENGTH + NEC_COMMAND_LENGTH)
#define NEC_BLOCKS			(NEC_DATA_LENGTH / 8)
// 2 for lead + space, each block has mark and space
#define NEC_LENGTH			(2 + NEC_DATA_LENGTH * 2)
#define NEC_TIMEOUT			(NEC_PULSE * 78UL)
#define NEC_TIMEOUT_HOLDING (NEC_PULSE * 173UL)
#define NEC_TIMEOUT_REPEAT	(NEC_TIMEOUT + NEC_LOGICAL_LEAD \
							+ NEC_LOGICAL_ZERO * (NEC_DATA_LENGTH / 2) \
							+ NEC_LOGICAL_ONE * (NEC_DATA_LENGTH / 2))
#define NEC_MARK_LEAD		(NEC_PULSE * 16UL)
#define NEC_SPACE_LEAD		(NEC_PULSE * 8UL)
#define NEC_SPACE_HOLDING	(NEC_PULSE * 4UL)
#define NEC_LOGICAL_LEAD	(NEC_MARK_LEAD + NEC_SPACE_LEAD)
#define NEC_LOGICAL_HOLDING	(NEC_MARK_LEAD + NEC_SPACE_HOLDING)
#define NEC_MARK_ZERO		(NEC_PULSE * 1UL)
#define NEC_MARK_ONE		(NEC_PULSE * 1UL)
#define NEC_SPACE_ZERO		(NEC_PULSE * 1UL)
#define NEC_SPACE_ONE		(NEC_PULSE * 3UL)
#define NEC_LOGICAL_ZERO	(NEC_MARK_ZERO + NEC_SPACE_ZERO)
#define NEC_LOGICAL_ONE		(NEC_MARK_ONE + NEC_SPACE_ONE)

// Enum as unique number for each protocol
enum Nec_type_t : uint8_t {
	IRL_NEC_NO_PROTOCOL = 0x00,
	IRL_NEC,
	IRL_NEC_REPEAT,
};

// Struct that is returned by the read() function
struct Nec_data_t
{
	Nec_type_t protocol;
	uint16_t address;
	uint8_t command;
};

//==============================================================================
// Nec Decoding Class
//==============================================================================

class CNec
{
public:
	// Attach the interrupt so IR signals are detected
	inline bool begin(uint8_t pin);
	inline bool end(uint8_t pin);

	// User API to access library data
	inline bool available(void);
	inline Nec_data_t read(void);
	inline uint32_t timeout(void);
	inline uint32_t lastEvent(void);

protected:
	// Enum as unique number for each protocol
	static volatile Nec_type_t protocol;

	// Temporary buffer to hold bytes for decoding the protocol
	static uint8_t countNec;
	static uint8_t dataNec[NEC_BLOCKS];

	// Data that all protocols need for decoding
	static volatile uint8_t IRLProtocol;

	// Time values for the last interrupt and the last valid protocol
	static uint32_t mlastTime;
	static volatile uint32_t mlastEvent;

	// Interrupt function that is attached
	static inline void interrupt(void);
};

extern CNec Nec;

//==============================================================================
// API Class
//==============================================================================

typedef void(*NecEventCallback)(void);

template<const NecEventCallback callback, const uint16_t address = 0x0000>
class CNecAPI : public CNec
{
public:
	// User API to access library data
	inline void read(void);
	inline uint8_t command(void);
	inline uint8_t pressCount(void);
	inline uint8_t holdCount(const uint8_t debounce = 0);
	inline uint8_t pressTimeout(void);
	inline bool releaseButton (void);
	inline void reset(void);

protected:
	// Differenciate between timeout types
	enum TimeoutType : uint8_t
	{
		NO_TIMEOUT, 	// Keydown
		TIMEOUT, 		// Key release with timeout
		NEXT_BUTTON, 	// Key release, pressed again
		NEW_BUTTON, 	// Key release, another key is pressed
	} NecTimeoutType;

	// Keep track which key was pressed/held down how often
	uint8_t lastCommand = 0;
	uint8_t lastPressCount = 0;
	uint8_t lastHoldCount = 0;
};

// Include protocol implementation
#include "IRL_Nec.hpp"