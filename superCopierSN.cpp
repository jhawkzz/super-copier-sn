
#include "superCopierSN.h"

#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"

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

void SuperCopierSN::UploadToSRAM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer)
{
    if (!pSRAMBuffer)
    {
        printf("No buffer provided for uploading to SRAM!\n");
        return;
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    uint32_t numBanks = 1;

    // LoROM (Memory Map 20) sram is stored at bank 0x70
    for (uint32_t c = MAP_MODE_20_SRAM_START_BANK; c < MAP_MODE_20_SRAM_START_BANK + numBanks; c++)
    {
        printf("Uploading Bank: $%x\n", c);

        for (uint32_t i = 0; i < romHeader.GetRAMSizeBytes(); i++)
        {
            uint32_t address = (c << 16) | i;

            // To write a byte, we need to:
            // Disable writeEnable
            snCartIO.mWriteEnablePin.Disable();
            WAIT();

            snCartIO.mReadEnablePin.Enable();
            WAIT();

            // Disable cartEnable (disable the ROM/SRAM chip output)
            snCartIO.mCartEnablePin.Disable();
            WAIT();

            // Put the dataBus into HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            // Set the address where we want to write
            snCartIO.mAddressBus.SetAddress(address);
            WAIT();

            // NOW we're ready to write the byte, so:
            // Enable cartEnable (enable the ROM/SRAM chips)
            snCartIO.mCartEnablePin.Enable();
            WAIT();

            // Enable writeEnable (flips off OutputEnable on SRAM)
            snCartIO.mWriteEnablePin.Enable();
            WAIT();

            snCartIO.mReadEnablePin.Disable();
            WAIT();

            // Put the byte on the dataBus
            printf("%x: %x\n", address, pSRAMBuffer[i]);
            snCartIO.mDataBus.Write(pSRAMBuffer[i]);
            WAIT();

            // Disable writeEnable so SRAM can latch the bytes
            snCartIO.mWriteEnablePin.Disable();
            WAIT();

            snCartIO.mReadEnablePin.Enable();
            WAIT();

            // Put the dataBus into HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            // Disable cartEnable (disable the ROM/SRAM chips)
            snCartIO.mCartEnablePin.Disable();
            WAIT();
        }
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    /*char sramFileName[300] = { 0 };
    snprintf(sramFileName, sizeof(sramFileName) - 1, "./%s.%s", pRomName, SRAM_EXTENSION);

    FILE* pFile = fopen(sramFileName, "rb");
    if (pFile)
    {
        memset(mSRAMBuffer, 0, sizeof(mSRAMBuffer));

        fread(mSRAMBuffer, sramSize, 1, pFile);

        fclose(pFile);
        pFile = NULL;
    }
    else
    {
        printf("UploadToSRAM: Failed to open sram file '%s.%s'.\n", pRomName, SRAM_EXTENSION);
        return;
    }

    SetCartToIdleState();
    WAIT();

    for (uint32_t i = 0; i < sramSize; i++)
    {
        uint32_t address = i + SRAM_BANK_START_ADDRESS;

        // To write a byte, we need to:
        // Disable writeEnable
        mWriteEnablePin.Disable();
        WAIT();

        mReadEnablePin.Enable();
        WAIT();

        // Disable cartEnable (disable the ROM/SRAM chips)
        mCartEnablePin.Disable();
        WAIT();

        // Put the dataBus into HiZ
        mDataBus.HiZ();
        WAIT();

        // Set the address where we want to write
        mAddressBus.SetAddress(address);
        WAIT();

        // NOW we're ready to write the byte, so:
        // Enable cartEnable (enable the ROM/SRAM chips)
        mCartEnablePin.Enable();
        WAIT();

        // Enable writeEnable (flips off OutputEnable on SRAM)
        mWriteEnablePin.Enable();
        WAIT();

        mReadEnablePin.Disable();
        WAIT();

        // Put the byte on the dataBus
        printf("%x: %x\n", address, mSRAMBuffer[i]);
        mDataBus.Write(mSRAMBuffer[i]);
        WAIT();

        // Disable writeEnable so SRAM can latch the bytes
        mWriteEnablePin.Disable();
        WAIT();

        mReadEnablePin.Enable();
        WAIT();

        // Put the dataBus into HiZ
        mDataBus.HiZ();
        WAIT();

        // Disable cartEnable (disable the ROM/SRAM chips)
        mCartEnablePin.Disable();
        WAIT();
    }

    // Now we're done, so put the cart back into a safe, idle state.
    SetCartToIdleState();
    
    printf("UploadToSRAM: Uploaded contents of file '%s' to cart SRAM.\n", sramFileName);*/
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

    switch (romHeader.GetCoProcessor())
    {
        case CoProcessor::None:
        case CoProcessor::DSP:
        {
            switch (romHeader.GetMapMode())
            {
                case MapMode::MapMode_20:
                {
                    if (romHeader.GetRAMSizeBytes() <= MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE)
                    {
                        printf("Uploading SRAM with No CoProcessor or DSP, MapMode 20 (LoROM)\n");
                        UploadToSRAM_MapMode20(romHeader, snCartIO, mSRAMBuffer);
                    }
                    else
                    {
                        printf("Detected a game with No CoProcessor or DSP, MapMode 20 (LoROM) and a ram size of '%d'. The largest known LoROM game used an ram size of '%d' No game ever shipped with that config. Corrupt header?\n", romHeader.GetRAMSizeBytes(), MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE);
                    }
                    break;
                }

                case MapMode::MapMode_21:
                {
                    if (romHeader.GetRAMSizeBytes() <= MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE)
                    {
                        printf("Uploading SRAM with No CoProcessor or DSP, MapMode 21 (HiROM)\n");
                        UploadToSRAM_MapMode20(romHeader, snCartIO, mSRAMBuffer);
                    }
                    else
                    {
                        printf("Detected a game with No CoProcessor or DSP, MapMode 21 (HiROM) and a ram size of '%d'. The largest known LoROM game used an ram size of '%d' No game ever shipped with that config. Corrupt header?\n", romHeader.GetRAMSizeBytes(), MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE);
                    }
                    break;
                }

                case MapMode::MapMode_25:
                {
                    printf("MapMode 25 coming soon. Cannot upload!");
                    break;
                }

                default:
                {
                    printf("Found no CoProcessor and MapMode: '%d' which is not supported. Header might be corrupt. Cannot upload!\n", (int32_t)romHeader.GetMapMode());
                    break;
                }
            }
            break;
        }

        case CoProcessor::SuperFX:
        {
            printf("Detected SuperFX coprocessor. Cannot upload yet!\n");
            break;
        }

        default:
        {
            printf("Detected unsupported (as of yet) coprocessor. Cannot upload!\n");
            break;
        }
    }

    printf("UploadToSRAM: Uploaded contents of file '%s' to cart SRAM\n", sramFileName);
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

    switch (romHeader.GetCoProcessor())
    {
        case CoProcessor::None:
        case CoProcessor::DSP:
        {
            switch (romHeader.GetMapMode())
            {
                case MapMode::MapMode_20:
                {
                    if (romHeader.GetRAMSizeBytes() <= MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE)
                    {
                        printf("Downloading SRAM with No CoProcessor or DSP, MapMode 20 (LoROM)\n");
                        DownloadFromSRAM_MapMode20(romHeader, snCartIO, pFile);
                    }
                    else
                    {
                        printf("Detected a game with No CoProcessor or DSP, MapMode 20 (LoROM) and a ram size of '%d'. The largest known LoROM game used an ram size of '%d' No game ever shipped with that config. Corrupt header?\n", romHeader.GetRAMSizeBytes(), MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE);
                    }
                    break;
                }

                case MapMode::MapMode_21:
                {
                    if (romHeader.GetRAMSizeBytes() <= MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE)
                    {
                        printf("Downloading SRAM with No CoProcessor or DSP, MapMode 21 (HiROM)\n");
                        //todo: need to handle game specific diffs for games using the uncommon 0x10 starting bank.
                        DownloadFromSRAM_MapMode21(romHeader, snCartIO, MAP_MODE_21_SRAM_START_BANK_COMMON_START, pFile);
                    }
                    else
                    {
                        printf("Detected a game with No CoProcessor or DSP, MapMode 21 (HiROM) and a ram size of '%d'. The largest known HiROM game used an ram size of '%d' No game ever shipped with that config. Corrupt header?\n", romHeader.GetRAMSizeBytes(), MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE);
                    }
                    break;
                }

                case MapMode::MapMode_25:
                {
                    printf("MapMode 25 coming soon. Cannot Download!");
                    break;
                }

                default:
                {
                    printf("Found no CoProcessor or DSP and MapMode: '%d' which is not supported. Header might be corrupt. Cannot Download!\n", (int32_t)romHeader.GetMapMode());
                    break;
                }
            }
            break;
        }

        case CoProcessor::SuperFX:
        {
            printf("Detected SuperFX coprocessor. Cannot download yet!\n");
            break;
        }

        default:
        {
            printf("Detected unsupported (as of yet) coprocessor. Cannot download!\n");
            break;
        }
    }

    printf("DownloadFromSRAM: Downloaded contents of SRAM from cart to file '%s'\n", sramFileName);

    fclose(pFile);
    pFile = NULL;
}

void SuperCopierSN::DownloadFromSRAM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("No file handle provided for downloading sram!\n");
        return;
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    // LoROM (Memory Map 20) sram is stored in bank 0x70
    printf("Dumping Bank: $%x\n", MAP_MODE_20_SRAM_START_BANK);

    for (uint32_t i = 0; i < romHeader.GetRAMSizeBytes(); i++)
    {
        uint32_t address = (MAP_MODE_20_SRAM_START_BANK << 16) | i;

        // To read a byte we need to:
        // disable cart output (disable rom/sram chips. This is only true on LoROM)
        snCartIO.mCartEnablePin.Disable();
        WAIT();

        // set the address for the byte we want to read
        snCartIO.mAddressBus.SetAddress(address);
        WAIT();

        // Set the dataBus to HiZ
        snCartIO.mDataBus.HiZ();
        WAIT();

        // enable the cart output (enable the rom/sram chips)
        snCartIO.mCartEnablePin.Enable();
        WAIT();

        uint8_t value = snCartIO.mDataBus.Read();
        WAIT();

        fwrite(&value, 1, 1, pOutFile);
            
        snCartIO.mDataBus.HiZ();
        WAIT();
    }

    fflush(pOutFile);

    SetCartToIdleState(snCartIO);
    WAIT();
}

void SuperCopierSN::DownloadFromSRAM_MapMode21(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("No file handle provided for downloading sram!\n");
        return;
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    // HiROM (Memory Map 21) sram is stored in bank 0x20
    uint32_t numBanks = romHeader.GetRAMSizeBytes() / MAP_MODE_21_SRAM_BANK_SIZE;
    if (numBanks == 0) numBanks = 1;

    uint32_t bytesPerBank = romHeader.GetRAMSizeBytes() < MAP_MODE_21_SRAM_BANK_SIZE ? romHeader.GetRAMSizeBytes() : MAP_MODE_21_SRAM_BANK_SIZE;

    printf("Downloading from SRAM: %d bytes per bank. %d banks. %x thru %x\n", bytesPerBank, numBanks, startingBank, startingBank + numBanks);

    for (uint32_t c = startingBank; c < startingBank + numBanks; c++)
    {
        printf("Dumping Bank: $%x\n", c);

        for (uint32_t i = 0; i < bytesPerBank; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_21_SRAM_BANK_BASE_ADDRESS);
            
            // For HiROM carts using MAD-1, they enable SRAM when /CART is pulled _HIGH_.
            // This is opposite LoROM where /CART should be LOW for ROM or SRAM reads.
            // 
            // todo: write wrapper functions to make this part more readable
            // To read a byte we need to:
            
            snCartIO.mCartEnablePin.Enable(); //Turn OFF Sram
            WAIT();

            // set the address for the byte we want to read
            snCartIO.mAddressBus.SetAddress(address);
            WAIT();

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            snCartIO.mCartEnablePin.Disable(); //Turn ON sram
            WAIT();

            uint8_t value = snCartIO.mDataBus.Read();
            WAIT();

            //printf("Address: %x, Value: %d\n", address, value);
            fwrite(&value, 1, 1, pOutFile);

            snCartIO.mDataBus.HiZ();
            WAIT();
        }
    }

    fflush(pOutFile);

    SetCartToIdleState(snCartIO);
    WAIT();
}
                    
void SuperCopierSN::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, bool firstBankOnly)
{
    char romTitle[GAME_TITLE_LEN_BYTES + 1] = { 0 };
    romHeader.GetTitle(romTitle, sizeof(romTitle));

    char romFileName[300] = { 0 };
    snprintf(romFileName, sizeof(romFileName) - 1, "./%s_ROMVER_%d%s.%s", romTitle, romHeader.GetRomVersion(), firstBankOnly ? "_FirstBank" : "", ROM_EXTENSION);

    FILE* pFile = fopen(romFileName, "wb");
    if (!pFile)
    {
        printf("Failed to open file '%s' for write!\n", romFileName);
        return;
    }

    switch (romHeader.GetCoProcessor())
    {
        case CoProcessor::None:
        case CoProcessor::DSP:
        {
            switch (romHeader.GetMapMode())
            {
                case MapMode::MapMode_20:
                {
                    printf("Dumping ROM with No CoProcessor or DSP, MapMode 20 (LoROM)\n");
                    DumpROM_MapMode20(romHeader, snCartIO, pFile, firstBankOnly);
                    break;
                }

                case MapMode::MapMode_21:
                {
                    printf("Dumping ROM with No CoProcessor or DSP, MapMode 21 (HiROM)\n");
                    DumpROM_MapMode21(romHeader, snCartIO, pFile, firstBankOnly);
                    break;
                }
                case MapMode::MapMode_25:
                {
                    printf("MapMode 25 coming soon. Cannot dump!");
                    break;
                }

                default:
                {
                    printf("Found no CoProcessor or DSP and MapMode: '%d' which is not supported. Header might be corrupt. Cannot dump!\n", (int32_t)romHeader.GetMapMode());
                    break;
                }
            }
            break;
        }
        
        case CoProcessor::SuperFX:
        {
            printf("Detected SuperFX coprocessor. Cannot dump yet!\n");
            break;
        }

        default:
        {
            printf("Detected unsupported (as of yet) coprocessor. Cannot dump!\n");
            break;
        }
    }

    printf("DumpROM: Wrote contents to file '%s'\n", romFileName);
    
    fclose(pFile);
    pFile = NULL;
}

void SuperCopierSN::DumpROM_MapMode20(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly)
{
    if (!pOutFile)
    {
        printf("No file handle provided for dumping!\n");
        return;
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    uint32_t numBanks = firstBankOnly ? 1 : romHeader.GetNumBanks();

    // LoROM (Memory Map 20) games are in banks $00 thru $7D
    for (uint32_t c = MAP_MODE_20_ROM_START_BANK; c < MAP_MODE_20_ROM_START_BANK  + numBanks; c++)
    {
        printf("Dumping Bank: $%x\n", c);

        for (uint32_t i = 0; i < MAP_MODE_20_BANK_SIZE; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_20_ROM_BANK_BASE_ADDRESS);

            // Disable the cartEnable for map mode 20, this does disable both ROM & SRAM
            snCartIO.mCartEnablePin.Disable();

            // Set the address to read a byte from
            snCartIO.mAddressBus.SetAddress(address);

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();

            // Now we're read, so enable the cartEnable
            snCartIO.mCartEnablePin.Enable();
            WAIT();

            // Grab the byte off the lines
            uint8_t value = snCartIO.mDataBus.Read();

            fwrite(&value, 1, 1, pOutFile);

            snCartIO.mDataBus.HiZ();
        }

        fflush(pOutFile);
    }

    SetCartToIdleState(snCartIO);
    WAIT();
}

void SuperCopierSN::DumpROM_MapMode21(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly)
{
    if (!pOutFile)
    {
        printf("No file handle provided for dumping!\n");
        return;
    }

    SetCartToIdleState(snCartIO);
    WAIT();

    uint32_t numBanks = firstBankOnly ? 1 : romHeader.GetNumBanks();

    // HiROM (Memory Map 21) games are in banks $C0 thru $FF
    for (uint32_t c = MAP_MODE_21_ROM_START_BANK; c < MAP_MODE_21_ROM_START_BANK + numBanks; c++)
    {
        printf("Dumping Bank: $%x\n", c);

        for (uint32_t i = 0; i < MAP_MODE_21_BANK_SIZE; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_21_ROM_BANK_BASE_ADDRESS);

            // Disable the cartEnable (disables ROM. For MAD-1 games w/ SRAM, does enable SRAM)
            snCartIO.mCartEnablePin.Disable();

            // Set the address to read a byte from
            snCartIO.mAddressBus.SetAddress(address);

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();

            // Now we're ready, so enable the cartEnable
            snCartIO.mCartEnablePin.Enable();
            WAIT();

            // Grab the byte off the lines
            uint8_t value = snCartIO.mDataBus.Read();

            fwrite(&value, 1, 1, pOutFile);

            snCartIO.mDataBus.HiZ();
        }

        fflush(pOutFile);
    }

    SetCartToIdleState(snCartIO);
    WAIT();
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
    // Holy crap!! 12-7: After fixing wiring last night and cleaning up my LoRom vs HiRom dumping,
    // i have had 5 clean dumps in a row. 2 SMW, 1 Zombies, 1 Ken Griffey, and DONKEY KONG COUNTRY.
    // DKC is significant because its HiROM, but here's the greatest part! It didnt binary compare
    // with my known copy, but the checksum was ok. Turns out i have Rev 1 and the cleanRom is rev 0!!!


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

    // todo: allow re-inserting. for now, jsut abort.
    //if (!mROMHeader.IsValid())
    //{
    //	printf("ERROR: Game cannot be read. Aborting...\n");
    //	
    //	// Configure lines for removing cartridge
    //	SetCartToIdleState();
    //	WAIT();

    //	mResetPin.Enable();
    //	WAIT();

    //	printf("REMOVE CARTRIDGE and press any key.\n");
    //	while (!getchar())
    //	{
    //	}

    //	return;
    //}
    //

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
        printf("Dump [F]irst Bank Only\n");
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
                DumpROM(mROMHeader, mSNCartIO, false);
                break;
            }

            case 'f':
            {
                DumpROM(mROMHeader, mSNCartIO, true);
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
