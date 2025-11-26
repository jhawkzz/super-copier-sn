#pragma once

#include <cstdint>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "gpioLine.h"

template<int NumLines>
class AddressBus
{
public:
	~AddressBus()
	{
		Release();
	}
	
	void Create(gpiod_chip* pChip, uint8_t* pGPIOVals, uint8_t latch1Val, uint8_t latch2Val)
	{
		if (!pChip)
		{
			printf("Invalid chip passed!\n");
			return;
		}

		for (uint32_t i = 0; i < NumLines; i++)
		{
			mLines[i].Create(pChip, pGPIOVals[i]);
		}

		mLatch1.Create(pChip, latch1Val);
		mLatch2.Create(pChip, latch2Val);
	}

	void Release()
	{
		mLatch2.Release();
		mLatch1.Release();

		for (uint32_t i = 0; i < NumLines; i++)
		{
			mLines[i].Release();
		}
	}

	void HiZ()
	{
		// Set lines 0-7:
		// open the latch
		mLatch1.Write(1);
		WAIT();

		// set lines 0-7
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}

		// latch them
		mLatch1.Write(0);
		WAIT();
		//

		// Set lines 8-15:
		// open the latch
		mLatch2.Write(1);
		WAIT();

		// set lines 8-15
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}

		// latch them
		mLatch2.Write(0);
		WAIT();
		//

		// Set lines 16-23:
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}
	}

	void SetAddress(uint32_t value)
	{
		uint8_t addr0Thru7 = value & 0x00FF;
		uint8_t addr8Thru15 = (value & 0xFF00) >> 8;
		uint8_t addr16Thru23 = (value & 0xFF0000) >> 16;

		//printf("requesting address: %d, low: %d, high: %d\n", value, lowVals, highVals);

		// SET ADDRESS LINES 0-7
		// Prepare
		mLatch1.Write(1);
		WAIT();

		// Set
		for (int32_t i = 0; i < NumLines; i++)
		{
			//printf("Set low value bit %d: %d\n", i, (lowVals >> i) & 0x1);
			mLines[i].Write((addr0Thru7 >> i) & 0x1);
		}

		// Latch
		mLatch1.Write(0);
		WAIT();
		// END ADDRESS LINES 0-7

		// SET ADDRESS LINES 8-15
		// Prepare
		mLatch2.Write(1);
		WAIT();

		// Set
		for (int32_t i = 0; i < NumLines; i++)
		{
			//printf("Set high value bit %d: %d\n", i, (highVals >> i) & 0x1);
			mLines[i].Write((addr8Thru15 >> i) & 0x1);
		}

		// Latch
		mLatch2.Write(0);
		WAIT();
		// END ADDRESS LINES 8-15

		// SET ADDRESS LINES 16-23
		for (int32_t i = 0; i < NumLines; i++)
		{
			//printf("Set high value bit %d: %d\n", i, (highVals >> i) & 0x1);
			mLines[i].Write((addr16Thru23 >> i) & 0x1);
		}
		// END ADDRESS LINES 16-23
	}
	
private:
	GPIOLine mLines[NumLines];
	GPIOLine mLatch1;
	GPIOLine mLatch2;
};
