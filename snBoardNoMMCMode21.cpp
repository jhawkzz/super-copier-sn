#include "snBoardNoMMCMode21.h"

#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "snCartIO.h"
#include "snRomHeader.h"

void SNBoardNoMMCMode21::UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer)
{
    //todo
}

void SNBoardNoMMCMode21::DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, FILE* pOutFile)
{
    //todo: should this figure out the starting bank internally?

    if (!pOutFile)
    {
        printf("No file handle provided for downloading sram!\n");
        return;
    }

    SetCartToIdleState(snCartIO);

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
            EnableROM_DisableSRAM(snCartIO); //Tuen OFF SRAM

            // set the address for the byte we want to read
            snCartIO.mAddressBus.SetAddress(address);
            WAIT();

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            DisableROM_EnableSRAM(snCartIO); // Turn ON sram

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
}

void SNBoardNoMMCMode21::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile, bool firstBankOnly)
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
            DisableROM_EnableSRAM(snCartIO); //Disable the ROM since we'll be reading from it

            // Set the address to read a byte from
            snCartIO.mAddressBus.SetAddress(address);

            // Set the dataBus to HiZ
            snCartIO.mDataBus.HiZ();

            // Now we're ready, so enable the cartEnable
            EnableROM_DisableSRAM(snCartIO);

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

void SNBoardNoMMCMode21::SetCartToIdleState(SNCartIO& snCartIO)
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

void SNBoardNoMMCMode21::EnableROM_DisableSRAM(SNCartIO& snCartIO)
{
    // For memory map mode 21, enabling CartEnable (which pulls it low) will enable /CE on ROM, but disable /CE on SRAM
    snCartIO.mCartEnablePin.Enable();
    WAIT();
}

void SNBoardNoMMCMode21::DisableROM_EnableSRAM(SNCartIO& snCartIO)
{
    // For memory map mode 21, disabling CartEnable (which pulls it high) will disable /CE on ROM, but enable /CE on SRAM
    snCartIO.mCartEnablePin.Disable();
    WAIT();
}