
#include "superCopierSN.h"

#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "snBoardNoMMCMode20.h"
#include "snBoardNoMMCMode21.h"

/*
* A $49 SNES cartridge health checker and save manager.

• Instantly tells you whether a game cartridge is electrically stable or failing
• Detects hidden ROM corruption that won’t show up at the title screen
• Backup and restore SRAM saves (including writing saves back to carts)
• Optional ROM dumping for personal archiving

Know whether a game is worth keeping before the return window closes.

"This device has been physically tested against every officially released SNES cartridge."
bonus points, i should hunt down every variant too.
I even ordered games from japan

setup a table with games at ridiculous save percentages, and obvious visuals of bad vs good games.
tlel someone "go buy a game somewhere, and bring it here to test"
*/

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

uint16_t SuperCopierSN::CalcChecksum(const char* pFileName)
{
    FILE* pFile = fopen(pFileName, "rb");
    if (!pFile)
    {
        printf("Can't compute checksum. Couldn't open file '%s'!\n", pFileName);
        return 0;
    }

    // calc checksum
    fseek(pFile, 0, SEEK_END);

    uint32_t size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    uint16_t checkSum = 0;
    for (uint32_t i = 0; i < size; i++)
    {
        uint8_t byte = 0;
        fread(&byte, 1, 1, pFile);

        checkSum += byte;
    }

    return checkSum;
}

bool SuperCopierSN::CompareDumps(const char* pFileNameA, const char* pFileNameB)
{
    FILE* pFileA = nullptr;
    FILE* pFileB = nullptr;

    bool success = false;

    do
    {
        pFileA = fopen(pFileNameA, "rb");
        if (!pFileA)
        {
            printf("CompareDumps: Couldn't open file '%s' for A comparison\n", pFileNameA);
            break;
        }

        pFileB = fopen(pFileNameB, "rb");
        if (!pFileB)
        {
            printf("CompareDumps: Couldn't open file '%s' for B comparison\n", pFileNameB);
            break;
        }

        // check lengths
        fseek(pFileA, 0, SEEK_END);
        fseek(pFileB, 0, SEEK_END);

        uint32_t sizeA = ftell(pFileA);
        uint32_t sizeB = ftell(pFileB);
        if (sizeA != sizeB)
        {
            printf("CompareDumps: Size checks failed. A != B. SizeA: '%d', SizeB: '%d'\n", sizeA, sizeB);
            break;
        }

        fseek(pFileA, 0, SEEK_SET);
        fseek(pFileB, 0, SEEK_SET);

        // check 4 bytes at a time
        uint32_t fourBytesA = 0;
        uint32_t fourBytesB = 0;

        uint32_t i = 0;
        for (i = 0; i < sizeA; i++)
        {
            fread(&fourBytesA, sizeof(fourBytesA), 1, pFileA);
            fread(&fourBytesB, sizeof(fourBytesB), 1, pFileB);

            if (fourBytesA != fourBytesB)
            {
                printf("CompareDumps: Comparison failed! The 4 bytes at file offset '%ld' did not match. A: %x, B: %x\n", ftell(pFileA),  fourBytesA, fourBytesB);
                break;
            }
        }

        if (i == sizeA)
        {
            success = true;
        }
    }
    while(0);

    if (pFileA)
    {
        fclose(pFileA);
        pFileA = nullptr;
    }

    if (pFileB)
    {
        fclose(pFileB);
        pFileB = nullptr;
    }

    return success;
}
      
void SuperCopierSN::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO)
{
    char romTitle[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    romHeader.GetTitle(romTitle, sizeof(romTitle));

    char romFileNameVanilla[300] = { 0 };
    snprintf(romFileNameVanilla, sizeof(romFileNameVanilla) - 1, "./%s_ROMVER_%d.%s", romTitle, romHeader.GetRomVersion(), ROM_EXTENSION);

    char romFileNameA[300] = { 0 };
    snprintf(romFileNameA, sizeof(romFileNameA) - 1, "./%s_ROMVER_%d_A.%s", romTitle, romHeader.GetRomVersion(), ROM_EXTENSION);

    char romFileNameB[300] = { 0 };
    snprintf(romFileNameB, sizeof(romFileNameB) - 1, "./%s_ROMVER_%d_B.%s", romTitle, romHeader.GetRomVersion(), ROM_EXTENSION);

    FILE* pFileA = fopen(romFileNameA, "wb");
    if (!pFileA)
    {
        printf("Failed to open file '%s' for write!\n", romFileNameA);
        return;
    }

    FILE* pFileB = fopen(romFileNameB, "wb");
    if (!pFileB)
    {
        printf("Failed to open file '%s' for write!\n", romFileNameB);

        if (pFileA)
        {
            fclose(pFileA);
        }
        return;
    }

    // If no MMC (very, VERY common) we can go off of just the map mode.
    if (!romHeader.HasMMC())
    {
        switch (romHeader.GetMapMode())
        {
            case MapMode::MapMode_20:
            {
                // dump it twice.
                SNBoardNoMMCMode20::DumpROM(romHeader, snCartIO, pFileA);
                SNBoardNoMMCMode20::DumpROM(romHeader, snCartIO, pFileB);

                if (CompareDumps(romFileNameA, romFileNameB))
                {
                    // Now for final chef's kiss, validate the checksum.
                    uint16_t checkSum = CalcChecksum(romFileNameA);
                    if (checkSum == romHeader.GetChecksum() && (checkSum ^ 0xFFFF) == romHeader.GetChecksumComplement())
                    {
                        printf("ROM data is stable and internally consistent. No evidence of corruption or physical degradation was detected.\n");
                        printf("Your game is 100%% healthy, and your ROM image is too.\n");
                    }
                    else
                    {
                        // todo: see if the checksum I read at least matches the internal header's
                        //#define CHECKSUM_COMPLEMENT_ADDRESS 0x7FDC


                        printf("“ROM data is electrically stable. Header checksum does not match; this is commonly due to an incorrect checksum in the cartridge header and does not, by itself, indicate ROM failure.”\n");
                    }

                    rename(romFileNameA, romFileNameVanilla);
                    unlink(romFileNameB);
                }
                // dumps didn't match
                else
                {
                    // so does rom A's checksum match the header? if so, just take it.
                    printf("Rom dumps don't match. Checking to see if one of the two has a valid checksum.\n");
                    uint16_t checksumA = CalcChecksum(romFileNameA);
                    if (checksumA == romHeader.GetChecksum())
                    {
                        printf("ROM appears intact but exhibited transient read instability.”\n");
                        rename(romFileNameA, romFileNameVanilla);
                        unlink(romFileNameB);
                        break;
                    }
                    else
                    {
                        printf("ROM A's checksum doesn't match\n");
                    }
                    
                    // what about B's?
                    uint16_t checksumB = CalcChecksum(romFileNameB);
                    if (checksumB == romHeader.GetChecksum())
                    {
                        printf("ROM appears intact but exhibited transient read instability.”\n");
                        rename(romFileNameB, romFileNameVanilla);
                        unlink(romFileNameA);
                        break;
                    }
                    else
                    {
                        printf("ROM B's checksum doesn't match.\n");
                    }

                    // so we've got two garbage dumps. lets try in recovery mode.
                    printf("Neither ROM dump had a good checksum, and neither matched byte for byte. Doing a dump in recovery mode.\n");
                    FILE* pFile = fopen(romFileNameVanilla, "wb");
                    if (!pFile)
                    {
                        printf("Couldnt open file '%s' for dumping a recovery ROM\n", romFileNameVanilla);
                        break;
                    }
                    SNBoardNoMMCMode20::DumpROM_RecoveryMode(romHeader, snCartIO, pFile);
                    printf("Restoration dump complete. I got what I could, but this is in NO WAY a safe ROM\n");
                }

                //Solid	Dumps match, checksum matches
                //    Stable(Checksum mismatch)	Dumps match, checksum fails
                //    Marginal	Dumps differ, one checksum matches
                //    Failing	Dumps differ, neither checksum matches
                //    Degraded(Recovered)	Required multi - sampling

                printf("Final ROM should be: '%s'", romFileNameVanilla);
                break;
            }

            case MapMode::MapMode_21:
            {
                SNBoardNoMMCMode21::DumpROM(romHeader, snCartIO, pFileA);
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
    
    if (pFileA)
    {
        fclose(pFileA);
        pFileA = NULL;
    }

    if (pFileB)
    {
        fclose(pFileB);
        pFileB = NULL;
    }
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
