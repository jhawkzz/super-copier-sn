// MISC tests

void WriteReadRAMTest()
{
	// Use with the 15bit addressable ram chip.

	for (uint32_t i = 0; i < 65536; i++)
	{
		uint32_t address = i;
		uint32_t writeValue = i % 256;

		// write
		LOG("---Write---");
		LOG("Setting write enable high");
		gWriteEnable.Write(1);
		usleep(10);

		LOG("Setting data hiZ");
		gDataLines.HiZ();
		usleep(10);

		printf("Setting address 0%x\n", address);
		gAddressLines.SetAddress(address);
		usleep(10);

		LOG("Setting write enable low");
		gWriteEnable.Write(0);
		usleep(10);

		printf("Writing value: 0%x\n", writeValue);
		gDataLines.Write(writeValue);
		usleep(10);

		LOG("Setting write enable high");
		gWriteEnable.Write(1);
		usleep(10);

		LOG("Setting data hiZ");
		gDataLines.HiZ();
		usleep(10);
	}

	for (uint32_t i = 0; i < 65536; i++)
	{
		uint32_t address = i;

		// read
		LOG("---Read---");
		LOG("Setting write enable high");
		gWriteEnable.Write(1);
		usleep(10);

		printf("Setting address 0%x\n", address);
		gAddressLines.SetAddress(address);
		usleep(10);

		LOG("Setting data hiZ");
		gDataLines.HiZ();
		usleep(10);

		LOG("Reading");
		uint8_t value = gDataLines.Read();
		printf("Value at dataline: %x\n", value);
		usleep(10);

		if (value != i % 256)
		{
			printf("*****THIS DOES NOT MATCH******\n");
			return;
		}

		LOG("Setting data hiZ");
		gDataLines.HiZ();
		usleep(10);
	}

	printf("*****ALL VALUES MATCH******\n");
}

void HandleTests()
{
	if (argc > 2)
	{
		if (!strcmp(argv[1], "--data"))
		{
			uint8_t value = atoi(argv[2]);
			gDataLines.Write(value % 256);
		}
		else if (!strcmp(argv[1], "--write"))
		{
			uint8_t value = atoi(argv[2]);
			gWriteEnable.Write(value % 2);
		}
		else if (!strcmp(argv[1], "--address"))
		{
			uint16_t value = atoi(argv[2]);
			gAddressLines.SetAddress(value);

			printf("Set address to %x\n", value);
		}
		else if (!strcmp(argv[1], "--address-line-on"))
		{
			uint8_t value = atoi(argv[2]);
			gAddressLines.SetAddress(0x1 << value);
		}
		else if (!strcmp(argv[1], "--address-line-off"))
		{
			gAddressLines.SetAddress(0);
		}
		else if (!strcmp(argv[1], "--data-line-on"))
		{
			uint8_t value = atoi(argv[2]);
			gDataLines.Write(0x1 << value);
		}
	}
	else if (argc > 1)
	{
		if (!strcmp(argv[1], "--on"))
		{
			gAddressLines.SetAddress(65535);
			gDataLines.Write(0xFF);
			gWriteEnable.Write(1);
		}
		else if (!strcmp(argv[1], "--off"))
		{
			gAddressLines.SetAddress(0);
			gDataLines.Write(0);
			gWriteEnable.Write(0);
		}
		else if (!strcmp(argv[1], "--hiz"))
		{
			gAddressLines.HiZ();
			gDataLines.HiZ();
			gWriteEnable.HiZ();
		}
		else
		{
			LOG("Unknown arg. Did you forget a value for the arg?");
		}
	}
}