// LAST CALL - the morning events wrapper. See header for the contract.

#include "Events/ULCEventDirector.h"

#include "Core/ULCSaveSystem.h"
#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString EventsFilePath()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Events"),
			TEXT("events.json"));
	}

	FString Utf8ToFString(const std::string& Text)
	{
		return FString(UTF8_TO_TCHAR(Text.c_str()));
	}

	std::string ToUtf8(const FString& Text)
	{
		FTCHARToUTF8 Converter(*Text);
		return std::string(Converter.Get(), Converter.Length());
	}

	std::vector<std::pair<std::string, double>> ToStanding(
		const TMap<FString, double>& Standing)
	{
		std::vector<std::pair<std::string, double>> Converted;
		Converted.reserve(Standing.Num());
		for (const TPair<FString, double>& Entry : Standing)
		{
			Converted.emplace_back(ToUtf8(Entry.Key), Entry.Value);
		}
		return Converted;
	}
} // namespace

void ULCEventDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	// The phase clock opens the morning; make sure it exists before we
	// listen (init order between the two is free otherwise).
	Super::Initialize(Collection);
	ULCPhaseManager* Phases = Collection.InitializeDependency<ULCPhaseManager>();
	LoadEvents();

	if (Phases)
	{
		Phases->OnPhaseChanged.AddDynamic(this, &ULCEventDirector::HandlePhaseChanged);
		UE_LOG(LogLastCall, Verbose, TEXT("Event director bound to the phase clock (day %d)"),
			Phases->GetDay());
	}
}

int32 ULCEventDirector::LoadEvents()
{
	const FString Path = EventsFilePath();
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Event pool file unreadable: %s"), *Path);
		return 0;
	}

	lc::EventLibrary Fresh;
	std::string Error;
	if (!Fresh.load(ToUtf8(Text), &Error))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Event pool refused: %s"), *Utf8ToFString(Error));
		return 0;
	}

	Library = std::move(Fresh);
	UE_LOG(LogLastCall, Log, TEXT("Event pool: %d mornings loaded"),
		static_cast<int32>(Library.events().size()));
	return static_cast<int32>(Library.events().size());
}

TArray<FLCEventInfo> ULCEventDirector::GetEvents() const
{
	TArray<FLCEventInfo> Rows;
	for (const lc::EventSpec& Spec : Library.events())
	{
		FLCEventInfo Row;
		Row.Id = Utf8ToFString(Spec.id);
		Row.NameEn = Utf8ToFString(Spec.name_en);
		Row.NameRu = Utf8ToFString(Spec.name_ru);
		Row.WeightBase = Spec.weight_base;
		Row.PerHeat = Spec.per_heat;
		Row.PerDay = Spec.per_day;
		Row.CrowdDelta = Spec.effects.crowd_delta;
		Row.OrdersMultiplier = Spec.effects.orders_multiplier;
		Rows.Add(Row);
	}
	return Rows;
}

TArray<FString> ULCEventDirector::GetEventIds() const
{
	TArray<FString> Ids;
	for (const lc::EventSpec& Spec : Library.events())
	{
		Ids.Add(Utf8ToFString(Spec.id));
	}
	return Ids;
}

FLCEventInfo ULCEventDirector::GetEvent(const FString& EventId) const
{
	FLCEventInfo Row;
	const lc::EventSpec* Spec = Library.find(ToUtf8(EventId));
	if (!Spec)
	{
		return Row;
	}
	Row.Id = Utf8ToFString(Spec->id);
	Row.NameEn = Utf8ToFString(Spec->name_en);
	Row.NameRu = Utf8ToFString(Spec->name_ru);
	Row.WeightBase = Spec->weight_base;
	Row.PerHeat = Spec->per_heat;
	Row.PerDay = Spec->per_day;
	Row.CrowdDelta = Spec->effects.crowd_delta;
	Row.OrdersMultiplier = Spec->effects.orders_multiplier;
	return Row;
}

double ULCEventDirector::EventWeight(const FString& EventId,
	const TMap<FString, double>& Standing, double Heat, int32 Day) const
{
	const lc::EventSpec* Spec = Library.find(ToUtf8(EventId));
	if (!Spec)
	{
		return 0.0;
	}
	return Library.weight(*Spec, ToStanding(Standing), Heat, Day);
}

FString ULCEventDirector::SelectEvent(const TMap<FString, double>& Standing,
	double Heat, int32 Day, double Roll) const
{
	return Utf8ToFString(Library.select(ToStanding(Standing), Heat, Day, Roll));
}

FString ULCEventDirector::TriggerMorning(const TMap<FString, double>& Standing,
	double Heat, int32 Day, double Roll)
{
	LastEvent = FLCEventInfo(); // this morning starts knowing nothing

	const FString Picked = SelectEvent(Standing, Heat, Day, Roll);
	if (Picked.IsEmpty())
	{
		return Picked;
	}

	const FLCEventInfo Event = GetEvent(Picked);
	if (Event.Id.IsEmpty())
	{
		return FString(); // the core picked something the reload lost
	}
	LastEvent = Event;
	OnMorningEvent.Broadcast(Picked, Event);
	return Picked;
}

FString ULCEventDirector::RunMorning()
{
	if (!Seeded)
	{
		Stream.Initialize(static_cast<int32>(FDateTime::UtcNow().GetTicks() & 0x7fffffff));
		Seeded = true;
	}

	const ULCPhaseManager* Phases = PhaseManager();
	const ULCSaveSystem* Save = SaveSystem();
	const int32 Day = Phases ? FMath::Max(Phases->GetDay(), 1) : 1;
	const double Heat = Save ? FMath::Max(static_cast<double>(Save->GetHeat()), 0.0) : 0.0;
	return TriggerMorning(StandingState, Heat, Day, Stream.FRand());
}

void ULCEventDirector::SetStanding(const TMap<FString, double>& InStanding)
{
	StandingState = InStanding;
}

void ULCEventDirector::SetStandingPoint(const FString& FactionId, double Value)
{
	StandingState.Add(FactionId, Value);
}

TMap<FString, double> ULCEventDirector::GetStanding() const
{
	return StandingState;
}

FLCEventInfo ULCEventDirector::GetLastEvent() const
{
	return LastEvent;
}

void ULCEventDirector::SetRandomSeed(int32 Seed)
{
	Stream.Initialize(Seed);
	Seeded = true;
}

void ULCEventDirector::HandlePhaseChanged(ELCPhase /*OldPhase*/, ELCPhase NewPhase)
{
	if (NewPhase == ELCPhase::Morning)
	{
		RunMorning();
	}
}

ULCPhaseManager* ULCEventDirector::PhaseManager() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULCPhaseManager>() : nullptr;
}

ULCSaveSystem* ULCEventDirector::SaveSystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULCSaveSystem>() : nullptr;
}
