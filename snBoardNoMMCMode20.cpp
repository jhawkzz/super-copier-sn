#include "snBoardNoMMCMode20.h"

#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "snCartIO.h"
#include "snRomHeader.h"

void SNBoardNoMMCMode20::UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer)
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

            // Disable the ROM/SRAM chip output
            DisableROMAndSRAMChips(snCartIO);

            // Put the dataBus into HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            // Set the address where we want to write
            snCartIO.mAddressBus.SetAddress(address);
            WAIT();

            // NOW we're ready to write the byte, so:
            // Enable the ROM/SRAM chips
            EnableROMAndSRAMChips(snCartIO);

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

            // Disable the ROM/SRAM chip output
            DisableROMAndSRAMChips(snCartIO);
        }
    }

    SetCartToIdleState(snCartIO);
    
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

void SNBoardNoMMCMode20::DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
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
        DisableROMAndSRAMChips(snCartIO);

        // set the address for the byte we want to read
        snCartIO.mAddressBus.SetAddress(address);
        WAIT();

        // Set the dataBus to HiZ
        snCartIO.mDataBus.HiZ();
        WAIT();

        EnableROMAndSRAMChips(snCartIO);

        uint8_t value = snCartIO.mDataBus.Read();
        WAIT();

        fwrite(&value, 1, 1, pOutFile);

        snCartIO.mDataBus.HiZ();
        WAIT();
    }

    fflush(pOutFile);

    SetCartToIdleState(snCartIO);
}

void SNBoardNoMMCMode20::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly)
{
    if (!pOutFile)
    {
        printf("No file handle provided for dumping!\n");
        return;
    }

    SetCartToIdleState(snCartIO);

    uint32_t numBanks = firstBankOnly ? 1 : romHeader.GetNumBanks();

    // LoROM (Memory Map 20) games are in banks $00 thru $7D
    for (uint32_t c = MAP_MODE_20_ROM_START_BANK; c < MAP_MODE_20_ROM_START_BANK + numBanks; c++)
    {
        printf("Dumping Bank: $%x\n", c);

        for (uint32_t i = 0; i < MAP_MODE_20_BANK_SIZE; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_20_ROM_BANK_BASE_ADDRESS);

            // Disable the chips
            DisableROMAndSRAMChips(snCartIO);

            // Set the address to read a byte from
            snCartIO.mAddressBus.SetAddress(address);

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();

            // Now we're read, so enable the cartEnable
            EnableROMAndSRAMChips(snCartIO);

            // Grab the byte off the lines
            uint8_t value = snCartIO.mDataBus.Read();

            fwrite(&value, 1, 1, pOutFile);

            snCartIO.mDataBus.HiZ();
        }

        fflush(pOutFile);
    }

    SetCartToIdleState(snCartIO);
}

void SNBoardNoMMCMode20::SetCartToIdleState(SNCartIO& snCartIO)
{
    // Disable writeEnable to prevent SRAM writes
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

void SNBoardNoMMCMode20::EnableROMAndSRAMChips(SNCartIO& snCartIO)
{
    // For memory map mode 20, enabling CartEnable (which pulls it low) will activate /CE on both RAM and ROM
    snCartIO.mCartEnablePin.Enable();
    WAIT();
}

void SNBoardNoMMCMode20::DisableROMAndSRAMChips(SNCartIO& snCartIO)
{
    // For memory map mode 20, disabling CartEnable (which pulls it high) will disable /CE on both RAM and ROM
    snCartIO.mCartEnablePin.Disable();
    WAIT();
}