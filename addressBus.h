#pragma once

#include <cstdint>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "gpioLine.h"

template<int NumLines, int NumBankLines>
class AddressBus
{
public:
	~AddressBus()
	{
		Release();
	}
	
	void Create(gpiod_chip* pChip, uint8_t* pGPIOVals, uint8_t latchVal, uint8_t* pBankGPIOVals, uint8_t bankLatchVal)
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

		for (uint32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].Create(pChip, pBankGPIOVals[i]);
		}

		mLatch.Create(pChip, latchVal);
		mBankLatch.Create(pChip, bankLatchVal);
	}

	void Release()
	{
		mBankLatch.Release();
		mLatch.Release();

		for (uint32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].Release();
		}

		for (uint32_t i = 0; i < NumLines; i++)
		{
			mLines[i].Release();
		}
	}

	void HiZ()
	{
		// prepare the latch 
		mLatch.Write(1);
		WAIT();

		// set the low values
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}

		// prepare the latch 
		mLatch.Write(0);
		WAIT();

		// set the high values
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}


		// prepare the latch 
		mBankLatch.Write(1);
		WAIT();

		// set the low values
		for (int32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].HiZ();
		}

		// prepare the latch 
		mBankLatch.Write(0);
		WAIT();

		// set the high values
		for (int32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].HiZ();
		}
	}

	void SetAddress(uint32_t value)
	{
		uint8_t lowVals = value & 0x00FF;
		uint8_t highVals = (value & 0xFF00) >> 8;
		uint8_t bankVals = (value & 0xFF0000) >> 16;

		//printf("requesting address: %d, low: %d, high: %d\n", value, lowVals, highVals);

		// SET ADDRESS LINES

		// prepare the latch 
		mLatch.Write(1);
		WAIT();

		// set the low values
		for (int32_t i = 0; i < NumLines; i++)
		{
			//printf("Set low value bit %d: %d\n", i, (lowVals >> i) & 0x1);
			mLines[i].Write((lowVals >> i) & 0x1);
		}

		// set the latch 
		mLatch.Write(0);
		WAIT();

		// set the high values
		for (int32_t i = 0; i < NumLines; i++)
		{
			//printf("Set high value bit %d: %d\n", i, (highVals >> i) & 0x1);
			mLines[i].Write((highVals >> i) & 0x1);
		}


		// SET BANK LINES
		uint8_t lowBankVals = bankVals & 0xF;
		uint8_t highBankVals = (bankVals & 0xF0) >> 4;

		// prepare the bank latch 
		mBankLatch.Write(1);
		WAIT();

		// set the low values
		for (int32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].Write((lowBankVals >> i) & 0x1);
		}

		// set the latch
		mBankLatch.Write(0);
		WAIT();

		// set the high values 
		for (int32_t i = 0; i < NumBankLines; i++)
		{
			mBankLines[i].Write((highBankVals >> i) & 0x1);
		}
	}
	
private:
	GPIOLine mLines[NumLines];
	GPIOLine mBankLines[NumBankLines];
	GPIOLine mLatch;
	GPIOLine mBankLatch;
};
