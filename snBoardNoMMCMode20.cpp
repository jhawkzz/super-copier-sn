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

    // LoROM (Memory Map 20) sram is stored at bank 0x70
    for (uint32_t c = MAP_MODE_20_SRAM_START_BANK; c < MAP_MODE_20_SRAM_START_BANK + 1; c++)
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
            //printf("%x: %x\n", address, pSRAMBuffer[i]);
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

void SNBoardNoMMCMode20::DumpROM_RecoveryMode(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("No file handle provided for dumping!\n");
        return;
    }

    // This will do a 5x sample of each address and take the majority byte returned.
    // In the event of a tie, it takes the lowest byte (arbitrary).
    // We do this to be as "true" as possible, in the sense that its a value the ROM really did report,
    // whether accurate and intended by the developer or not.

    // working byte recoverer
    uint32_t numChecks = 5;
    uint8_t frequencyBuffer[256] = { 0 };

    for (uint32_t c = MAP_MODE_20_ROM_START_BANK; c < MAP_MODE_20_ROM_START_BANK + romHeader.GetNumBanks(); c++)
    {
        printf("Dumping Bank (in Recovery Mode): $%x\n", c);

        for (uint32_t i = 0; i < MAP_MODE_20_BANK_SIZE; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_20_ROM_BANK_BASE_ADDRESS);

            // reset the buffer.
            for (uint32_t j = 0; j < sizeof(frequencyBuffer); j++)
            {
                frequencyBuffer[j] = 0;
            }

            // read each byte NUMCHECKS times and take the most hit byte.
            //printf("Sampling address: 0x%x %d times.\n", address, numChecks);
            for (uint32_t d = 0; d < numChecks; d++)
            {
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
                
                frequencyBuffer[value] += 1;

                snCartIO.mDataBus.HiZ();
            }

            // now take the byte with the best hit rate
            uint8_t bestByteIndex = 0;
            for (uint32_t d = 0; d < 256; d++)
            {
                if (frequencyBuffer[d] > frequencyBuffer[bestByteIndex])
                {
                    bestByteIndex = d;
                }
            }

            fwrite(&bestByteIndex, 1, 1, pOutFile);
        }

        fflush(pOutFile);
    }

    SetCartToIdleState(snCartIO);
}

void SNBoardNoMMCMode20::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("No file handle provided for dumping!\n");
        return;
    }

    SetCartToIdleState(snCartIO);

    // LoROM (Memory Map 20) games are in banks $00 thru $7D
    for (uint32_t c = MAP_MODE_20_ROM_START_BANK; c < MAP_MODE_20_ROM_START_BANK + romHeader.GetNumBanks(); c++)
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
