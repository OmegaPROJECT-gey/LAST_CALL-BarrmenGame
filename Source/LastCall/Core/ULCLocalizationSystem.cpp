// LAST CALL - the localization system implementation. See header for the contract.

#include "Core/ULCLocalizationSystem.h"

#include "Core/JsonCore.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString LocalizationFilePath(const FString& Language)
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Localization"),
			Language + TEXT(".json"));
	}

	bool LoadLanguageFile(const FString& Language, TMap<FString, FString>& Out)
	{
		const FString Path = LocalizationFilePath(Language);
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path))
		{
			UE_LOG(LogLastCall, Warning, TEXT("Localization file unreadable: %s"), *Path);
			return false;
		}

		FTCHARToUTF8 Converter(*Text);
		lc::JsonValue Doc;
		std::string Error;
		if (!lc::json_parse(std::string(Converter.Get(), Converter.Length()), Doc, &Error)
			|| Doc.kind != lc::JsonValue::Kind::Object)
		{
			UE_LOG(LogLastCall, Warning, TEXT("Localization refused (%s): %s"), *Path,
				UTF8_TO_TCHAR(Error.c_str()));
			return false;
		}

		Out.Empty(static_cast<int32>(Doc.object.size()));
		for (const std::pair<std::string, lc::JsonValue>& Entry : Doc.object)
		{
			if (Entry.second.kind == lc::JsonValue::Kind::String)
			{
				Out.Add(FString(UTF8_TO_TCHAR(Entry.first.c_str())),
					FString(UTF8_TO_TCHAR(Entry.second.string.c_str())));
			}
		}
		return true;
	}
} // namespace

void ULCLocalizationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// The desktop the player sits at picks the default; SetLanguage
	// overrides it (and a future save may carry the choice).
	const FString Culture =
		FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
	bRussian = Culture.Equals(TEXT("ru"), ESearchCase::IgnoreCase);

	LoadStrings();
}

int32 ULCLocalizationSystem::LoadStrings()
{
	LoadLanguageFile(TEXT("en"), StringsEn);
	LoadLanguageFile(TEXT("ru"), StringsRu);
	return CurrentTable().Num();
}

void ULCLocalizationSystem::SetLanguage(const FString& Language)
{
	if (Language.Equals(TEXT("ru"), ESearchCase::IgnoreCase))
	{
		bRussian = true;
	}
	else if (Language.Equals(TEXT("en"), ESearchCase::IgnoreCase))
	{
		bRussian = false;
	}
	else
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("SetLanguage refused '%s' - the language stays '%s'"),
			*Language, *GetLanguage());
	}
}

FText ULCLocalizationSystem::GetText(const FString& Key) const
{
	const FString* Found = CurrentTable().Find(Key);
	return Found ? FText::FromString(*Found) : FText::GetEmpty();
}

bool ULCLocalizationSystem::HasText(const FString& Key) const
{
	return CurrentTable().Contains(Key);
}

const TMap<FString, FString>& ULCLocalizationSystem::CurrentTable() const
{
	return bRussian ? StringsRu : StringsEn;
}
