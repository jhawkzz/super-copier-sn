
#include "superCopierSN.h"

#include <cstring>
#include <stdio.h>
#include "defines.h"
#include <unistd.h>

SuperCopierSN& SuperCopierSN::Get()
{
	static SuperCopierSN Instance;
	return Instance;
}

void SuperCopierSN::Create(gpiod_chip* pChip)
{
	// Address
	uint8_t addressLineIndices[] = { 2,3,4,17,27,22,10,9 };
	uint8_t latch1 = 5;
	uint8_t latch2 = 16;
	mAddressBus.Create(pChip, addressLineIndices, latch1, latch2);

	// Data
	uint8_t dataLineIndices[] = { 14,15,18,23,24,25,8,7 };
	mDataBus.Create(pChip, dataLineIndices);

	// Misc Pins
	mWriteEnablePin.Create(pChip, 12);
	mReadEnablePin.Create(pChip, 6);
	mResetPin.Create(pChip, 21);
	mCartEnablePin.Create(pChip, 20);
}

SuperCopierSN::~SuperCopierSN()
{
	Release();
}

void SuperCopierSN::Release()
{
	mCartEnablePin.Release();
	mResetPin.Release();
	mWriteEnablePin.Release();
	mReadEnablePin.Release();

	mDataBus.Release();
	mAddressBus.Release();
}

void SuperCopierSN::UploadToSRAM(const char* pRomName, uint32_t sramSize)
{
	char sramFileName[300] = { 0 };
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
	
	printf("UploadToSRAM: Uploaded contents of file '%s' to cart SRAM.\n", sramFileName);
}

void SuperCopierSN::DownloadFromSRAM(const char* pRomName, uint32_t sramSize)
{
	char sramFileName[300] = { 0 };
	snprintf(sramFileName, sizeof(sramFileName) - 1, "./%s.%s", pRomName, SRAM_EXTENSION);

	FILE* pFile = fopen(sramFileName, "wb");
	if (!pFile)
	{
		printf("DownloadFromSRAM: Failed to open file '%s' for write!\n", sramFileName);
		return;
	}

	memset(mSRAMBuffer, 0, sizeof(mSRAMBuffer));

	SetCartToIdleState();
	WAIT();

	for (uint32_t i = 0; i < sramSize; i++)
	{
		uint32_t address = i + SRAM_BANK_START_ADDRESS;

		// To read a byte we need to:
		// disable cart output (disable rom/sram chips)
		mCartEnablePin.Disable();
		WAIT();

		// set the address for the byte we want to read
		mAddressBus.SetAddress(address);
		WAIT();

		// Set the dataBus to HiZ
		mDataBus.HiZ();
		WAIT();

		// enable the cart output (enable the rom/sram chips)
		mCartEnablePin.Enable();
		WAIT();

		uint8_t value = mDataBus.Read();
		WAIT();
		mSRAMBuffer[i] = value;
		printf("%x: %x\n", address, value);

		mDataBus.HiZ();
		WAIT();
	}

	SetCartToIdleState();
	WAIT();

	// Write the buffer
	fwrite(mSRAMBuffer, sramSize, 1, pFile);
	printf("DownloadFromSRAM: Downloaded contents of cart SRAM to file '%s'\n", sramFileName);

	fclose(pFile);
	pFile = NULL;
}

void SuperCopierSN::DumpROM(const char *pRomName, uint32_t numBanks, uint32_t bankSize)
{
	char romFileName[300] = { 0 };
	snprintf(romFileName, sizeof(romFileName) - 1, "./%s.%s", pRomName, ROM_EXTENSION);

	FILE* pFile = fopen(romFileName, "wb");
	if (!pFile)
	{
		printf("Failed to open file '%s' for write!\n", romFileName);
		return;
	}

	memset(mROMBuffer, 0, sizeof(mROMBuffer));

	//todo: need to deal with hiRom, and things like bank skipping (sram games dont have rom data at banks 0x700000-0x7DFFFF)
	//todo: handle things like roms that don't have even bank usage (we gotta add the two banks and round up to nearest pow2...or something)
	//todo: might be better to stream out the data to disk, but eh

	SetCartToIdleState();
	WAIT();

	for (uint32_t c = 0; c < numBanks; c++)
	{
		printf("Dumping Bank: $%x\n", c);

		for (uint32_t i = 0; i < bankSize; i++)
		{
			uint32_t address = (c << 16) | i;

			// Disable the cartEnable (disable the rom/sram chips)
			mCartEnablePin.Disable();
			WAIT();

			// Set the address to read a byte from
			mAddressBus.SetAddress(address);
			WAIT();

			// Set the dataBus to HiZ
			mDataBus.HiZ();
			WAIT();

			// Now we're read, so enable the cartEnable
			mCartEnablePin.Enable();
			WAIT();

			// Grab the byte off the lines
			uint8_t value = mDataBus.Read();
			WAIT();
			mROMBuffer[(c * bankSize) + i] = value;
			//printf("%x: %x\n", address, value);

			mDataBus.HiZ();
			WAIT();
		}
	}

	SetCartToIdleState();
	WAIT();

	printf("DumpROM: Wrote contents to file '%s'\n", romFileName);
	fwrite(mROMBuffer, numBanks * bankSize, 1, pFile);

	fclose(pFile);
	pFile = NULL;	
}

void SuperCopierSN::SetCartToIdleState()
{
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

	// put the address into HiZ
	mAddressBus.HiZ();
	WAIT();
}

void SuperCopierSN::PrintGameInfo(const char* pRomName, uint32_t numBanks, uint32_t bankSize, uint32_t sramSize)
{
	printf("Game Info\n");
	printf("=-=-=-=-=-\n");
	printf("   Name: %s\n", pRomName);
	printf("   Num Banks: 0x%x\n", numBanks);
	printf("   Bank Size: 0x%x\n", bankSize);
	printf("   SRAM Size: 0x%x\n", sramSize);
	printf("=-=-=-=-=-\n");
}

void SuperCopierSN::TestAddresses()
{
	SetCartToIdleState();
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
		mCartEnablePin.Disable();
		WAIT();

		// Set the address to read a byte from
		printf("Setting Address to: %x\n", i);
		mAddressBus.SetAddress(i);
		WAIT();

		// Set the dataBus to HiZ
		mDataBus.HiZ();
		WAIT();

		// Now we're read, so enable the cartEnable
		mCartEnablePin.Enable();
		WAIT();

		// Grab the byte off the lines
		uint8_t value = mDataBus.Read();
		WAIT();
		printf("Address %x: Data %x\n", i, value);

		mDataBus.HiZ();
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
		} while (c != '\n');
		if (inputChoice == 'x')
		{
			break;
		}
	}
}

void SuperCopierSN::Execute()
{
	// Setup for a game to be inserted.
	SetCartToIdleState();
	WAIT();

	// Enable the Reset Pin so the SRAM (if there is one) draws from battery and not 5v)
	// it's critical that we ENABLE Reset, so that the transistor for sram (if there is one)
	// doesn't toggle to 5v the moment it touches the 5v line, causing bits to suddenly flip.
	mResetPin.Enable();
	WAIT();
	
	printf("INSERT GAME and press any key.\n");
	while(!getchar())
	{
	}

	//todo: write a function to read the ROM header and detect the game. Then, give them the chance to change it.
	// for now...
	const char gameName[] = "SUPERMARIOWORLD";
	const uint32_t numBanks = 0x10; //smw
	const uint32_t bankSize = 32768;//smw
	const uint32_t sramSize = 2048;//smw
		
	// Set Reset High so we send sram 5v and can read/write.
	mResetPin.Disable();
	WAIT();

	bool shouldExit = false;	
	while(!shouldExit)
	{
		printf("\n");
		printf("SuperCopier SN\n");
		printf("=-=-=-=-=-=-=-\n");
		PrintGameInfo(gameName, numBanks, bankSize, sramSize);
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
				DownloadFromSRAM(gameName, sramSize);
				break;
			}
			
			case 'u':
			{
				UploadToSRAM(gameName, sramSize);
				break;
			}
			
			case 'r':
			{
				DumpROM(gameName, numBanks, bankSize);
				break;
			}

			case 't':
			{
				TestAddresses();
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
	SetCartToIdleState();
	WAIT();

	mResetPin.Enable();
	WAIT();
		
	printf("REMOVE CARTRIDGE and press any key.\n");
	while(!getchar())
	{
	}
}
