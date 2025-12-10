#pragma once

#include <cstdint>

#define ROM_HEADER_SIZE_BYTES         (48)
#define HEADER_ADDRESS_MAPMODE_NOT_25 (0xFFB0) //Sounds silly, but ALL games put it here, regardless of co-processors, except mapmode 25 games.
#define HEADER_ADDRESS_MAPMODE_25     (0x40FFB0)

#define GAME_TITLE_LEN_BYTES (21)
#define GAME_CODE_LEN_BYTES   (4)

// From 1990-1993(ish) this was the DeveloperID. Then they re-allocated it as a signal that an Expanded Header exists.
// (Which is actually just the 16 bytes preceding what WAS the start of the header, heh.)
#define EXPANDED_HEADER_PRESENT (0x33)

#define MAP_MODE_20_ROM_BANK_BASE_ADDRESS (0x8000)
#define MAP_MODE_20_ROM_START_BANK        (0x0)
#define MAP_MODE_20_BANK_SIZE             (32768)
#define MAP_MODE_20_SRAM_START_BANK       (0x70)

#define MAP_MODE_21_ROM_BANK_BASE_ADDRESS (0x0)
#define MAP_MODE_21_ROM_START_BANK        (0xC0)
#define MAP_MODE_21_BANK_SIZE             (65536)

// HiROM sram is complicated.
/*
* Board               ROM Area      ROM Mirrors   SRAM Area
  Type                at 0000-FFFF  at 8000-FFFF  at 6000-7FFF
  SHVC-1J3B-01        40-7d,c0-ff   00-3f,80-bf   20-3f,a0-bf
  SHVC-1J1M-11,20     40-7d,c0-ff   00-3f,80-bf   20-3f,a0-bf
  SHVC-1J3M-01,11,20  40-7d,c0-ff   00-3f,80-bf   20-3f,a0-bf
  SHVC-BJ3M-10        40-7d,c0-ff   00-3f,80-bf   20-3f,a0-bf [FF3]
  SHVC-1J5M-11,20     40-7d,c0-ff   00-3f,80-bf   20-3f,a0-bf [Uncharted Waters, 256kb so important]

  SHVC-2J3M-01,11,20  40-7d,c0-ff   00-3f,80-bf   10-1f,30-3f,90-9f,b0-bf [Secret of Mana, 7th Saga]
  SHVC-2J5M-01        40-7d,c0-ff   00-3f,80-bf   10-1f,90-9f,30-3f,b0-bf [Brandish	USA	SNS-QF-0, Operation Europe: Path To Victory 1939-45	USA	SNS-YP-0]
  SHVC-LJ3M-01        40-7d,c0-ff   00-3f,80-bf   80-bf [Tales of Phantasia]
*/
// Common because MOST games use this mapping, and it goes from banks 0x20 to 0x3F, at 8192 bytes per bank.
// No game shipped using more than 32KiB, which would only require reading 0x20->0x23.
// 
// A FEW boards store it at 10-1F. Again, we only need to read 0x10->0x13.
#define MAP_MODE_21_SRAM_START_BANK_COMMON_START (0x20) 

// SoM, 7th Saga, Brandish, Operation Europe: Path to Victory, etc.
#define MAP_MODE_21_SRAM_START_BANK_UNCOMMON_START (0x10) 

#define MAP_MODE_21_SRAM_BANK_BASE_ADDRESS (0x6000)
#define MAP_MODE_21_SRAM_BANK_SIZE         (8192)

// This is the biggest sram chip that shipped on map mode 20 carts. (Mario Paint used this size). 
// Theoretically bigger chips could be supported, but their mapping would have depended on the board used.
// Older boards mapped it to 64KiB banks, newer boards would have mapped it only to the lower 32KiB and spread it across banks.
// https://problemkaputt.de/fullsnes.htm
#define MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE (32768) 

#define MAP_MODE_20_2_68_MHZ (0x20) //smw
#define MAP_MODE_21_2_68_MHZ (0x21)
#define MAP_MODE_23_SA_1     (0x23)
#define MAP_MODE_25_2_68_MHZ (0x25)
#define MAP_MODE_20_3_58_MHZ (0x30)
#define MAP_MODE_21_3_58_MHZ (0x31)
#define MAP_MODE_25_3_58_MHZ (0x35)

#define CART_TYPE_ROM                     (0x00)
#define CART_TYPE_ROM_RAM                 (0x01)
#define CART_TYPE_ROM_RAM_BATTERY         (0x02) //smw
#define CART_TYPE_DSP_ROM                 (0x03)
#define CART_TYPE_DSP_ROM_RAM             (0x04)
#define CART_TYPE_DSP_ROM_RAM_BATTERY     (0x05)
#define CART_TYPE_DSP_ROM_BATTERY         (0x06)
#define CART_TYPE_SUPERFX_ROM             (0x13)
#define CART_TYPE_SUPERFX_ROM_RAM         (0x14)
#define CART_TYPE_SUPERFX_ROM_RAM_BATTERY (0x15)
#define CART_TYPE_SUPERFX_ROM_BATTERY     (0x16)
#define CART_TYPE_OBC1_ROM                (0x23)
#define CART_TYPE_OBC1_ROM_RAM            (0x24)
#define CART_TYPE_OBC1_ROM_RAM_BATTERY    (0x25)
#define CART_TYPE_OBC1_ROM_BATTERY        (0x26)
#define CART_TYPE_SA1_ROM                 (0x33)
#define CART_TYPE_SA1_ROM_RAM             (0x34)
#define CART_TYPE_SA1_ROM_RAM_BATTERY     (0x35)
#define CART_TYPE_SA1_ROM_BATTERY         (0x36)
#define CART_TYPE_OTHER_ROM               (0xE3)
#define CART_TYPE_OTHER_ROM_RAM           (0xE4)
#define CART_TYPE_OTHER_ROM_RAM_BATTERY   (0xE5)
#define CART_TYPE_OTHER_ROM_BATTERY       (0xE6)
#define CART_TYPE_CUSTOM_ROM              (0xF3)
#define CART_TYPE_CUSTOM_ROM_RAM          (0xF4)
#define CART_TYPE_CUSTOM_ROM_RAM_BATTERY  (0xF5)
#define CART_TYPE_CUSTOM_ROM_BATTERY      (0xF6)

#define ROM_SIZE_3MBit_4MBit   (0x9)
#define ROM_SIZE_5MBit_8MBit   (0xA)
#define ROM_SIZE_9MBit_16MBit  (0xB)
#define ROM_SIZE_17MBit_32MBit (0xC)
#define ROM_SIZE_33MBit_64MBit (0xD)

// Note, 32kbit and 128kbit are not supported.
#define RAM_SIZE_NONE    (0x0)
#define RAM_SIZE_16KBit  (0x1)
#define RAM_SIZE_64KBit  (0x3)
#define RAM_SIZE_256KBit (0x5)
#define RAM_SIZE_512KBit (0x6)
#define RAM_SIZE_1MBit   (0x7)

// Note, 32kbit and 128kbit are not supported.
#define EXPANSION_RAM_SIZE_NONE    (0x0)
#define EXPANSION_RAM_SIZE_16KBit  (0x1)
#define EXPANSION_RAM_SIZE_64KBit  (0x3)
#define EXPANSION_RAM_SIZE_256KBit (0x5)
#define EXPANSION_RAM_SIZE_512KBit (0x6)
#define EXPANSION_RAM_SIZE_1MBit   (0x7)

#define REGION_CODE_JAPAN       (0x0)
#define REGION_CODE_USA_CAN     (0x1)
#define REGION_CODE_EUROPE      (0x2)
#define REGION_CODE_SCANDANAVIA (0x3)
#define REGION_CODE_FRANCE      (0x6)
#define REGION_CODE_DUTCH       (0x7)
#define REGION_CODE_SPANISH     (0x8)
#define REGION_CODE_GERMAN      (0x9)
#define REGION_CODE_ITALIAN     (0xA)
#define REGION_CODE_CHINESE     (0xB)
#define REGION_CODE_KOREAN      (0xD)
#define REGION_CODE_COMMON      (0xE)
#define REGION_CODE_CANADA      (0xF)
#define REGION_CODE_BRAZIL      (0x10)
#define REGION_CODE_AUSTRALIA   (0x11)
#define REGION_CODE_VARIATION_1 (0x12)
#define REGION_CODE_VARIATION_2 (0x13)
#define REGION_CODE_VARIATION_3 (0x14)

enum class CoProcessor
{
    None,
    DSP,
    SuperFX,
    OBC1,
    SA1,
    Unknown
};

enum class MapMode
{
    MapMode_20, //LoROM
    MapMode_21, //HiROM
    MapMode_23, //SA-1
    MapMode_25, //ExHiROM
    MapMode_Unknown
};

struct ROMHeader
{
    ROMHeader() {}

    bool IsValid() const;
    void Reset();
    CoProcessor GetCoProcessor() const;
    MapMode GetMapMode() const;
    bool HasBattery() const;
    bool HasSuperFX() const;
    uint32_t GetROMSizeBytes() const;
    uint32_t GetRAMSizeBytes() const;
    uint32_t GetNumBanks() const;
    uint32_t GetBankSizeBytes() const;
    void GetRegion(char* pRegion, uint32_t size) const;
    void GetTitle(char* pTitle, uint32_t size) const;
    void GetMapModeDisplay(char* pMapMode, uint32_t size) const;
    void GetCartType(char* pCartType, uint32_t size) const;
    uint8_t GetDeveloperId_NoExpandedHeader() const;
    uint8_t GetRomVersion() const;
    uint16_t GetChecksumComplement() const;
    uint16_t GetChecksum() const;
    bool HasExpandedHeader() const;
    uint16_t GetMakerCode_ExpandedHeader() const;
    void GetGameCode_ExpandedHeader(char* pGameCode, uint32_t size) const;
    uint32_t GetExpansionRAMSizeBytes_ExpandedHeader() const;
    uint8_t GetSpecialVersion_ExpandedHeader() const;
    uint8_t GetCartSubVersion_ExpandedHeader() const;

    // See https://sneslab.net/wiki/SNES_ROM_Header
    struct Values
    {
        /* Start Expansion Header (not used in all games) */
        uint16_t mMakerCode = 0;
        uint8_t  mGameCode[5] = { 0 };
        uint8_t  mFixedValue[6] = { 0 };
        uint8_t  mExpansionRAMSizeKBit = 0;
        uint8_t  mSpecialVersion = 0;
        uint8_t  mCartSubVersion = 0;
        /* End Expansion Header */

        uint8_t  mCartTitle[21] = { 0 };
        uint8_t  mMapMode = 0;
        uint8_t  mCartType = 0;
        uint8_t  mROMSizeMBit = 0;
        uint8_t  mRAMSizeKBit = 0;
        uint8_t  mCountryCode = 0;
        uint8_t  mDeveloperID_Or_ExpandedHeaderFlag = 0; //if 0x33 it means we support the expanded header.
        uint8_t  mROMVersion = 0;
        uint16_t mChecksumComplement = 0;
        uint16_t mChecksum = 0;
    };

    union
    {
        Values  mValues;
        uint8_t mBuffer[sizeof(Values)] = { 0 };
    };
};
