#pragma once

#include <cstdint>
#include "snCartPins.h"
#include "dataBus.h"
#include "addressBus.h"

#define SRAM_BUFFER_SIZE (1024 * 1024)
#define ROM_BUFFER_SIZE (1024 * 1024 * 10)

#define LOROM_HEADER_BYTES (32)
#define ROM_HEADER_ADDRESS (0x7FC0)

struct LoROMHeader
{
	LoROMHeader() {}

	struct Values
	{
		uint8_t mCartTitle[21] = { 0 };
		uint8_t mROMType = 0;
		uint8_t mChipset = 0;
		uint8_t mROMSize = 0;
		uint8_t mRAMSize = 0;
		uint8_t mCountryCode = 0;
		uint8_t mDeveloperID = 0;
		uint8_t mROMVersion = 0;
		uint8_t mChecksumComplement[2] = { 0 };
		uint8_t mChecksum[2] = { 0 };
	};

	union
	{
		Values mValues;
		uint8_t mBuffer[sizeof(Values)] = { 0 };
	};
};

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
	// Controls address lines A0 - A15 with support of a latch and A16-A23 (Bank Addresses BA0-BA7) with another latch.
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
