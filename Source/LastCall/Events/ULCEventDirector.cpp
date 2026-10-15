// LAST CALL - the morning events wrapper. See header for the contract.

#include "Events/ULCEventDirector.h"

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
	Super::Initialize(Collection);
	LoadEvents();
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
	OnMorningEvent.Broadcast(Picked, Event);
	return Picked;
}
