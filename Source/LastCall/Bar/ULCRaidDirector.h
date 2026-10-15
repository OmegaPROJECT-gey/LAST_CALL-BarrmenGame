// LAST CALL - the raid director wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of B4, wired to D3's morning pool: when the pool's
// morning pick is `raid`, today's search is scheduled (OnRaidScheduled),
// and the check that decides whether the police leave empty-handed or
// walk in on the rest runs through RunSearch - the engine-independent
// lc::resolve_raid (Bar/RaidCore, unit-tested) over the evidence that
// is out in the bar, with tuning read from Data/Balance/core.json
// "raid". Failure is heat and fines, never game over (GAME-DESIGN 5.2).
//
// What is evidence comes from the inventory, which does not exist yet -
// the caller passes it in, hide-tags and all. License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Bar/RaidCore.h"
#include "Events/ULCEventDirector.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCRaidDirector.generated.h"

/** What the search turned up, as Blueprint reads it. */
USTRUCT(BlueprintType)
struct FLCRaidResult
{
	GENERATED_BODY()

	/** Did the distraction check hold them at the door? */
	UPROPERTY(BlueprintReadOnly, Category = "Raid")
	bool DistractionWorked = false;

	/** Items the partner got into hiding in time. */
	UPROPERTY(BlueprintReadOnly, Category = "Raid")
	int32 HiddenByPartner = 0;

	/** Items still out when they walked in. */
	UPROPERTY(BlueprintReadOnly, Category = "Raid")
	int32 Found = 0;

	/** Heat this search stirred up (0 when nothing was found). */
	UPROPERTY(BlueprintReadOnly, Category = "Raid")
	double HeatDelta = 0.0;

	/** The fine per found item. */
	UPROPERTY(BlueprintReadOnly, Category = "Raid")
	double Fine = 0.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLCRaidScheduled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLCRaidResolved, const FLCRaidResult&,
	Result);

UCLASS()
class LASTCALL_API ULCRaidDirector : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: the search rules are known before the knock comes.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Did this morning's pick schedule today's search? Cleared by
	 * RunSearch and by the next morning's pick either way.
	 */
	UFUNCTION(BlueprintPure, Category = "Raid")
	bool IsRaidScheduled() const { return bRaidScheduled; }

	/**
	 * Resolve the search over what is out in the bar: the partner
	 * hides up to PartnerCapacity whatever happens, the distraction
	 * check (score vs balance raid.distraction_threshold) decides
	 * whether the police leave empty-handed, and found evidence costs
	 * per-item heat plus a fine - heat and fines, never game over.
	 * Broadcasts OnRaidResolved and clears today's schedule.
	 */
	UFUNCTION(BlueprintCallable, Category = "Raid")
	FLCRaidResult RunSearch(const TArray<FString>& Evidence, int32 PartnerCapacity,
		double DistractionScore);

	/** The morning's pick said: the police are coming today. */
	UPROPERTY(BlueprintAssignable, Category = "Raid")
	FOnLCRaidScheduled OnRaidScheduled;

	/** The search is over - this is what it cost. */
	UPROPERTY(BlueprintAssignable, Category = "Raid")
	FOnLCRaidResolved OnRaidResolved;

private:
	/** D3's morning pool decides when raids happen (B4's open item). */
	UFUNCTION()
	void HandleMorning(const FString& EventId, const FLCEventInfo& Event);

	/** Data/Balance core.json "raid"; compiled defaults until it loads. */
	lc::RaidTuning Tuning;

	bool bRaidScheduled = false;
};
