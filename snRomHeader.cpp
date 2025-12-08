
#define _CRT_SECURE_NO_WARNINGS

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
            printf("Title Check Failed: Found Value: 0x%x\n", mValues.mCartTitle[i]);
            return false;
        }

        if (mValues.mCartTitle[i] > 0x7F && mValues.mCartTitle[i] < 0xA1)
        {
            printf("Title Check Failed: Found Value: 0x%x\n", mValues.mCartTitle[i]);
            return false;
        }

        if (mValues.mCartTitle[i] > 0xDF)
        {
            printf("Title Check Failed: Found Value: 0x%x\n", mValues.mCartTitle[i]);
            return false;
        }
    }

    // Check the map mode
    if (   mValues.mMapMode != MAP_MODE_20_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_21_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_23_SA_1
        && mValues.mMapMode != MAP_MODE_25_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_20_3_58_MHZ
        && mValues.mMapMode != MAP_MODE_21_3_58_MHZ
        && mValues.mMapMode != MAP_MODE_25_3_58_MHZ)
    {
        printf("Map Mode Check Failed: Found Value: 0x%x\n", mValues.mMapMode);
        return false;
    }

    // Check the cart type
    if (   mValues.mCartType != CART_TYPE_ROM
        && mValues.mCartType != CART_TYPE_ROM_RAM
        && mValues.mCartType != CART_TYPE_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_DSP_ROM
        && mValues.mCartType != CART_TYPE_DSP_ROM_RAM
        && mValues.mCartType != CART_TYPE_DSP_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_DSP_ROM_BATTERY
        && mValues.mCartType != CART_TYPE_SUPERFX_ROM
        && mValues.mCartType != CART_TYPE_SUPERFX_ROM_RAM
        && mValues.mCartType != CART_TYPE_SUPERFX_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_SUPERFX_ROM_BATTERY
        && mValues.mCartType != CART_TYPE_OBC1_ROM
        && mValues.mCartType != CART_TYPE_OBC1_ROM_RAM
        && mValues.mCartType != CART_TYPE_OBC1_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_OBC1_ROM_BATTERY
        && mValues.mCartType != CART_TYPE_SA1_ROM
        && mValues.mCartType != CART_TYPE_SA1_ROM_RAM
        && mValues.mCartType != CART_TYPE_SA1_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_SA1_ROM_BATTERY
        && mValues.mCartType != CART_TYPE_OTHER_ROM
        && mValues.mCartType != CART_TYPE_OTHER_ROM_RAM
        && mValues.mCartType != CART_TYPE_OTHER_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_OTHER_ROM_BATTERY
        && mValues.mCartType != CART_TYPE_CUSTOM_ROM
        && mValues.mCartType != CART_TYPE_CUSTOM_ROM_RAM
        && mValues.mCartType != CART_TYPE_CUSTOM_ROM_RAM_BATTERY
        && mValues.mCartType != CART_TYPE_CUSTOM_ROM_BATTERY)
    {
        printf("Cart Type Check Failed: Found Value: 0x%x\n", mValues.mCartType);
        return false;
    }

    // Check ROM
    if (   mValues.mROMSizeMBit != ROM_SIZE_3MBit_4MBit
        && mValues.mROMSizeMBit != ROM_SIZE_5MBit_8MBit
        && mValues.mROMSizeMBit != ROM_SIZE_9MBit_16MBit
        && mValues.mROMSizeMBit != ROM_SIZE_17MBit_32MBit
        && mValues.mROMSizeMBit != ROM_SIZE_33MBit_64MBit)
    {
        printf("ROM Check Failed: Found Value: 0x%x\n", mValues.mROMSizeMBit);
        return false;
    }

    // Check RAM
    if (   mValues.mRAMSizeKBit != RAM_SIZE_NONE
        && mValues.mRAMSizeKBit != RAM_SIZE_16KBit
        && mValues.mRAMSizeKBit != RAM_SIZE_64KBit
        && mValues.mRAMSizeKBit != RAM_SIZE_256KBit
        && mValues.mRAMSizeKBit != RAM_SIZE_512KBit
        && mValues.mRAMSizeKBit != RAM_SIZE_1MBit)
    {
        printf("RAM Check Failed: Found Value: 0x%x\n", mValues.mRAMSizeKBit);
        return false;
    }

    // Make sure the country code is within range
    if (mValues.mCountryCode > REGION_CODE_VARIATION_3)
    {
        printf("Country Code Check Failed: Found Value: 0x%x\n", mValues.mCountryCode);
        return false;
    }

    // Last but not least, and honestly most importantly, check the checksum.
    // If this is valid, we're as good as gold.
    if ((mValues.mChecksumComplement ^ mValues.mChecksum) != 0xFFFF)
    {
        printf("Checksum Failed: Checksum: 0x%x, Complement: 0x%x\n", mValues.mChecksum, mValues.mChecksumComplement);
        return false;
    }

    return true;
}

void ROMHeader::Reset()
{
    memset(&mBuffer, 0, sizeof(mBuffer));
}

CoProcessor ROMHeader::GetCoProcessor() const
{
    // the cart type might tell us, but we may have to get more specific later.
    switch (mValues.mCartType)
    {
        case CART_TYPE_ROM:
        case CART_TYPE_ROM_RAM:
        case CART_TYPE_ROM_RAM_BATTERY:
        {
            return CoProcessor::None;
        }

        case CART_TYPE_DSP_ROM: 
        case CART_TYPE_DSP_ROM_RAM:
        case CART_TYPE_DSP_ROM_RAM_BATTERY:
        case CART_TYPE_DSP_ROM_BATTERY:
        {
            return CoProcessor::DSP;
        }

        case CART_TYPE_SUPERFX_ROM:
        case CART_TYPE_SUPERFX_ROM_RAM:
        case CART_TYPE_SUPERFX_ROM_RAM_BATTERY:
        case CART_TYPE_SUPERFX_ROM_BATTERY:
        {
            return CoProcessor::SuperFX;
        }

        case CART_TYPE_OBC1_ROM:
        case CART_TYPE_OBC1_ROM_RAM:
        case CART_TYPE_OBC1_ROM_RAM_BATTERY:
        case CART_TYPE_OBC1_ROM_BATTERY:
        {
            return CoProcessor::OBC1;
        }

        case CART_TYPE_SA1_ROM:
        case CART_TYPE_SA1_ROM_RAM:
        case CART_TYPE_SA1_ROM_RAM_BATTERY:
        case CART_TYPE_SA1_ROM_BATTERY:
        {
            return CoProcessor::SA1;
        }

        case CART_TYPE_OTHER_ROM:
        case CART_TYPE_OTHER_ROM_RAM:
        case CART_TYPE_OTHER_ROM_RAM_BATTERY:
        case CART_TYPE_OTHER_ROM_BATTERY:
        case CART_TYPE_CUSTOM_ROM:
        case CART_TYPE_CUSTOM_ROM_RAM:
        case CART_TYPE_CUSTOM_ROM_RAM_BATTERY:
        case CART_TYPE_CUSTOM_ROM_BATTERY:
        {
            //todo: inspect the title of the game or something like that.
            return CoProcessor::Unknown;
        }

        default:
        {
            break;
        }
    }

    return CoProcessor::Unknown;
}

MapMode ROMHeader::GetMapMode() const
{
    switch (mValues.mMapMode)
    {
        case MAP_MODE_20_2_68_MHZ: return MapMode::MapMode_20;
        case MAP_MODE_21_2_68_MHZ: return MapMode::MapMode_21;
        case MAP_MODE_23_SA_1:     return MapMode::MapMode_23;
        case MAP_MODE_25_2_68_MHZ: return MapMode::MapMode_25;
        case MAP_MODE_20_3_58_MHZ: return MapMode::MapMode_20;
        case MAP_MODE_21_3_58_MHZ: return MapMode::MapMode_21;
        case MAP_MODE_25_3_58_MHZ: return MapMode::MapMode_25;
        default:                   break;
    }

    return MapMode::MapMode_Unknown;
}

bool ROMHeader::HasBattery() const
{
    switch (mValues.mCartType)
    {
        case CART_TYPE_ROM_RAM_BATTERY:
        case CART_TYPE_DSP_ROM_RAM_BATTERY:
        case CART_TYPE_DSP_ROM_BATTERY:
        case CART_TYPE_SUPERFX_ROM_RAM_BATTERY:
        case CART_TYPE_SUPERFX_ROM_BATTERY:
        case CART_TYPE_OBC1_ROM_RAM_BATTERY:
        case CART_TYPE_OBC1_ROM_BATTERY:
        case CART_TYPE_SA1_ROM_RAM_BATTERY:
        case CART_TYPE_SA1_ROM_BATTERY:
        case CART_TYPE_OTHER_ROM_RAM_BATTERY:
        case CART_TYPE_OTHER_ROM_BATTERY:
        case CART_TYPE_CUSTOM_ROM_RAM_BATTERY:
        case CART_TYPE_CUSTOM_ROM_BATTERY:
        {
            return true;
        }

        default:
        {
            break;
        }
    }

    return false;
}

uint32_t ROMHeader::GetROMSizeBytes() const
{
    // See https://sneslab.net/wiki/SNES_ROM_Header says we could do 2^ROMSize,
    // which is does WORK, but is not correct. The SNES docs say this is a lookup table,
    // and not an algorithm.
    switch (mValues.mROMSizeMBit)
    {
        case ROM_SIZE_3MBit_4MBit:   return 512 * 1024; //4Mbits == 512 KBytes
        case ROM_SIZE_5MBit_8MBit:   return 1024 * 1024; //8MBits == 1 MByte
        case ROM_SIZE_9MBit_16MBit:  return 2 * 1024 * 1024; //16MBits == 2 MBytes
        case ROM_SIZE_17MBit_32MBit: return 4 * 1024 * 1024; //32MBits == 4 MBytes
        case ROM_SIZE_33MBit_64MBit: return 8 * 1024 * 1024; //64MBits == 8 MBytes
        default:                     break;
    }

    return 0;
}

bool ROMHeader::HasSuperFX() const
{
    switch (mValues.mCartType)
    {
        case CART_TYPE_SUPERFX_ROM:
        case CART_TYPE_SUPERFX_ROM_RAM:
        case CART_TYPE_SUPERFX_ROM_RAM_BATTERY:
        case CART_TYPE_SUPERFX_ROM_BATTERY:
        {
            return true;
        }
    }

    return false;
}

uint32_t ROMHeader::GetRAMSizeBytes() const
{
    // For SuperFX games, its stored in the ExpansionRAM spot.
    if (HasSuperFX())
    {
        return GetExpansionRAMSizeBytes_ExpandedHeader();
    }
    else
    {
        // See https://sneslab.net/wiki/SNES_ROM_Header says we could do 2^RAMSize,
        // which is does WORK, but is not correct. The SNES docs say this is a lookup table,
        // and not an algorithm.
        switch (mValues.mRAMSizeKBit)
        {
            case RAM_SIZE_NONE:    return 0;
            case RAM_SIZE_16KBit:  return 2 * 1024; //16 KBits == 2 KBytes
            case RAM_SIZE_64KBit:  return 8 * 1024; //64 KBits == 8 KBytes
            case RAM_SIZE_256KBit: return 32 * 1024; //256 KBits == 32 KBytes
            case RAM_SIZE_512KBit: return 64 * 1024; //512 KBits == 64 KBytes
            case RAM_SIZE_1MBit:   return 128 * 1024; //1Megabit == 128 KBytes
            default:               break;
        }
    }

    return 0;
}

uint32_t ROMHeader::GetNumBanks() const
{
    return GetROMSizeBytes() / GetBankSizeBytes();
}

uint32_t ROMHeader::GetBankSizeBytes() const
{
    switch (GetMapMode())
    {
        // LoROM/SA-1
        case MapMode::MapMode_20:
        case MapMode::MapMode_23:
        {
            return MAP_MODE_20_BANK_SIZE;
        }

        // HiROM/ExHiROM
        case MapMode::MapMode_21:
        case MapMode::MapMode_25:
        {
            return MAP_MODE_21_BANK_SIZE;
        }

        case MapMode::MapMode_Unknown:
        {
            break;
        }
    }

    return 0;
}

void ROMHeader::GetRegion(char* pRegion, uint32_t size) const
{
    switch (mValues.mCountryCode)
    {
        case REGION_CODE_JAPAN:       strncpy(pRegion, "JP", size - 1);	    break;
        case REGION_CODE_USA_CAN:     strncpy(pRegion, "US/CA", size - 1);  break;
        case REGION_CODE_EUROPE:      strncpy(pRegion, "EU", size - 1);	    break;
        case REGION_CODE_SCANDANAVIA: strncpy(pRegion, "SE", size - 1);	    break;
        case REGION_CODE_FRANCE:      strncpy(pRegion, "FR", size - 1);	    break;
        case REGION_CODE_DUTCH:       strncpy(pRegion, "NL", size - 1);	    break;
        case REGION_CODE_SPANISH:     strncpy(pRegion, "ES", size - 1);	    break;
        case REGION_CODE_GERMAN:      strncpy(pRegion, "DE", size - 1);	    break;
        case REGION_CODE_ITALIAN:     strncpy(pRegion, "IT", size - 1);	    break;
        case REGION_CODE_CHINESE:     strncpy(pRegion, "CN", size - 1);	    break;
        case REGION_CODE_KOREAN:      strncpy(pRegion, "KO", size - 1);	    break;
        case REGION_CODE_COMMON:      strncpy(pRegion, "COMMON", size - 1); break;
        case REGION_CODE_CANADA:      strncpy(pRegion, "CA", size - 1);	    break;
        case REGION_CODE_BRAZIL:      strncpy(pRegion, "BR", size - 1);	    break;
        case REGION_CODE_AUSTRALIA:   strncpy(pRegion, "AU", size - 1);	    break;
        case REGION_CODE_VARIATION_1: strncpy(pRegion, "VAR1", size - 1);   break;
        case REGION_CODE_VARIATION_2: strncpy(pRegion, "VAR2", size - 1);   break;
        case REGION_CODE_VARIATION_3: strncpy(pRegion, "VAR3", size - 1);   break;
    }
}

void ROMHeader::GetTitle(char* pTitle, uint32_t size) const
{
    int sizeToCopy = size - 1 < sizeof(mValues.mCartTitle) + 1 ? size - 1 : sizeof(mValues.mCartTitle) + 1;

    strncpy(pTitle, (const char*)mValues.mCartTitle, sizeToCopy);
    pTitle[sizeToCopy] = 0;
}

void ROMHeader::GetMapModeDisplay(char* pMapMode, uint32_t size) const
{
    switch(mValues.mMapMode)
    {
        case MAP_MODE_20_2_68_MHZ: strncpy(pMapMode, "Map Mode 20 (LoROM), 2.68 MHz", size - 1);   break;
        case MAP_MODE_21_2_68_MHZ: strncpy(pMapMode, "Map Mode 21 (HiROM), 2.68 MHz", size - 1);   break;
        case MAP_MODE_23_SA_1:     strncpy(pMapMode, "Map Mode 23 (SA1)", size - 1);               break;
        case MAP_MODE_25_2_68_MHZ: strncpy(pMapMode, "Map Mode 25 (ExHiROM), 2.68 MHz", size - 1); break;
        case MAP_MODE_20_3_58_MHZ: strncpy(pMapMode, "Map Mode 20 (LoROM), 3.58 MHz", size - 1);   break;
        case MAP_MODE_21_3_58_MHZ: strncpy(pMapMode, "Map Mode 21 (HiROM), 3.58 MHz", size - 1);   break;
        case MAP_MODE_25_3_58_MHZ: strncpy(pMapMode, "Map Mode 25 (ExHiROM), 3.58 MHz", size - 1); break;
    }
}

void ROMHeader::GetCartType(char* pCartType, uint32_t size) const
{
    switch (mValues.mCartType)
    {
        case CART_TYPE_ROM:                     strncpy(pCartType, "ROM", size - 1);                                            break;
        case CART_TYPE_ROM_RAM:                 strncpy(pCartType, "ROM, RAM", size - 1);                                       break;
        case CART_TYPE_ROM_RAM_BATTERY:         strncpy(pCartType, "ROM, RAM, Battery Backup", size - 1);                       break;
        case CART_TYPE_DSP_ROM:                 strncpy(pCartType, "ROM, CoProcessor: DSP", size - 1);                          break;
        case CART_TYPE_DSP_ROM_RAM:             strncpy(pCartType, "ROM, RAM, CoProcessor: DSP", size - 1);                     break;
        case CART_TYPE_DSP_ROM_RAM_BATTERY:     strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: DSP", size - 1);     break;
        case CART_TYPE_DSP_ROM_BATTERY:         strncpy(pCartType, "ROM, Battery Backup, CoProcessor: DSP", size - 1);          break;
        case CART_TYPE_SUPERFX_ROM:             strncpy(pCartType, "ROM, CoProcessor: SuperFX", size - 1);                      break;
        case CART_TYPE_SUPERFX_ROM_RAM:         strncpy(pCartType, "ROM, RAM, CoProcessor: SuperFX", size - 1);                 break;
        case CART_TYPE_SUPERFX_ROM_RAM_BATTERY: strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: SuperFX", size - 1); break;
        case CART_TYPE_SUPERFX_ROM_BATTERY:     strncpy(pCartType, "ROM, Battery Backup, CoProcessor: SuperFX", size - 1);      break;
        case CART_TYPE_OBC1_ROM:                strncpy(pCartType, "ROM, CoProcessor: OBC1", size - 1);                         break;
        case CART_TYPE_OBC1_ROM_RAM:            strncpy(pCartType, "ROM, RAM, CoProcessor: OBC1", size - 1);                    break;
        case CART_TYPE_OBC1_ROM_RAM_BATTERY:    strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: OBC1", size - 1);    break;
        case CART_TYPE_OBC1_ROM_BATTERY:        strncpy(pCartType, "ROM, Battery Backup, CoProcessor: OBC1", size - 1);         break;
        case CART_TYPE_SA1_ROM:                 strncpy(pCartType, "ROM, CoProcessor: SA1", size - 1);                          break;
        case CART_TYPE_SA1_ROM_RAM:             strncpy(pCartType, "ROM, RAM, CoProcessor: SA1", size - 1);                     break;
        case CART_TYPE_SA1_ROM_RAM_BATTERY:     strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: SA1", size - 1);     break;
        case CART_TYPE_SA1_ROM_BATTERY:         strncpy(pCartType, "ROM, Battery Backup, CoProcessor: SA1", size - 1);          break;
        case CART_TYPE_OTHER_ROM:               strncpy(pCartType, "ROM, CoProcessor: Other", size - 1);                        break;
        case CART_TYPE_OTHER_ROM_RAM:           strncpy(pCartType, "ROM, RAM, CoProcessor: Other", size - 1);                   break;
        case CART_TYPE_OTHER_ROM_RAM_BATTERY:   strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: Other", size - 1);   break;
        case CART_TYPE_OTHER_ROM_BATTERY:       strncpy(pCartType, "ROM, Battery Backup, CoProcessor: Other", size - 1);        break;
        case CART_TYPE_CUSTOM_ROM:              strncpy(pCartType, "ROM, CoProcessor: Custom", size - 1);                       break;
        case CART_TYPE_CUSTOM_ROM_RAM:          strncpy(pCartType, "ROM, RAM, CoProcessor: Custom", size - 1);                  break;
        case CART_TYPE_CUSTOM_ROM_RAM_BATTERY:  strncpy(pCartType, "ROM, RAM, Battery Backup, CoProcessor: Custom", size - 1);  break;
        case CART_TYPE_CUSTOM_ROM_BATTERY:      strncpy(pCartType, "ROM, Battery Backup, CoProcessor: Custom", size - 1);       break;
        default:                                                                                                                break;
    }
}

uint8_t ROMHeader::GetDeveloperId_NoExpandedHeader() const
{
    // This is not valid if its value is EXPANDED_HEADER_PRESENT. Instead, use GetExpandedHeader_GetMakerId()
    return mValues.mDeveloperID_Or_ExpandedHeaderFlag;
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

bool ROMHeader::HasExpandedHeader() const
{
    return mValues.mDeveloperID_Or_ExpandedHeaderFlag == EXPANDED_HEADER_PRESENT;
}

uint16_t ROMHeader::GetMakerCode_ExpandedHeader() const
{
    return mValues.mMakerCode;
}

uint32_t ROMHeader::GetExpansionRAMSizeBytes_ExpandedHeader() const
{
    // See https://sneslab.net/wiki/SNES_ROM_Header says we could do 2^RAMSize,
    // which is does WORK, but is not correct. The SNES docs say this is a lookup table,
    // and not an algorithm.
    switch (mValues.mExpansionRAMSizeKBit)
    {
        case EXPANSION_RAM_SIZE_NONE:    return 0;
        case EXPANSION_RAM_SIZE_16KBit:  return 2 * 1024; //16 KBits == 2 KBytes
        case EXPANSION_RAM_SIZE_64KBit:  return 8 * 1024; //64 KBits == 8 KBytes
        case EXPANSION_RAM_SIZE_256KBit: return 32 * 1024; //256 KBits == 32 KBytes
        case EXPANSION_RAM_SIZE_512KBit: return 64 * 1024; //512 KBits == 64 KBytes
        case EXPANSION_RAM_SIZE_1MBit:   return 128 * 1024; //1Megabit == 128 KBytes
        default: break;
    }

    return 0;
}

uint8_t ROMHeader::GetSpecialVersion_ExpandedHeader() const
{
    return mValues.mSpecialVersion;
}

uint8_t ROMHeader::GetCartSubVersion_ExpandedHeader() const
{
    return mValues.mCartSubVersion;
}

void ROMHeader::GetGameCode_ExpandedHeader(char* pGameCode, uint32_t size) const
{
    int sizeToCopy = size - 1 < sizeof(mValues.mGameCode) + 1 ? size - 1 : sizeof(mValues.mGameCode) + 1;

    strncpy(pGameCode, (const char*)mValues.mGameCode, sizeToCopy);
    pGameCode[sizeToCopy] = 0;
}
