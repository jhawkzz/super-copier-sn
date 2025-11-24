#pragma once

#include <cstdint>
#include "snCartPins.h"
#include "dataBus.h"
#include "addressBus.h"

#define SRAM_BUFFER_SIZE (1024 * 1024)
#define ROM_BUFFER_SIZE (1024 * 1024 * 10)

class SuperCopierSN
{
public:
	static SuperCopierSN& Get();
	~SuperCopierSN();
	
	void Create(gpiod_chip* pChip);
	void Release();

	void Execute();

private:
	SuperCopierSN() {}

	void SetCartToIdleState();

	void UploadToSRAM(const char* pRomName, uint32_t sramSize);
	void DownloadFromSRAM(const char* pRomName, uint32_t sramSize);

	void DumpROM(const char* pRomName, uint32_t numBanks, uint32_t bankSize);

	void PrintGameInfo(const char* pRomName, uint32_t numBanks, uint32_t bankSize, uint32_t sramSize);

private:
	// Controls address lines A0 - A15 with support of a latch and A16-A23 (Bank Addresses BA0-BA7) with another latch.
	AddressBus<8, 4> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	uint8_t mROMBuffer[ROM_BUFFER_SIZE];
};
