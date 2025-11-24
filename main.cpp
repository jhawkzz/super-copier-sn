
#include <gpiod.h>
#include <stdio.h>
#include "defines.h"
#include "superCopierSN.h"

int main(int argc, const char** argv)
{
	gpiod_chip* pChip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
	if (!pChip)
	{
		printf("main: open chip failed!\n");
		return 0;
	}

	if (argc > 1)
	{
		// kick off test harness stuff
		//todo: implement. See them in tests.cpp
	}
	else
	{
		SuperCopierSN::Get().Create(pChip);

		SuperCopierSN::Get().Execute();

		SuperCopierSN::Get().Release();
	}
	
	if(pChip)
	{
		gpiod_chip_close(pChip);
		pChip = nullptr;
	}
	
	return 0;
}
