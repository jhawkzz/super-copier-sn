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
	void DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO);
	void DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO);

	void PrintGameInfo(const ROMHeader& romHeader);

	void ReadHeader(ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t romHeaderAddress);

	void TestAddresses(SNCartIO& snCartIO);

private:
	SNCartIO mSNCartIO;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	ROMHeader mROMHeader;
};
