
#include "snRomHeader.h"

#include <cstring>
#include <stdio.h>

bool ROMHeader::IsValid() const
{
	// use heuristics to figure out if this is valid.

	// Check the title. It should adhere to JIS_X_0201 (https://en.wikipedia.org/wiki/JIS_X_0201)
	for (int32_t i = 0; i < 21; i++)
	{
		// valid ranges are:
		// 0x20 -> 0x7F
		// or
		// 0xA1 -> 0xDF
		
		if (mValues.mCartTitle[i] < 0x20)
		{
			printf("Failed title check\n");
			return false;
		}

		if (mValues.mCartTitle[i] > 0x7F && mValues.mCartTitle[i] < 0xA1)
		{
			printf("Failed title check\n");
			return false;
		}

		if (mValues.mCartTitle[i] > 0xDF)
		{
			printf("Failed title check\n");
			return false;
		}
	}

	// Check the map mode
	if (	mValues.mMapMode != MAP_MODE_LOROM_2_68_MHZ
		&&	mValues.mMapMode != MAP_MODE_LOROM_2_68_MHZ
		&&	mValues.mMapMode != MAP_MODE_HIROM_2_68_MHZ
		&&	mValues.mMapMode != MAP_MODE_SA_1
		&&	mValues.mMapMode != MAP_MODE_EX_HIROM_2_68_MHZ
		&&	mValues.mMapMode != MAP_MODE_LOROM_3_58_MHZ
		&&	mValues.mMapMode != MAP_MODE_HIROM_3_58_MHZ
		&&	mValues.mMapMode != MAP_MODE_EX_HIROM_3_58_MHZ
		)
	{
		printf("Failed mapmode check: 0x%x\n", mValues.mMapMode);
		return false;
	}

	// Check the cart type
	if (	mValues.mCartType != CART_TYPE_ROM
		&&	mValues.mCartType != CART_TYPE_ROM_RAM
		&&	mValues.mCartType != CART_TYPE_ROM_RAM_SRAM
		&&	mValues.mCartType != CART_TYPE_ROM_SA1
		&&	mValues.mCartType != CART_TYPE_ROM_SA1_RAM
		&&	mValues.mCartType != CART_TYPE_ROM_SA1_RAM_SRAM
		)
	{
		printf("Failed cartType check: 0x%x\n", mValues.mCartType);
		return false;
	}

	// Make sure the ROM size is reasonable
	if (mValues.mROMSize > ROM_MAX_ALLOWED_SIZE)
	{
		printf("Failed romsize check\n");
		return false;
	}

	// Make sure SRAM size makes sense
	if (mValues.mSRAMSize > SRAM_MAX_ALLOWED_SIZE)
	{
		printf("Failed sram check\n");
		return false;
	}

	// Make sure the country code is within range
	if (mValues.mCountryCode > REGION_CODE_VARIATION_3)
	{
		printf("Failed country code check\n");
		return false;
	}

	// Last but not least, and honestly most importantly, check the checksum.
	// If this is valid, we're as good as gold.
	if ((mValues.mChecksumComplement ^ mValues.mChecksum) != 0xFFFF)
	{
		printf("Failed CRC check\n");
		return false;
	}

	return true;
}

void ROMHeader::Reset()
{
	memset(&mBuffer, 0, sizeof(mBuffer));
}

bool ROMHeader::IsLoROM() const
{
	// We might ultimately want something more specific, but for now LoROM vs Hi is fine.
	if (	mValues.mMapMode == MAP_MODE_LOROM_2_68_MHZ 
		||	mValues.mMapMode == MAP_MODE_LOROM_2_68_MHZ
		||	mValues.mMapMode == MAP_MODE_LOROM_3_58_MHZ
		)
	{
		return true;
	}

	return false;
}

bool ROMHeader::HasSRAM() const
{
	if (mValues.mCartType == CART_TYPE_ROM_RAM_SRAM || mValues.mCartType == CART_TYPE_ROM_SA1_RAM_SRAM)
	{
		return true;
	}

	return false;
}

uint32_t ROMHeader::GetROMSize() const
{
	return (1 << mValues.mROMSize) * 1024;
}

uint32_t ROMHeader::GetSRAMSize() const
{
	return (1 << mValues.mSRAMSize) * 1024;
}

uint32_t ROMHeader::GetNumBanks() const
{
	return GetROMSize() / GetBankSize();
}

uint32_t ROMHeader::GetBankSize() const
{
	if (IsLoROM())
	{
		return LOROM_BANK_SIZE;
	}

	return HIROM_BANK_SIZE;
}

void ROMHeader::GetRegion(char* pRegion, uint32_t size) const
{
	switch (mValues.mCountryCode)
	{
		case REGION_CODE_JAPAN:			strncpy(pRegion, "JP", size - 1);	break;
		case REGION_CODE_USA_CAN:		strncpy(pRegion, "US/CA", size - 1); break;
		case REGION_CODE_EUROPE:		strncpy(pRegion, "EU", size - 1);	break;
		case REGION_CODE_SCANDANAVIA:	strncpy(pRegion, "SE", size - 1);	break;
		case REGION_CODE_FRANCE:		strncpy(pRegion, "FR", size - 1);	break;
		case REGION_CODE_DUTCH:			strncpy(pRegion, "NL", size - 1);	break;
		case REGION_CODE_SPANISH:		strncpy(pRegion, "ES", size - 1);	break;
		case REGION_CODE_GERMAN:		strncpy(pRegion, "DE", size - 1);	break;
		case REGION_CODE_ITALIAN:		strncpy(pRegion, "IT", size - 1);	break;
		case REGION_CODE_CHINESE:		strncpy(pRegion, "CN", size - 1);	break;
		case REGION_CODE_KOREAN:		strncpy(pRegion, "KO", size - 1);	break;
		case REGION_CODE_COMMON:		strncpy(pRegion, "COMMON", size - 1); break;
		case REGION_CODE_CANADA:		strncpy(pRegion, "CA", size - 1);	break;
		case REGION_CODE_BRAZIL:		strncpy(pRegion, "BR", size - 1);	break;
		case REGION_CODE_AUSTRALIA:		strncpy(pRegion, "AU", size - 1);	break;
		case REGION_CODE_VARIATION_1:	strncpy(pRegion, "VAR1", size - 1);	break;
		case REGION_CODE_VARIATION_2:	strncpy(pRegion, "VAR2", size - 1);	break;
		case REGION_CODE_VARIATION_3:	strncpy(pRegion, "VAR3", size - 1);	break;
	}
}

void ROMHeader::GetTitle(char* pTitle, uint32_t size) const
{
	strncpy(pTitle, (const char*)mValues.mCartTitle, size - 1);
}

void ROMHeader::GetMapMode(char* pMapMode, uint32_t size) const
{
	switch(mValues.mMapMode)
	{
		case MAP_MODE_LOROM_2_68_MHZ: strncpy(pMapMode, "LoROM, 2.68 MHz", size - 1); break;
		case MAP_MODE_HIROM_2_68_MHZ:  strncpy(pMapMode, "HiROM, 2.68 MHz", size - 1); break;
		case MAP_MODE_SA_1:				strncpy(pMapMode, "SA1", size - 1); break;
		case MAP_MODE_EX_HIROM_2_68_MHZ:  strncpy(pMapMode, "Ex HiROM, 2.68 MHz", size - 1); break;
		case MAP_MODE_LOROM_3_58_MHZ:   strncpy(pMapMode, "LoROM, 3.58 MHz", size - 1); break;
		case MAP_MODE_HIROM_3_58_MHZ:    strncpy(pMapMode, "HiROM, 3.58 MHz", size - 1); break;
		case MAP_MODE_EX_HIROM_3_58_MHZ:     strncpy(pMapMode, "Ex HiROM, 3.58 MHz", size - 1); break;
	}
}

void ROMHeader::GetCartType(char* pCartType, uint32_t size) const
{
	switch (mValues.mCartType)
	{
		case CART_TYPE_ROM:				strncpy(pCartType, "ROM", size - 1); break;
		case CART_TYPE_ROM_RAM:			strncpy(pCartType, "ROM, RAM", size - 1); break;
		case CART_TYPE_ROM_RAM_SRAM:	strncpy(pCartType, "ROM, RAM, SRAM", size - 1); break;
		case CART_TYPE_ROM_SA1:			strncpy(pCartType, "ROM, SA1", size - 1); break;
		case CART_TYPE_ROM_SA1_RAM:		strncpy(pCartType, "ROM, RAM, SA1", size - 1); break;
		case CART_TYPE_ROM_SA1_RAM_SRAM: strncpy(pCartType, "ROM, RAM, SA1, SRAM", size - 1); break;
	}
}

uint8_t ROMHeader::GetDeveloperId() const
{
	return mValues.mDeveloperID;
}

uint8_t ROMHeader::GetRomVersion() const
{
	return mValues.mROMVersion;
}

uint16_t ROMHeader::GetChecksumComplement() const
{
	return mValues.mChecksumComplement;
}

uint16_t ROMHeader::GetChecksum() const
{
	return mValues.mChecksum;
}
