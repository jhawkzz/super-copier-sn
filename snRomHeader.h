#pragma once

#include <cstdint>

#define ROM_HEADER_SIZE_BYTES		(48)
#define LOROM_HEADER_ADDRESS	(0x7FB0) //MapModes 20 and 23
#define HIROM_HEADER_ADDRESS	(0xFFB0) //MapMode 21
//Tales Of Phantasia
//Dai Kaiju Monogatari
#define EX_HIROM_HEADER_ADDRESS	(0x40FFB0) //MapMode 25

#define EXPANDED_HEADER_PRESENT (0x33)

#define LOROM_BANK_SIZE (32768)
#define HIROM_BANK_SIZE (65536)

// Note, 32kb and 128kb are not supported.
#define EXPANSION_RAM_SIZE_NONE (0x0)
#define EXPANSION_RAM_SIZE_16KB (0x1)
#define EXPANSION_RAM_SIZE_64KB (0x3)
#define EXPANSION_RAM_SIZE_256KB (0x5)
#define EXPANSION_RAM_SIZE_512KB (0x6)
#define EXPANSION_RAM_SIZE_1MB (0x7)

#define MAP_MODE_LOROM_2_68_MHZ		(0x20) //smw
#define MAP_MODE_HIROM_2_68_MHZ		(0x21)
#define MAP_MODE_SA_1				(0x23)
#define MAP_MODE_EX_HIROM_2_68_MHZ	(0x25)
#define MAP_MODE_LOROM_3_58_MHZ		(0x30)
#define MAP_MODE_HIROM_3_58_MHZ		(0x31)
#define MAP_MODE_EX_HIROM_3_58_MHZ	(0x35)

#define CART_TYPE_ROM					(0x00)
#define CART_TYPE_ROM_RAM				(0x01)
#define CART_TYPE_ROM_RAM_BATTERY		(0x02) //smw
#define CART_TYPE_DSP_ROM				(0x03)
#define CART_TYPE_DSP_ROM_RAM			(0x04)
#define CART_TYPE_DSP_ROM_RAM_BATTERY	(0x05)
#define CART_TYPE_DSP_ROM_BATTERY		(0x06)
#define CART_TYPE_SUPERFX_ROM				(0x13)
#define CART_TYPE_SUPERFX_ROM_RAM			(0x14)
#define CART_TYPE_SUPERFX_ROM_RAM_BATTERY	(0x15)
#define CART_TYPE_SUPERFX_ROM_BATTERY		(0x16)
#define CART_TYPE_OBC1_ROM				(0x23)
#define CART_TYPE_OBC1_ROM_RAM			(0x24)
#define CART_TYPE_OBC1_ROM_RAM_BATTERY	(0x25)
#define CART_TYPE_OBC1_ROM_BATTERY		(0x26)
#define CART_TYPE_SA1_ROM			(0x33)
#define CART_TYPE_SA1_ROM_RAM		(0x34)
#define CART_TYPE_SA1_ROM_RAM_BATTERY (0x35)
#define CART_TYPE_SA1_ROM_BATTERY (0x36)
#define CART_TYPE_OTHER_ROM			(0xE3)
#define CART_TYPE_OTHER_ROM_RAM		(0xE4)
#define CART_TYPE_OTHER_ROM_RAM_BATTERY (0xE5)
#define CART_TYPE_OTHER_ROM_BATTERY (0xE6)
#define CART_TYPE_CUSTOM_ROM			(0xF3)
#define CART_TYPE_CUSTOM_ROM_RAM		(0xF4)
#define CART_TYPE_CUSTOM_ROM_RAM_BATTERY (0xF5)
#define CART_TYPE_CUSTOM_ROM_BATTERY (0xF6)

#define ROM_SIZE_512KB (0x9)
#define ROM_SIZE_1MB (0xA)
#define ROM_SIZE_2MB (0xB)
#define ROM_SIZE_4MB (0xC)
#define ROM_SIZE_8MB (0xD)

// Note, 32kb and 128kb are not supported.
#define RAM_SIZE_NONE (0x0)
#define RAM_SIZE_16KB (0x1)
#define RAM_SIZE_64KB (0x3)
#define RAM_SIZE_256KB (0x5)
#define RAM_SIZE_512KB (0x6)
#define RAM_SIZE_1MB (0x7)

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
	bool HasBattery() const;
	bool HasSuperFX() const;
	uint32_t GetROMSize() const;
	uint32_t GetRAMSize() const;
	uint32_t GetNumBanks() const;
	uint32_t GetBankSize() const;
	void GetRegion(char* pRegion, uint32_t size) const;
	void GetTitle(char* pTitle, uint32_t size) const;
	void GetMapMode(char* pMapMode, uint32_t size) const;
	void GetCartType(char* pCartType, uint32_t size) const;
	uint8_t GetDeveloperId_NoExpandedHeader() const;
	uint8_t GetRomVersion() const;
	uint16_t GetChecksumComplement() const;
	uint16_t GetChecksum() const;
	bool HasExpandedHeader() const;
	uint16_t GetMakerCode_ExpandedHeader() const;
	void GetGameCode_ExpandedHeader(char* pGameCode, uint32_t size) const;
	uint32_t GetExpansionRAMSize_ExpandedHeader() const;
	uint8_t GetSpecialVersion_ExpandedHeader() const;
	uint8_t GetCartSubVersion_ExpandedHeader() const;

	// See https://sneslab.net/wiki/SNES_ROM_Header
	struct Values
	{
		/* Start Expansion Header (not used in all games) */
		uint16_t mMakerCode = 0;
		uint8_t mGameCode[5] = { 0 };
		uint8_t mFixedValue[6] = { 0 };
		uint8_t mExpansionRAMSize = 0;
		uint8_t mSpecialVersion = 0;
		uint8_t mCartSubVersion = 0;
		/* End Expansion Header */

		uint8_t mCartTitle[21] = { 0 };
		uint8_t mMapMode = 0;
		uint8_t mCartType = 0;
		uint8_t mROMSize = 0;
		uint8_t mRAMSize = 0;
		uint8_t mCountryCode = 0;
		uint8_t mDeveloperID_Or_ExpandedHeaderFlag = 0; //if 0x33 it means we support the expanded header.
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
