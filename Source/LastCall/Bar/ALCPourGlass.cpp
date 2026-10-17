// LAST CALL - the pour glass actor implementation. See header for the contract.

#include "Bar/ALCPourGlass.h"

void ALCPourGlass::ApplyFill(float InFillFraction, float InVolumeMl, float InSpillMl,
	bool bInOverflowed)
{
	FillFraction = FMath::Clamp(InFillFraction, 0.0f, 1.0f);
	VolumeMl = FMath::Max(InVolumeMl, 0.0f);
	SpillMl = FMath::Max(InSpillMl, 0.0f);
	bOverflowed = bInOverflowed;
}

void ALCPourGlass::ClearGlass()
{
	ApplyFill(0.0f, 0.0f, 0.0f, false);
}
