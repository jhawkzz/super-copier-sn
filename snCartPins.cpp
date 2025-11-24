
#include "snCartPins.h"

// SNCartEnablePin
void SNCartEnablePin::Enable()
{
	PullLow();
}

void SNCartEnablePin::Disable()
{
	PullHigh();
}
// End SNCartEnablePin

// SNResetPin
void SNResetPin::Enable()
{
	PullLow();
}

void SNResetPin::Disable()
{
	PullHigh();
}
// End SNResetPin

// SNWriteEnablePin
void SNWriteEnablePin::Enable()
{
	PullLow();
}

void SNWriteEnablePin::Disable()
{
	PullHigh();
}
// End SNWriteEnablePin