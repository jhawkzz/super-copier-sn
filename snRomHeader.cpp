
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
    if (   mValues.mMapMode != MAP_MODE_LOROM_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_HIROM_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_SA_1
        && mValues.mMapMode != MAP_MODE_EX_HIROM_2_68_MHZ
        && mValues.mMapMode != MAP_MODE_LOROM_3_58_MHZ
        && mValues.mMapMode != MAP_MODE_HIROM_3_58_MHZ
        && mValues.mMapMode != MAP_MODE_EX_HIROM_3_58_MHZ)
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
    if (   mValues.mROMSize != ROM_SIZE_512KB
        && mValues.mROMSize != ROM_SIZE_1MB
        && mValues.mROMSize != ROM_SIZE_2MB
        && mValues.mROMSize != ROM_SIZE_4MB
        && mValues.mROMSize != ROM_SIZE_8MB)
    {
        printf("ROM Check Failed: Found Value: 0x%x\n", mValues.mROMSize);
        return false;
    }

    // Check RAM
    if (   mValues.mRAMSize != RAM_SIZE_NONE
        && mValues.mRAMSize != RAM_SIZE_16KB
        && mValues.mRAMSize != RAM_SIZE_64KB
        && mValues.mRAMSize != RAM_SIZE_256KB
        && mValues.mRAMSize != RAM_SIZE_512KB
        && mValues.mRAMSize != RAM_SIZE_1MB)
    {
        printf("RAM Check Failed: Found Value: 0x%x\n", mValues.mRAMSize);
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

bool ROMHeader::IsLoROM() const
{
    if ( mValues.mMapMode == MAP_MODE_LOROM_2_68_MHZ || mValues.mMapMode == MAP_MODE_LOROM_3_58_MHZ )
    {
        return true;
    }

    return false;
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

uint32_t ROMHeader::GetROMSize() const
{
    // In practice, we could do 2^ROMSize. However, we're reporting the chip size,
    // not the actual amount of ROM _used_.
    switch (mValues.mROMSize)
    {
        case ROM_SIZE_512KB: return 512 * 1024;
        case ROM_SIZE_1MB:   return 1 * 1024 * 1024;
        case ROM_SIZE_2MB:   return 2 * 1024 * 1024;
        case ROM_SIZE_4MB:   return 4 * 1024 * 1024;
        case ROM_SIZE_8MB:   return 8 * 1024 * 1024;
        default:             break;
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

uint32_t ROMHeader::GetRAMSize() const
{
    // For SuperFX games, its stored in the ExpansionRAM spot.
    if (HasSuperFX())
    {
        return GetExpansionRAMSize_ExpandedHeader();
    }
    else
    {
        // In practice, we could do 2^RamSize. However, we're reporting the chip size,
        // not the actual amount of Ram _used_.
        switch (mValues.mRAMSize)
        {
            case RAM_SIZE_NONE:  return 0;
            case RAM_SIZE_16KB:  return 16 * 1024;
            case RAM_SIZE_64KB:  return 64 * 1024;
            case RAM_SIZE_256KB: return 256 * 1024;
            case RAM_SIZE_512KB: return 512 * 1024;
            case RAM_SIZE_1MB:   return 1024 * 1024;
            default:             break;
        }
    }

    return 0;
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

void ROMHeader::GetMapMode(char* pMapMode, uint32_t size) const
{
    switch(mValues.mMapMode)
    {
        case MAP_MODE_LOROM_2_68_MHZ:    strncpy(pMapMode, "LoROM, 2.68 MHz", size - 1);    break;
        case MAP_MODE_HIROM_2_68_MHZ:    strncpy(pMapMode, "HiROM, 2.68 MHz", size - 1);    break;
        case MAP_MODE_SA_1:              strncpy(pMapMode, "SA1", size - 1);                break;
        case MAP_MODE_EX_HIROM_2_68_MHZ: strncpy(pMapMode, "Ex HiROM, 2.68 MHz", size - 1); break;
        case MAP_MODE_LOROM_3_58_MHZ:    strncpy(pMapMode, "LoROM, 3.58 MHz", size - 1);    break;
        case MAP_MODE_HIROM_3_58_MHZ:    strncpy(pMapMode, "HiROM, 3.58 MHz", size - 1);    break;
        case MAP_MODE_EX_HIROM_3_58_MHZ: strncpy(pMapMode, "Ex HiROM, 3.58 MHz", size - 1); break;
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

uint32_t ROMHeader::GetExpansionRAMSize_ExpandedHeader() const
{
    // In practice, we could do 2^mExpansionRAMSize. However, we're reporting the chip size,
    // not the actual amount of mExpansionRAMSize _used_.
    switch (mValues.mExpansionRAMSize)
    {
        case EXPANSION_RAM_SIZE_NONE:  return 0;
        case EXPANSION_RAM_SIZE_16KB:  return 16 * 1024;
        case EXPANSION_RAM_SIZE_64KB:  return 64 * 1024;
        case EXPANSION_RAM_SIZE_256KB: return 256 * 1024;
        case EXPANSION_RAM_SIZE_512KB: return 512 * 1024;
        case EXPANSION_RAM_SIZE_1MB:   return 1024 * 1024;
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
