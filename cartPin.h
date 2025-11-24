#pragma once

#include <cstdint>
#include "gpioLine.h"

class CartPin
{
public:
	virtual ~CartPin();

	virtual void Create(gpiod_chip* pChip, uint8_t gpioVal);
	virtual void Release();
	void HiZ();

protected:
	void PullHigh();
	void PullLow();

private:
	GPIOLine mLine;
};
