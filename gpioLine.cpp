
#include "gpioLine.h"

#include <gpiod.h>
#include <stdio.h>
#include "defines.h"

GPIOLine::~GPIOLine()
{
	if (mpLine)
	{
		gpiod_line_release(mpLine);
	}

	mpLine = nullptr;
	mpChip = nullptr;
}

void GPIOLine::Create(gpiod_chip* pChip, uint32_t gpioLineNum)
{
	if (!pChip)
	{
		printf("Invalid chip passed.\n");
		return;
	}

	mpChip = pChip;
	mGPIOLineNum = gpioLineNum;
}

void GPIOLine::HiZ()
{
	ConfigForHiZ();
}

void GPIOLine::Write(uint8_t bit)
{
	ConfigForOutput();

	int32_t result = gpiod_line_set_value(mpLine, (bit & 0x1));
	if (result == -1)
	{
		printf("Failed to write to line.\n");
	}
}

int8_t GPIOLine::Read()
{
	ConfigForInput();

	int8_t value = gpiod_line_get_value(mpLine);
	if (value > -1)
	{
		return value & 0x1;
	}
	else
	{
		printf("Failed to read value for line.\n");
		return -1;
	}
}

void GPIOLine::Release()
{
	if (mpLine)
	{
		gpiod_line_release(mpLine);
		mpLine = nullptr;
	}
}

bool GPIOLine::OpenLine()
{
	if (!mpChip)
	{
		printf("Chip invalid\n");
		return false;
	}

	if (mGPIOLineNum < 0)
	{
		printf("GPIOLineNum not set\n");
		return false;
	}

	mpLine = gpiod_chip_get_line(mpChip, mGPIOLineNum);
	return mpLine != nullptr;
}

void GPIOLine::ConfigForOutput()
{
	if (mDirection != Direction::Output)
	{
		mDirection = Direction::Output;

		Release();

		if (!OpenLine())
		{
			printf("Failed to open line.\n");
			return;
		}

		int32_t result = gpiod_line_request_output(mpLine, PRODUCT_NAME, 0);
		if (result == -1)
		{
			printf("Failed to set line to output.\n");
		}
	}
}

void GPIOLine::ConfigForInput()
{
	if (mDirection != Direction::Input)
	{
		mDirection = Direction::Input;

		Release();

		if (!OpenLine())
		{
			printf("Failed to open line.\n");
			return;
		}

		int32_t result = gpiod_line_request_input_flags(mpLine, PRODUCT_NAME, GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN);
		if (result == -1)
		{
			printf("Failed to set line to input.\n");
		}
	}
}

void GPIOLine::ConfigForHiZ()
{
	if (mDirection != Direction::HiZ)
	{
		mDirection = Direction::HiZ;

		Release();

		if (!OpenLine())
		{
			printf("Failed to open line.\n");
			return;
		}

		int32_t result = gpiod_line_request_input_flags(mpLine, PRODUCT_NAME, GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
		if (result == -1)
		{
			printf("Failed to set line to input.\n");
		}
	}
}
