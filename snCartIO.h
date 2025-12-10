#pragma once

#include <cstdint>
#include "addressBus.h"
#include "dataBus.h"
#include "snCartPins.h"

class SNCartIO
{
public:
	static SNCartIO& Get();
	~SNCartIO();
	
	void Create(gpiod_chip* pChip);
	void Release();

private:
	SNCartIO() {}

public:
	AddressBus<8> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNReadEnablePin mReadEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;
};
