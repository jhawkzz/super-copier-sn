#pragma once

#include <cstdint>

#define ROM_HEADER_SIZE_BYTES		(32)
#define LOROM_HEADER_ADDRESS	(0x7FC0)
#define HIROM_HEADER_ADDRESS	(0xFFC0)

#define LOROM_BANK_SIZE (32768)
#define HIROM_BANK_SIZE (65536)

#define MAP_MODE_LOROM_2_68_MHZ		(0x20) //smw
#define MAP_MODE_HIROM_2_68_MHZ		(0x21)
#define MAP_MODE_SA_1				(0x23)
#define MAP_MODE_EX_HIROM_2_68_MHZ	(0x25)
#define MAP_MODE_LOROM_3_58_MHZ		(0x30)
#define MAP_MODE_HIROM_3_58_MHZ		(0x31)
#define MAP_MODE_EX_HIROM_3_58_MHZ	(0x35)

#define CART_TYPE_ROM				(0x00)
#define CART_TYPE_ROM_RAM			(0x01)
#define CART_TYPE_ROM_RAM_SRAM		(0x02) //smw
#define CART_TYPE_ROM_SA1			(0x33)
#define CART_TYPE_ROM_SA1_RAM		(0x34)
#define CART_TYPE_ROM_SA1_RAM_SRAM	(0x35)

// Realistically this is the biggest snes game (Star Ocean)
#define ROM_MAX_ALLOWED_SIZE	(0xD) //2^15 means 8,192 MB
#define SRAM_MAX_ALLOWED_SIZE	(0x7) //Defined in the snes spec, sram limit is 2^7 (except when its 0 and using SuperFX)

#define REGION_CODE_JAPAN		(0x0)
#define REGION_CODE_USA_CAN		(0x1)
#define REGION_CODE_EUROPE		(0x2)
#define REGION_CODE_SCANDANAVIA (0x3)
#define REGION_CODE_FRANCE		(0x6)
#define REGION_CODE_DUTCH		(0x7)
#define REGION_CODE_SPANISH		(0x8)
#define REGION_CODE_GERMAN		(0x9)
#define REGION_CODE_ITALIAN		(0xA)
#define REGION_CODE_CHINESE		(0xB)
#define REGION_CODE_KOREAN		(0xD)
#define REGION_CODE_COMMON		(0xE)
#define REGION_CODE_CANADA		(0xF)
#define REGION_CODE_BRAZIL		(0x10)
#define REGION_CODE_AUSTRALIA	(0x11)
#define REGION_CODE_VARIATION_1 (0x12)
#define REGION_CODE_VARIATION_2 (0x13)
#define REGION_CODE_VARIATION_3 (0x14)

struct ROMHeader
{
	ROMHeader() {}

	bool IsValid() const;
	void Reset();
	bool IsLoROM() const;
	bool HasSRAM() const;
	uint32_t GetROMSize() const;
	uint32_t GetSRAMSize() const;
	uint32_t GetNumBanks() const;
	uint32_t GetBankSize() const;
	void GetRegion(char* pRegion, uint32_t size) const;
	void GetTitle(char* pTitle, uint32_t size) const;
	void GetMapMode(char* pMapMode, uint32_t size) const;
	void GetCartType(char* pCartType, uint32_t size) const;
	uint8_t GetDeveloperId() const;
	uint8_t GetRomVersion() const;
	uint16_t GetChecksumComplement() const;
	uint16_t GetChecksum() const;

	// See https://sneslab.net/wiki/SNES_ROM_Header
	struct Values
	{
		uint8_t mCartTitle[21] = { 0 };
		uint8_t mMapMode = 0;
		uint8_t mCartType = 0;
		uint8_t mROMSize = 0;
		uint8_t mSRAMSize = 0;
		uint8_t mCountryCode = 0;
		uint8_t mDeveloperID = 0;
		uint8_t mROMVersion = 0;
		uint16_t mChecksumComplement = 0;
		uint16_t mChecksum = 0;
	};

	union
	{
		Values mValues;
		uint8_t mBuffer[sizeof(Values)] = { 0 };
	};
};
