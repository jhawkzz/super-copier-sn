#pragma once

#include <cstdint>
#include <stdio.h>

class ROMHeader;
class SNCartIO;

// Manages reading/writing to a cart with No CoProcessor, or a CoProcessor that does not impact memory mapping
// and which uses Memory Map Mode 20. This is colloqueally known as 'LoROM' and about 70% of snes games.
class SNBoardNoMMCMode20
{
public:
	static void UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer);
	static void DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile);
	
	static void DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly);

private:
	static void SetCartToIdleState(SNCartIO& snCartIO);
	static void EnableROMAndSRAMChips(SNCartIO& snCartIO);
	static void DisableROMAndSRAMChips(SNCartIO& snCartIO);
};

//todo: test this refactor. verified HiROM works! just double check LoROM
// smw download rom, download / upload sram