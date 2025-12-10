#pragma once

#include <cstdint>
#include "addressBus.h"
#include "dataBus.h"
#include "snCartPins.h"
#include "snRomHeader.h"

#define SRAM_BUFFER_SIZE (1024 * 1024)

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

	void UploadToSRAM(const ROMHeader& romHeader);
	void UploadToSRAM_MapMode20(const ROMHeader& romHeader, uint8_t* pSRAMBuffer);

	void DownloadFromSRAM(const ROMHeader& romHeader);
	void DownloadFromSRAM_MapMode20(const ROMHeader& romHeader, FILE* pOutFile);
	void DownloadFromSRAM_MapMode21(const ROMHeader& romHeader, uint32_t startingBank, FILE* pOutFile);

	void DumpROM(const ROMHeader& romHeader, bool firstBankOnly);
	void DumpROM_MapMode20(const ROMHeader& romHeader, FILE* pOutFile, bool firstBankOnly);
	void DumpROM_MapMode21(const ROMHeader& romHeader, FILE* pOutFile, bool firstBankOnly);

	void PrintGameInfo(const ROMHeader& romHeader);

	void ReadHeader(ROMHeader& romHeader, uint32_t romHeaderAddress);

	void TestAddresses();

private:
	AddressBus<8> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNReadEnablePin mReadEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	ROMHeader mROMHeader;
};
