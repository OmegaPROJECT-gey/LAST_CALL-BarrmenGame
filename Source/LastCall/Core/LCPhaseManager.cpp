// LAST CALL - the phase clock. See header for the contract.

#include "Core/LCPhaseManager.h"

#include "Core/BalanceCore.h"
#include "Core/ULCSaveSystem.h"
#include "LastCall.h"
#include "Misc/Paths.h"

namespace
{
	std::string BalancePathUtf8()
	{
		const FString Path =
			FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Balance"), TEXT("core.json"));
		FTCHARToUTF8 Converter(*Path);
		return std::string(Converter.Get(), Converter.Length());
	}
} // namespace

void ULCPhaseManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Phase durations come from Data/Balance/core.json (F1/F2); a refusal
	// keeps the compiled defaults, which the unit tests pin to the same
	// file - the game never runs on numbers nobody checked.
	lc::BalanceData Balance;
	std::string BalanceError;
	if (lc::load_balance_file(BalancePathUtf8(), Balance, &BalanceError))
	{
		Spec = Balance.phases;
		UE_LOG(LogLastCall, Verbose,
			TEXT("Balance: phases %.0fs / %.0fs / %.0fs / %.0fs"),
			Spec.morning_seconds, Spec.shift_seconds,
			Spec.night_seconds, Spec.summary_seconds);
	}
	else
	{
		UE_LOG(LogLastCall, Warning, TEXT("Balance refused (%s) - compiled phase defaults stay"),
			UTF8_TO_TCHAR(BalanceError.c_str()));
	}

	UE_LOG(LogLastCall, Verbose, TEXT("PhaseManager initialized on day %d"), Clock.day);
}

void ULCPhaseManager::Deinitialize()
{
	Super::Deinitialize();
}

ELCPhase ULCPhaseManager::GetPhase() const
{
	return ToEngine(Clock.phase);
}

int32 ULCPhaseManager::GetDay() const
{
	return Clock.day;
}

int32 ULCPhaseManager::GetHospitalDays() const
{
	return Clock.hospital_days_left;
}

void ULCPhaseManager::AdmitToHospital(int32 Days)
{
	lc::admit_to_hospital(Clock, Days);
}

bool ULCPhaseManager::RequestPhase(ELCPhase NewPhase)
{
	const lc::Phase From = Clock.phase;
	const lc::PhaseTransition Transition = lc::request_phase(Clock, ToCore(NewPhase));
	if (!Transition.changed)
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Illegal phase jump %d -> %d; the day moves forward only"),
			static_cast<int32>(From), static_cast<int32>(ToCore(NewPhase)));
		return false;
	}

	FinishTransition(Transition);
	return true;
}

bool ULCPhaseManager::AdvancePhase()
{
	const lc::PhaseTransition Transition = lc::advance_phase(Clock);
	if (!Transition.changed)
	{
		return false;
	}

	FinishTransition(Transition);
	return true;
}

int32 ULCPhaseManager::Tick(float DtSeconds)
{
	if (DtSeconds <= 0.0f)
	{
		return 0;
	}

	const std::vector<lc::PhaseTransition> Transitions =
		lc::tick(Clock, static_cast<double>(DtSeconds), Spec);
	for (const lc::PhaseTransition& Transition : Transitions)
	{
		FinishTransition(Transition);
	}
	return static_cast<int32>(Transitions.size());
}

float ULCPhaseManager::GetPhaseProgress() const
{
	return static_cast<float>(lc::phase_progress(Clock, Spec));
}

void ULCPhaseManager::ApplyLoadedState(int32 InDay, ELCPhase InPhase, int32 InHospitalDaysLeft)
{
	const ELCPhase From = GetPhase();

	Clock.day = InDay < 1 ? 1 : InDay;
	Clock.phase = ToCore(InPhase);
	Clock.phase_time_s = 0.0;
	Clock.day_time_s = 0.0;
	Clock.save_requested = false; // loading is not a boundary of its own
	lc::admit_to_hospital(Clock, InHospitalDaysLeft); // B1: the stay rides in

	UE_LOG(LogLastCall, Log, TEXT("State applied: day %d, phase %d, hospital days %d"),
		Clock.day, static_cast<int32>(InPhase), Clock.hospital_days_left);

	if (From != InPhase)
	{
		OnPhaseChanged.Broadcast(From, InPhase);
	}
}

ELCPhase ULCPhaseManager::ToEngine(lc::Phase InPhase)
{
	switch (InPhase)
	{
	case lc::Phase::Morning: return ELCPhase::Morning;
	case lc::Phase::Shift:   return ELCPhase::Shift;
	case lc::Phase::Night:   return ELCPhase::Night;
	case lc::Phase::Summary: return ELCPhase::Summary;
	default:                 return ELCPhase::Morning;
	}
}

lc::Phase ULCPhaseManager::ToCore(ELCPhase InPhase)
{
	switch (InPhase)
	{
	case ELCPhase::Morning: return lc::Phase::Morning;
	case ELCPhase::Shift:   return lc::Phase::Shift;
	case ELCPhase::Night:   return lc::Phase::Night;
	case ELCPhase::Summary: return lc::Phase::Summary;
	default:                return lc::Phase::Morning;
	}
}

void ULCPhaseManager::FinishTransition(const lc::PhaseTransition& Transition)
{
	const ELCPhase From = ToEngine(Transition.from);
	const ELCPhase To = ToEngine(Transition.to);

	if (Transition.new_day)
	{
		UE_LOG(LogLastCall, Log, TEXT("Day %d begins"), Clock.day);
		OnDayAdvanced.Broadcast(Clock.day);
	}

	// Phase boundaries are the only loading points (PHILOSOPHY 8) and the
	// only place the save file is written (ARCHITECTURE 9).
	NotifyPhaseBoundary();
	OnPhaseChanged.Broadcast(From, To);
}

void ULCPhaseManager::NotifyPhaseBoundary()
{
	if (!Clock.save_requested)
	{
		return; // the core refused before reaching any boundary
	}
	Clock.save_requested = false; // consumed here, once

	UGameInstance* GameInstance = GetGameInstance();
	ULCSaveSystem* SaveSystem =
		GameInstance ? GameInstance->GetSubsystem<ULCSaveSystem>() : nullptr;
	if (!SaveSystem)
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Phase boundary reached without ULCSaveSystem - save point skipped"));
		return;
	}

	if (SaveSystem->WriteSave())
	{
		UE_LOG(LogLastCall, Log, TEXT("Save point written (day %d, phase %d)"),
			Clock.day, static_cast<int32>(ToEngine(Clock.phase)));
	}
}
