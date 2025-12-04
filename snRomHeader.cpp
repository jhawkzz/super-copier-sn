
#include "snRomHeader.h"



bool ROMHeader::IsValid()
{
	// use heuristics to figure out if this is valid.

	// Check the title. It should adhere to JIS_X_0201 (https://en.wikipedia.org/wiki/JIS_X_0201)
	for (int32 i = 0; i < 21; i++)
	{
		// valid ranges are:
		// 0x20 -> 0x7F
		// or
		// 0xA1 -> 0xDF
		
		if (mValues.mCartTitle[i] < 0x20)
		{
			return false;
		}

		if (mValues.mCartTitle[i] > 0x7F && mValues.mCartTitle[i] < 0xA1)
		{
			return false;
		}

		if (mValues.mCartTitle[i] > 0xDF)
		{
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
		return false;
	}

	

}
