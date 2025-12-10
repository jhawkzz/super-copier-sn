#pragma once

#include <cstdint>
#include "addressBus.h"
#include "dataBus.h"
#include "snCartPins.h"
#include "snRomHeader.h"

#define SRAM_BUFFER_SIZE (1024 * 1024)

// Manages reading/writing to a cart with No CoProcessor, or a CoProcessor that does not impact memory mapping
// and which uses Memory Map Mode 20. This is colloqueally known as 'LoROM' and about 70% of snes games.
class SNCartNoMMCMode20
{
public:
	SNCartNoMMCMode20(const ROMHeader& romHeader);

	void SetCartToIdleState();

	void UploadToSRAM(const ROMHeader& romHeader, uint8_t* pSRAMBuffer);
	void DownloadFromSRAM(const ROMHeader& romHeader, FILE* pOutFile);
	
	void DumpROM(const ROMHeader& romHeader, FILE* pOutFile, bool firstBankOnly);

private:
	AddressBus<8> mAddressBus;
	DataBus<8> mDataBus;

	SNWriteEnablePin mWriteEnablePin;
	SNReadEnablePin mReadEnablePin;
	SNResetPin mResetPin;
	SNCartEnablePin mCartEnablePin;

	ROMHeader& mROMHeader;
};
