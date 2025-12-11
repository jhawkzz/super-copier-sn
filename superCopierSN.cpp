
#include "superCopierSN.h"

#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "snBoardNoMMCMode20.h"
#include "snBoardNoMMCMode21.h"

SuperCopierSN& SuperCopierSN::Get()
{
    static SuperCopierSN Instance;
    return Instance;
}

void SuperCopierSN::Create(gpiod_chip* pChip)
{
    mSNCartIO.Create(pChip);
}

SuperCopierSN::~SuperCopierSN()
{
    Release();
}

void SuperCopierSN::Release()
{
    mSNCartIO.Release();
}

void SuperCopierSN::UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO)
{
    char romTitle[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    romHeader.GetTitle(romTitle, sizeof(romTitle));

    char sramFileName[300] = { 0 };
    snprintf(sramFileName, sizeof(sramFileName) - 1, "./%s_ROMVER_%d.%s", romTitle, romHeader.GetRomVersion(), SRAM_EXTENSION);

    FILE* pFile = fopen(sramFileName, "rb");
    if (!pFile)
    {
        printf("Failed to open file '%s' for read!\n", sramFileName);
        return;
    }

    memset(mSRAMBuffer, 0, sizeof(mSRAMBuffer));
    fread(mSRAMBuffer, romHeader.GetRAMSizeBytes(), 1, pFile);
    fclose(pFile);

    // If no MMC (very, VERY common) we can go off of just the map mode.
    if (!romHeader.HasMMC())
    {
        switch (romHeader.GetMapMode())
        {
            case MapMode::MapMode_20:
            {
                SNBoardNoMMCMode20::UploadToSRAM(romHeader, snCartIO, mSRAMBuffer);
                break;
            }

            case MapMode::MapMode_21:
            {
                SNBoardNoMMCMode21::UploadToSRAM(romHeader, snCartIO, mSRAMBuffer);
                break;
            }

            default:
            {
                printf("Unsupported No-MMC map mode detected. Cannot upload SRAM.\n");
                break;
            }
        }
    }
    else
    {
        // the coProcessor will inform us as to how to dump.
        switch (romHeader.GetCoProcessor())
        {
            default:
            {
                printf("Detected unsupported (as of yet) coprocessor. Cannot upload!\n");
                break;
            }
        }
    }
}

void SuperCopierSN::DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO)
{
    char romTitle[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    romHeader.GetTitle(romTitle, sizeof(romTitle));

    char sramFileName[300] = { 0 };
    snprintf(sramFileName, sizeof(sramFileName) - 1, "./%s_ROMVER_%d.%s", romTitle, romHeader.GetRomVersion(), SRAM_EXTENSION);

    FILE* pFile = fopen(sramFileName, "wb");
    if (!pFile)
    {
        printf("Failed to open file '%s' for write!\n", sramFileName);
        return;
    }

    // If no MMC (very, VERY common) we can go off of just the map mode.
    if (!romHeader.HasMMC())
    {
        switch (romHeader.GetMapMode())
        {
            case MapMode::MapMode_20:
            {
                SNBoardNoMMCMode20::DownloadFromSRAM(romHeader, snCartIO, pFile);
                break;
            }

            case MapMode::MapMode_21:
            {
                SNBoardNoMMCMode21::DownloadFromSRAM(romHeader, snCartIO, pFile);
                break;
            }

            default:
            {
                printf("Unsupported No-MMC map mode detected. Cannot download SRAM.\n");
                break;
            }
        }
    }
    else
    {
        // the coProcessor will inform us as to how to dump.
        switch (romHeader.GetCoProcessor())
        {
            default:
            {
                printf("Detected unsupported (as of yet) coprocessor. Cannot download!\n");
                break;
            }
        }
    }

    fclose(pFile);
    pFile = NULL;
}
      
void SuperCopierSN::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO)
{
    char romTitle[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    romHeader.GetTitle(romTitle, sizeof(romTitle));

    char romFileName[300] = { 0 };
    snprintf(romFileName, sizeof(romFileName) - 1, "./%s_ROMVER_%d.%s", romTitle, romHeader.GetRomVersion(), ROM_EXTENSION);

    FILE* pFile = fopen(romFileName, "wb");
    if (!pFile)
    {
        printf("Failed to open file '%s' for write!\n", romFileName);
        return;
    }

    // If no MMC (very, VERY common) we can go off of just the map mode.
    if (!romHeader.HasMMC())
    {
        switch (romHeader.GetMapMode())
        {
            case MapMode::MapMode_20:
            {
                SNBoardNoMMCMode20::DumpROM(romHeader, snCartIO, pFile);
                break;
            }

            case MapMode::MapMode_21:
            {
                SNBoardNoMMCMode21::DumpROM(romHeader, snCartIO, pFile);
                break;
            }

            default:
            {
                printf("Unsupported No-MMC map mode detected. Cannot upload dump.\n");
                break;
            }
        }
    }
    else
    {
        // the coProcessor will inform us as to how to dump.
        switch (romHeader.GetCoProcessor())
        {
            default:
            {
                printf("Detected unsupported (as of yet) coprocessor. Cannot dump!\n");
                break;
            }
        }
    }
    
    fclose(pFile);
    pFile = NULL;
}

void SuperCopierSN::SetCartToIdleState(SNCartIO& snCartIO)
{
    // Disable writeEnable
    snCartIO.mWriteEnablePin.Disable();
    WAIT();

    snCartIO.mReadEnablePin.Enable();
    WAIT();

    // Put the cartEnable into HiZ
    snCartIO.mCartEnablePin.HiZ();
    WAIT();

    // Put the dataBus into HiZ
    snCartIO.mDataBus.HiZ();
    WAIT();

    // put the address into HiZ
    snCartIO.mAddressBus.HiZ();
    WAIT();
}

void SuperCopierSN::PrintGameInfo(const ROMHeader& romHeader)
{
    char title[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    char region[20] = { 0 };
    char mapMode[50] = { 0 };
    char cartType[50] = { 0 };

    romHeader.GetTitle(title, sizeof(title));
    romHeader.GetRegion(region, sizeof(region));
    romHeader.GetMapModeDisplay(mapMode, sizeof(mapMode));
    romHeader.GetCartType(cartType, sizeof(cartType));

    if (!romHeader.IsValid())
    {
        printf("****ATTENTION: ROM APPEARS TO BE CORRUPT****\n");
    }

    printf("Game Info\n");
    printf("=-=-=-=-=-\n");
    printf("   General\n");
    printf("   -------------------\n");
    printf("   Title: %s\n", title);
    printf("   Region: %s\n", region);
    if (!romHeader.HasExpandedHeader())
    {
        printf("   Developer ID: %d\n", romHeader.GetDeveloperId_NoExpandedHeader());
    }
    printf("   -------------------\n");
    printf("\n");
    printf("   Cart Features\n");
    printf("   -------------\n");
    printf("   Map Mode: %s\n", mapMode);
    printf("   Cart Type: %s\n", cartType);
    printf("   Has Battery: %s\n", romHeader.HasBattery() ? "Yes" : "No");
    printf("   Rom Version: %d\n", romHeader.GetRomVersion());
    printf("   -------------------\n");
    printf("\n");
    printf("   Chip Sizes\n");
    printf("   -------------\n");
    printf("   ROM Size: %d bytes\n", romHeader.GetROMSizeBytes());
    printf("   Num Banks: %d\n", romHeader.GetNumBanks());
    printf("   Bank Size: %d bytes\n", romHeader.GetBankSizeBytes());
    printf("   RAM Size: %d bytes %s\n", romHeader.GetRAMSizeBytes(), romHeader.HasSuperFX() ? "(Super FX)" : "");
    printf("   -------------\n");
    printf("\n");
    printf("   Checksum\n");
    printf("   --------\n");
    printf("   Checksum Complement: 0x%x\n", romHeader.GetChecksumComplement());
    printf("   Checksum: 0x%x\n", romHeader.GetChecksum());
    printf("   --------\n");
    if (romHeader.HasExpandedHeader())
    {
        printf("\n");
        char gameCode[GAME_CODE_LEN_BYTES + 1] = { 0 };
        romHeader.GetGameCode_ExpandedHeader(gameCode, sizeof(gameCode));

        printf("   Expanded Header Features\n");
        printf("   ------------------------\n");
        printf("   Game Code: %s\n", gameCode);
        printf("   Cart SubVersion: %d\n", romHeader.GetCartSubVersion_ExpandedHeader());
        printf("   Special Version: %d\n", romHeader.GetSpecialVersion_ExpandedHeader());
        printf("   Expansion RAM Size: %d bytes\n", romHeader.GetExpansionRAMSizeBytes_ExpandedHeader());
        printf("   Maker Code (Developer): 0x%x\n", romHeader.GetMakerCode_ExpandedHeader());
        printf("   ------------------------\n");
    }
    printf("=-=-=-=-=-\n");
}

void SuperCopierSN::TestAddresses(SNCartIO& snCartIO)
{
    SetCartToIdleState(snCartIO);
    WAIT();

    printf("Setting Cart State to Idle\n");
    printf("/WR:Disabled\n");
    printf("/RD:Enabled\n");
    printf("/CART:Disabled\n");
    printf("DataBus:HiZ\n");
    printf("AddressBus:HiZ\n");

    printf("Press any key to continue.\n");
    while (!getchar())
    {
    }

    printf("Walking memory addresses.\n");
    for (uint32_t i = 0; i < 0xFFFFFF; i++)
    {
        // Disable the cartEnable (disable the rom/sram chips)
        snCartIO.mCartEnablePin.Disable();
        WAIT();

        // Set the address to read a byte from
        printf("Setting Address to: %x\n", i);
        snCartIO.mAddressBus.SetAddress(i);
        WAIT();

        // Set the dataBus to HiZ
        snCartIO.mDataBus.HiZ();
        WAIT();

        // Now we're read, so enable the cartEnable
        snCartIO.mCartEnablePin.Enable();
        WAIT();

        // Grab the byte off the lines
        uint8_t value = snCartIO.mDataBus.Read();
        WAIT();
        printf("Address %x: Data %x\n", i, value);

        snCartIO.mDataBus.HiZ();
        WAIT();
        
        printf("Press X to stop. Press any other key to continue.\n");
        int inputChoice = 0;
        int c = 0;
        do
        {
            c = getchar();
            if (c != '\n')
            {
                inputChoice = c;
            }
        } 
        while (c != '\n');

        if (inputChoice == 'x')
        {
            break;
        }
    }
}

void SuperCopierSN::ReadHeader(ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t romHeaderAddress)
{
    romHeader.Reset();

    SetCartToIdleState(snCartIO);
    WAIT();

    // Disable the cartEnable (disable the rom/sram chips)
    snCartIO.mCartEnablePin.Disable();
    WAIT();

    for (uint32_t i = 0; i < ROM_HEADER_SIZE_BYTES; i++)
    {
        // Disable the cartEnable (disable the rom/sram chips)
        snCartIO.mCartEnablePin.Disable();
        WAIT();

        // Set the address to read a byte from
        snCartIO.mAddressBus.SetAddress(i + romHeaderAddress);
        WAIT();

        // Set the dataBus to HiZ
        snCartIO.mDataBus.HiZ();
        WAIT();

        // Now we're read, so enable the cartEnable
        snCartIO.mCartEnablePin.Enable();
        WAIT();

        // Grab the byte off the lines
        uint8_t value = snCartIO.mDataBus.Read();
        WAIT();
        romHeader.mBuffer[i] = value;
        
        snCartIO.mDataBus.HiZ();
        WAIT();
    }
}

void SuperCopierSN::Execute()
{
    // Setup for a game to be inserted.
    SetCartToIdleState(mSNCartIO);
    WAIT();

    // Enable the Reset Pin so the SRAM (if there is one) draws from battery and not 5v)
    // it's critical that we ENABLE Reset, so that the transistor for sram (if there is one)
    // doesn't toggle to 5v the moment it touches the 5v line, causing bits to suddenly flip.
    mSNCartIO.mResetPin.Enable();
    WAIT();
    
    printf("INSERT GAME and press any key.\n");
    while(!getchar())
    {
    }

    // Set Reset High so we send sram 5v and can read/write.
    mSNCartIO.mResetPin.Disable();
    WAIT();

    // try reading the header and checking it. If its no good, see if its HiRom
    printf("CHECKING ROM HEADER\n");
    printf("=-=-=-=-=-=-=-=-=-=\n");
    ReadHeader(mROMHeader, mSNCartIO, HEADER_ADDRESS_MAPMODE_NOT_25);
    if (!mROMHeader.IsValid())
    {
        ReadHeader(mROMHeader, mSNCartIO, HEADER_ADDRESS_MAPMODE_25);
    }

    char gameName[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    mROMHeader.GetTitle(gameName, sizeof(gameName));

    bool shouldExit = false;	
    while(!shouldExit)
    {
        printf("\n");
        printf("SuperCopier SN\n");
        printf("=-=-=-=-=-=-=-\n");
        PrintGameInfo(mROMHeader);
        printf("[D]ownload from SRAM\n");
        printf("[U]pload to SRAM\n");
        printf("Dump [R]OM\n");
        printf("[T]est Addresses\n");
        printf("E[x]it\n");
        printf("=-=-=-=-=-=-=-\n");
        printf("Your Selection: ");

        uint32_t inputChoice = 0;
        uint32_t c = 0;
        do
        {
            c = getchar();
            if(c != '\n')
            {
                inputChoice = c;
            }
        }
        while(c != '\n');
        
        printf("\n\n");
            
        switch(inputChoice)
        {
            case 'd':
            {
                DownloadFromSRAM(mROMHeader, mSNCartIO);
                break;
            }
            
            case 'u':
            {
                UploadToSRAM(mROMHeader, mSNCartIO);
                break;
            }
            
            case 'r':
            {
                DumpROM(mROMHeader, mSNCartIO);
                break;
            }

            case 't':
            {
                TestAddresses(mSNCartIO);
                break;
            }
            
            case 'x':
            {
                shouldExit = true;
                break;
            }

            default:
            {
                printf("Unknown choice '%c'\n", inputChoice);
                break;
            }
        }
    }
    
    // Configure lines for removing cartridge
    SetCartToIdleState(mSNCartIO);
    WAIT();

    mSNCartIO.mResetPin.Enable();
    WAIT();
        
    printf("REMOVE CARTRIDGE and press any key.\n");
    while(!getchar())
    {
    }
}
