#pragma once

#include <cstdint>

#define HEADER_SIZE_BYTES	(32)
#define ROM_HEADER_ADDRESS	(0x7FC0)

#define MAP_MODE_LOROM_2_68_MHZ		(0x20) //smw
#define MAP_MODE_HIROM_2_68_MHZ		(0x21)
#define MAP_MODE_SA_1				(0x23)
#define MAP_MODE_EX_HIROM_2_68_MHZ	(0x25)
#define MAP_MODE_LOROM_3_58_MHZ		(0x30)
#define MAP_MODE_HIROM_3_58_MHZ		(0x31)
#define MAP_MODE_EX_HIROM_3_58_MHZ	(0x35)

#define CART_TYPE_ROM (0x00)
#define CART_TYPE_ROM_RAM (0x01)
#define CART_TYPE_ROM_RAM_SRAM (0x02) //smw
#define CART_TYPE_ROM_SA1 (0x33)
#define CART_TYPE_ROM_SA1_RAM (0x34)
#define CART_TYPE_ROM_SA1_RAM_SRAM (0x35)

struct ROMHeader
{
	bool IsValid();

	// See https://sneslab.net/wiki/SNES_ROM_Header
	struct Values
	{
		uint8_t mCartTitle[21] = { 0 };
		uint8_t mMapMode = 0;
		uint8_t mCartType = 0;
		uint8_t mROMSize = 0;
		uint8_t mRAMSize = 0;
		uint8_t mCountryCode = 0;
		uint8_t mDeveloperID = 0;
		uint8_t mROMVersion = 0;
		uint8_t mChecksumComplement[2] = { 0 };
		uint8_t mChecksum[2] = { 0 };
	};

	union
	{
		Values mValues = { 0 };
		uint8_t mBuffer[sizeof(Values)] = { 0 };
	};
};
