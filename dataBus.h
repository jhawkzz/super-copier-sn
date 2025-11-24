#pragma once

#include <cstdint>
#include <stdio.h>
#include "gpioLine.h"

template<int NumLines>
class DataBus
{
public:
	~DataBus()
	{
		Release();
	}
	
	void Create(gpiod_chip* pChip, uint8_t* pGPIOVals)
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
	}

	void Release()
	{
		for (uint32_t i = 0; i < NumLines; i++)
		{
			mLines[i].Release();
		}
	}

	void HiZ()
	{
		for (int32_t i = 0; i < NumLines; i++)
		{
			mLines[i].HiZ();
		}
	}

	void Write(uint32_t value)
	{
		for (int32_t i = 0; i < NumLines; i++)
		{
			//todo: optimize this 
			mLines[i].Write((value & (0x1 << i)) != 0 ? 1 : 0);
		}
	}

	uint8_t Read()
	{
		uint8_t value = 0;

		for (int32_t i = 0; i < NumLines; i++)
		{
			uint8_t lineVal = mLines[i].Read();

			//todo: optimize this 
			value |= ((lineVal != 0 ? 0x1 : 0) << i);
		}

		return value;
	}

private:
	GPIOLine mLines[NumLines];
};
