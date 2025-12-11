
#include "snCartIO.h"

void SNCartIO::Create(gpiod_chip* pChip)
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

SNCartIO::~SNCartIO()
{
    Release();
}

void SNCartIO::Release()
{
    mCartEnablePin.Release();
    mResetPin.Release();
    mWriteEnablePin.Release();
    mReadEnablePin.Release();

    mDataBus.Release();
    mAddressBus.Release();
}
