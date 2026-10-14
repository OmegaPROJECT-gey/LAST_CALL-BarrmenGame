// LAST CALL - the endings wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of C4: a game-instance subsystem over the
// engine-independent lc::EndingLibrary - what the ending screen reads on
// the last day. The selection math (standing x information x venue tier,
// first matching rule wins) stays in Story/EndingsCore where
// scripts/test-endings.sh checks it without Unreal; this class loads
// Data/Endings from disk and hands Blueprint the chosen ending with its
// bilingual name and the localization keys of its last night and morning
// after - one pair per venue tier.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Story/EndingsCore.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCEndingsLibrary.generated.h"

/** One of the six endings as the ending screen reads it. */
USTRUCT(BlueprintType)
struct FLCEndingChoice
{
	GENERATED_BODY()

	/** Ending id ("archive", "closed", "keeper", ...). */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	FString Id;

	/** Bilingual ending name, straight from Data/Endings. */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	FString NameEn;

	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	FString NameRu;

	/** Localization key of this ending's last night for the venue tier. */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	FString NightKey;

	/** Localization key of the morning after. */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	FString MorningKey;

	/** Venue tier the texts were read for (1..5). */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	int32 Tier = 1;

	/** False when the selection refused (bad tier, library not loaded). */
	UPROPERTY(BlueprintReadOnly, Category = "Endings")
	bool bValid = false;
};

UCLASS()
class LASTCALL_API ULCEndingsLibrary : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: the endings are open before the last day needs them.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Reload Data/Endings/endings.json from disk. Refusals (bad JSON,
	 * a rule naming no ending, a missing `always` backstop) are logged;
	 * returns the number of loaded endings (0 when refused).
	 */
	UFUNCTION(BlueprintCallable, Category = "Endings")
	int32 LoadEndings();

	/**
	 * How many pieces of information these held items are - one per held
	 * wall fragment the document lists (the archive's 13).
	 */
	UFUNCTION(BlueprintPure, Category = "Endings")
	int32 CountInformation(const TArray<FString>& HeldItems) const;

	/**
	 * The ending for this state. `Standing` is faction id -> value and
	 * should carry every faction the standing ledger knows; `Information`
	 * comes from CountInformation(); `VenueTier` is 1..5 (the growth
	 * ladder). Refusals return an invalid row with bValid = false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Endings")
	FLCEndingChoice SelectEnding(const TMap<FString, double>& Standing,
		int32 Information, int32 VenueTier) const;

	/** All six ending ids, in load order. */
	UFUNCTION(BlueprintPure, Category = "Endings")
	TArray<FString> GetEndingIds() const;

private:
	lc::EndingLibrary Library;
};
