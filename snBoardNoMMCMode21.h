#pragma once

#include <cstdint>
#include <stdio.h>

class ROMHeader;
class SNCartIO;

// Manages reading/writing to a cart with No CoProcessor, or a CoProcessor that does not impact memory mapping
// and which uses Memory Map Mode 21. This is colloqueally known as 'HiROM' and about 25% of snes games.
class SNBoardNoMMCMode21
{
public:
	static void UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer);
	static void DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile);
	
	static void DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile);

private:
	static void UploadToSRAM_Internal(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, uint8_t* pSRAMBuffer);
	static void DownloadFromSRAM_Internal(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, FILE* pOutFile);

	static void SetCartToIdleState(SNCartIO& snCartIO);
	
	static void EnableROM_DisableSRAM(SNCartIO& snCartIO);
	static void DisableROM_EnableSRAM(SNCartIO& snCartIO);
	
	static bool IsSRAMValid(const ROMHeader& romHeader);
	static bool IsROMValid(const ROMHeader& romHeader);
};
