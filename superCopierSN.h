#pragma once

#include <cstdint>
#include "snCartIO.h"
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

	void SetCartToIdleState(SNCartIO& snCartIO);

	void UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO);
	void UploadToSRAM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer);

	void DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO);
	void DownloadFromSRAM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile);
	void DownloadFromSRAM_MapMode21(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, FILE* pOutFile);

	void DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, bool firstBankOnly);
	void DumpROM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly);
	void DumpROM_MapMode21(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly);

	void PrintGameInfo(const ROMHeader& romHeader);

	void ReadHeader(ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t romHeaderAddress);

	void TestAddresses(SNCartIO& snCartIO);

private:
	SNCartIO mSNCartIO;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	ROMHeader mROMHeader;
};
