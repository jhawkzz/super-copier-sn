#pragma once

#include "addressBus.h"
#include "dataBus.h"
#include "snCartPins.h"

class SNCartIO
{
public:
	~SNCartIO();

	void Create(gpiod_chip* pChip);
	void Release();

public:
	AddressBus<8> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNReadEnablePin mReadEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;
};
