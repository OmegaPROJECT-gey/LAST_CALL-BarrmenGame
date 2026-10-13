#include "Core/LCPhaseManager.h"

#include "LastCall.h"

void ULCPhaseManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogLastCall, Verbose, TEXT("PhaseManager initialized on day %d"), Day);
}

void ULCPhaseManager::Deinitialize()
{
	Super::Deinitialize();
}

ELCPhase ULCPhaseManager::NextPhase(ELCPhase InPhase)
{
	switch (InPhase)
	{
	case ELCPhase::Morning: return ELCPhase::Shift;
	case ELCPhase::Shift:   return ELCPhase::Night;
	case ELCPhase::Night:   return ELCPhase::Summary;
	case ELCPhase::Summary: return ELCPhase::Morning;
	default:                return ELCPhase::Morning;
	}
}

bool ULCPhaseManager::RequestPhase(ELCPhase NewPhase)
{
	if (NewPhase == Phase)
	{
		return false;
	}

	// The day only moves forward: Summary is the single place that starts a new day.
	const bool bWrapsToNewDay = (Phase == ELCPhase::Summary && NewPhase == ELCPhase::Morning);
	if (!bWrapsToNewDay && NewPhase != NextPhase(Phase))
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Illegal phase jump %d -> %d; the day moves forward only"),
			static_cast<int32>(Phase), static_cast<int32>(NewPhase));
		return false;
	}

	const ELCPhase OldPhase = Phase;
	Phase = NewPhase;

	if (bWrapsToNewDay)
	{
		++Day;
		UE_LOG(LogLastCall, Log, TEXT("Day %d begins"), Day);
		OnDayAdvanced.Broadcast(Day);
	}

	NotifyPhaseBoundary(OldPhase, Phase);
	OnPhaseChanged.Broadcast(OldPhase, Phase);
	return true;
}

bool ULCPhaseManager::AdvancePhase()
{
	return RequestPhase(NextPhase(Phase));
}

void ULCPhaseManager::NotifyPhaseBoundary(ELCPhase OldPhase, ELCPhase NewPhase)
{
	// Phase boundaries are the only loading points (PHILOSOPHY 8) and the only
	// place the save file is written (ARCHITECTURE 9). F2 wires ULCSaveSystem
	// here; until then the log is the record.
	UE_LOG(LogLastCall, Log, TEXT("Phase boundary %d -> %d (save point)"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
}
