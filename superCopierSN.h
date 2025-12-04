#pragma once

#include <cstdint>
#include "snCartPins.h"
#include "snRomHeader.h"
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

	void ReadHeader();

	void TestAddresses();

private:
	AddressBus<8> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNReadEnablePin mReadEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	uint8_t mROMBuffer[ROM_BUFFER_SIZE];
	LoROMHeader mROMHeader;
};
