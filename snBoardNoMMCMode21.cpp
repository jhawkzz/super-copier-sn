#include "snBoardNoMMCMode21.h"

#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "snCartIO.h"
#include "snRomHeader.h"

void SNBoardNoMMCMode21::UploadToSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, uint8_t* pSRAMBuffer)
{
    if (!pSRAMBuffer)
    {
        printf("SNBoardNoMMCMode21::UploadToSRAM - No buffer provided. Can't upload.\n");
        return;
    }

    if (!IsSRAMValid(romHeader))
    {
        return;
    }

    //todo: check the board type so we send the right starting bank
    UploadToSRAM_Internal(romHeader, snCartIO, MAP_MODE_21_SRAM_START_BANK_COMMON_BOARD, pSRAMBuffer);
}

void SNBoardNoMMCMode21::UploadToSRAM_Internal(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, uint8_t* pSRAMBuffer)
{
    SetCartToIdleState(snCartIO);

    uint32_t numBanks = romHeader.GetRAMSizeBytes() / MAP_MODE_21_SRAM_BANK_SIZE;
    if (numBanks == 0) numBanks = 1;

    uint32_t bytesPerBank = romHeader.GetRAMSizeBytes() < MAP_MODE_21_SRAM_BANK_SIZE ? romHeader.GetRAMSizeBytes() : MAP_MODE_21_SRAM_BANK_SIZE;

    printf("UploadToSRAM: %d bytes per bank. %d banks. $%x thru $%x\n", bytesPerBank, numBanks, startingBank, startingBank + numBanks);

    for (uint32_t c = startingBank; c < startingBank + numBanks; c++)
    {
        printf("Uploading Bank: $%x\n", c);

        for (uint32_t i = 0; i < bytesPerBank; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_21_SRAM_BANK_BASE_ADDRESS);

            // To write a byte, we need to:
            // Disable writeEnable
            snCartIO.mWriteEnablePin.Disable();
            WAIT();

            snCartIO.mReadEnablePin.Enable();
            WAIT();

            // Disable the SRAM chip output (its fine that ROM is enabled)
            EnableROM_DisableSRAM(snCartIO);

            // Put the dataBus into HiZ
            snCartIO.mDataBus.HiZ();
            WAIT();

            // Set the address where we want to write
            snCartIO.mAddressBus.SetAddress(address);
            WAIT();

            // NOW we're ready to write the byte, so:
            // Enable the SRAM
            DisableROM_EnableSRAM(snCartIO);

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

            // Disable the SRAM chip output (its fine that ROM is enabled)
            EnableROM_DisableSRAM(snCartIO);
        }
    }

    SetCartToIdleState(snCartIO);
}

void SNBoardNoMMCMode21::DownloadFromSRAM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("SNBoardNoMMCMode21::DownloadFromSRAM - No file handle provided. Can't download.\n");
        return;
    }

    if (!IsSRAMValid(romHeader))
    {
        return;
    }

    // TODO: Check the game name to see if it uses a 0x10 board.
    DownloadFromSRAM_Internal(romHeader, snCartIO, MAP_MODE_21_SRAM_START_BANK_COMMON_BOARD, pOutFile);
}

void SNBoardNoMMCMode21::DownloadFromSRAM_Internal(const ROMHeader& romHeader, SNCartIO& snCartIO, uint32_t startingBank, FILE* pOutFile)
{
    SetCartToIdleState(snCartIO);

    uint32_t numBanks = romHeader.GetRAMSizeBytes() / MAP_MODE_21_SRAM_BANK_SIZE;
    if (numBanks == 0) numBanks = 1;

    uint32_t bytesPerBank = romHeader.GetRAMSizeBytes() < MAP_MODE_21_SRAM_BANK_SIZE ? romHeader.GetRAMSizeBytes() : MAP_MODE_21_SRAM_BANK_SIZE;

    printf("DownloadFromSRAM: %d bytes per bank. %d banks. $%x thru $%x\n", bytesPerBank, numBanks, startingBank, startingBank + numBanks);

    for (uint32_t c = startingBank; c < startingBank + numBanks; c++)
    {
        printf("Downloading Bank: $%x\n", c);

        for (uint32_t i = 0; i < bytesPerBank; i++)
        {
            uint32_t address = (c << 16) | (i + MAP_MODE_21_SRAM_BANK_BASE_ADDRESS);

            // For HiROM carts using MAD-1, they enable SRAM when /CART is pulled _HIGH_.
            // This is opposite LoROM where /CART should be LOW for ROM or SRAM reads.
            // 
            // To read a byte we need to:
            EnableROM_DisableSRAM(snCartIO); //Turn OFF SRAM

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

void SNBoardNoMMCMode21::DumpROM(const ROMHeader& romHeader, SNCartIO& snCartIO, FILE* pOutFile)
{
    if (!pOutFile)
    {
        printf("SNBoardNoMMCMode21::DumpROM - No file handle provided. Can't download.\n");
        return;
    }

    if (!IsROMValid(romHeader))
    {
        return;
    }

    SetCartToIdleState(snCartIO);

    // HiROM (Memory Map 21) games are in banks $C0 thru $FF
    for (uint32_t c = MAP_MODE_21_ROM_START_BANK; c < MAP_MODE_21_ROM_START_BANK + romHeader.GetNumBanks(); c++)
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

bool SNBoardNoMMCMode21::IsSRAMValid(const ROMHeader& romHeader)
{
    // verify RAM size is not > 32KiB. On paper more could be supported, but nothing shipped, and it complicates read/write.
    if (romHeader.GetRAMSizeBytes() > MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE)
    {
        printf("%s - Max supported SRAM size is '%d'. Game reports size of '%d'. Can't download. Corrupt header?\n", __FUNCTION__, MAP_MODE_20_21_SRAM_MAX_SHIPPED_SIZE, romHeader.GetRAMSizeBytes());
        return false;
    }

    // make sure the game isn't using an MMC
    if (romHeader.HasMMC())
    {
        printf("%s - Game reports using an MMC (CoProcessor: '%d'). Can't download. Corrupt header, or wrong function called?\n", __FUNCTION__, (uint32_t)romHeader.GetCoProcessor());
        return false;
    }

    // Sanity check that it's map mode 21.
    if (romHeader.GetMapMode() != MapMode::MapMode_21)
    {
        printf("%s - Game reports using MapMode '%d'. This function is for Map Mode 21. Can't download.\n", __FUNCTION__, (uint32_t)romHeader.GetMapMode());
        return false;
    }

    return true;
}

bool SNBoardNoMMCMode21::IsROMValid(const ROMHeader& romHeader)
{    
    // make sure the game isn't using an MMC
    if (romHeader.HasMMC())
    {
        printf("%s - Game reports using an MMC (CoProcessor: '%d'). Can't download. Corrupt header, or wrong function called?\n", __FUNCTION__, (uint32_t)romHeader.GetCoProcessor());
        return false;
    }

    // Sanity check that it's map mode 21.
    if (romHeader.GetMapMode() != MapMode::MapMode_21)
    {
        printf("%s - Game reports using MapMode '%d'. This function is for Map Mode 21. Can't download.\n", __FUNCTION__, (uint32_t)romHeader.GetMapMode());
        return false;
    }

    return true;
}
