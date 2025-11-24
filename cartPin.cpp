
#include "cartPin.h"
#include <stdio.h>

CartPin::~CartPin()
{
	Release();
}

void CartPin::Create(gpiod_chip* pChip, uint8_t gpioVal)
{
	if (!pChip)
	{
		printf("Invalid chip passed!\n");
		return;
	}

	mLine.Create(pChip, gpioVal);
}

void CartPin::Release()
{
	mLine.Release();
}

void CartPin::HiZ()
{
	mLine.HiZ();
}

void CartPin::PullHigh()
{
	mLine.Write(1);
}

void CartPin::PullLow()
{
	mLine.Write(0);
}
