// LAST CALL - the localization system (F3: the dialogue widget's text layer).
//
// The engine half of the bilingual text contract: Data/Localization holds
// en.json and ru.json as flat key -> string maps (their parity is what
// lc-dialogue-lint owns); this subsystem loads both at boot and answers
// GetText(key) in the language the player plays in. The default language
// follows the system culture (a Russian desktop gets Russian), and
// SetLanguage flips every lookup at runtime - the choice is a player
// setting, so the save may carry it later (F2 boundary, not here).
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCLocalizationSystem.generated.h"

UCLASS()
class LASTCALL_API ULCLocalizationSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: both languages are open before the first screen.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Load Data/Localization/en.json and ru.json. Refusals (unreadable
	 * file, bad JSON) are logged and that language stays empty; returns
	 * the number of keys the CURRENT language resolves (both files are
	 * parity-checked in CI, so the count is equal).
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	int32 LoadStrings();

	/**
	 * Play in this language ("en" or "ru"); unknown values are refused
	 * (logged) - a typo must not blank every screen.
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(const FString& Language);

	UFUNCTION(BlueprintPure, Category = "Localization")
	FString GetLanguage() const { return bRussian ? TEXT("ru") : TEXT("en"); }

	/** The text behind a localization key ("" when unknown). */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetText(const FString& Key) const;

	/** True when the key exists in the current language. */
	UFUNCTION(BlueprintPure, Category = "Localization")
	bool HasText(const FString& Key) const;

private:
	const TMap<FString, FString>& CurrentTable() const;

	TMap<FString, FString> StringsEn;
	TMap<FString, FString> StringsRu;
	bool bRussian = false;
};
