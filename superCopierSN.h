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

	bool CompareDumps(const char* pFileNameA, const char* pFileNameB);
	uint16_t CalcChecksum(const char* pFileName);

	void TestAddresses(SNCartIO& snCartIO);

private:
	SNCartIO mSNCartIO;

	uint8_t mSRAMBuffer[SRAM_BUFFER_SIZE];
	ROMHeader mROMHeader;
};
