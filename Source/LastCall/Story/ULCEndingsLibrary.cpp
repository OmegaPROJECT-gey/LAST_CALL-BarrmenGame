// LAST CALL - the endings wrapper. See header for the contract.

#include "Story/ULCEndingsLibrary.h"

#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString EndingsFilePath()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Endings"),
			TEXT("endings.json"));
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
} // namespace

void ULCEndingsLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadEndings();
}

int32 ULCEndingsLibrary::LoadEndings()
{
	const FString Path = EndingsFilePath();
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Endings file unreadable: %s"), *Path);
		return 0;
	}

	lc::EndingLibrary Fresh;
	std::string Error;
	if (!Fresh.load(ToUtf8(Text), &Error))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Endings refused: %s"), *Utf8ToFString(Error));
		return 0;
	}

	Library = std::move(Fresh);
	UE_LOG(LogLastCall, Log, TEXT("Endings: %d endings loaded"),
		static_cast<int32>(Library.endings().size()));
	return static_cast<int32>(Library.endings().size());
}

int32 ULCEndingsLibrary::CountInformation(const TArray<FString>& HeldItems) const
{
	std::vector<std::string> Held;
	Held.reserve(HeldItems.Num());
	for (const FString& Item : HeldItems)
	{
		Held.push_back(ToUtf8(Item));
	}
	return Library.count_information(Held);
}

FLCEndingChoice ULCEndingsLibrary::SelectEnding(const TMap<FString, double>& Standing,
	int32 Information, int32 VenueTier) const
{
	std::vector<std::pair<std::string, double>> StandingUtf8;
	StandingUtf8.reserve(Standing.Num());
	for (const TPair<FString, double>& Entry : Standing)
	{
		StandingUtf8.emplace_back(ToUtf8(Entry.Key), Entry.Value);
	}

	std::string Error;
	const lc::EndingChoice Choice =
		Library.select(StandingUtf8, Information, VenueTier, &Error);
	if (Choice.id.empty())
	{
		UE_LOG(LogLastCall, Warning, TEXT("Ending refused: %s"), *Utf8ToFString(Error));
		return FLCEndingChoice();
	}

	FLCEndingChoice Row;
	Row.Id = Utf8ToFString(Choice.id);
	Row.NameEn = Utf8ToFString(Choice.name_en);
	Row.NameRu = Utf8ToFString(Choice.name_ru);
	Row.NightKey = Utf8ToFString(Choice.night_key);
	Row.MorningKey = Utf8ToFString(Choice.morning_key);
	Row.Tier = Choice.tier;
	Row.bValid = true;
	return Row;
}

TArray<FString> ULCEndingsLibrary::GetEndingIds() const
{
	TArray<FString> Ids;
	for (const lc::EndingSpec& Spec : Library.endings())
	{
		Ids.Add(Utf8ToFString(Spec.id));
	}
	return Ids;
}
