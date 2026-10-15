// LAST CALL - the morning events wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of D3: a game-instance subsystem over the
// engine-independent lc::EventLibrary - what the morning phase asks
// when a new day opens. The rules (weights from standing, heat and
// day, the file-order walk over an engine-owned roll, zero-weight rows
// never firing) stay in Events/EventCore where scripts/test-events.sh
// checks them without Unreal; this class loads Data/Events from disk,
// answers Blueprint's weight/selection questions and broadcasts what
// the morning picked so music, crowd and the raid director can react.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Core/LCPhaseManager.h"
#include "Events/EventCore.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCEventDirector.generated.h"

/** One row of the morning pool as the UI reads it. */
USTRUCT(BlueprintType)
struct FLCEventInfo
{
	GENERATED_BODY()

	/** Event id ("raid", "blackout", "quiet_day", ...). */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString Id;

	/** Bilingual event name, straight from Data/Events. */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString NameEn;

	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString NameRu;

	/** Base weight before standing, heat and day add their slopes. */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	double WeightBase = 0.0;

	/** Additive weight per heat point - how the raid finds you. */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	double PerHeat = 0.0;

	/** Additive weight per campaign day. */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	double PerDay = 0.0;

	/** Bodies this pick adds to (or takes out of) the room. */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int32 CrowdDelta = 0;

	/** Flow of orders against the baseline (1.0 = untouched). */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	double OrdersMultiplier = 1.0;
};

/** What the morning picked: the subsystem found out, now you react. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLCMorningEvent, const FString&, EventId,
	const FLCEventInfo&, Event);

UCLASS()
class LASTCALL_API ULCEventDirector : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: the pool is known before the morning asks.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Reload Data/Events/events.json from disk. Refusals (bad JSON, a
	 * duplicate id, a row that can never fire) are logged; returns the
	 * number of loaded events (0 when refused).
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	int32 LoadEvents();

	/** All pool rows, in load order, for a debug/summary list. */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FLCEventInfo> GetEvents() const;

	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FString> GetEventIds() const;

	/** One row by id; an unknown id returns an invalid struct. */
	UFUNCTION(BlueprintPure, Category = "Events")
	FLCEventInfo GetEvent(const FString& EventId) const;

	/**
	 * This event's weight in this world: base + per-point biases x
	 * standing + per_heat x heat + per_day x day, clamped at zero.
	 * An unknown event weighs 0 (it never fires).
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	double EventWeight(const FString& EventId, const TMap<FString, double>& Standing,
		double Heat, int32 Day) const;

	/**
	 * The morning's pick: `Roll` in [0,1) walks the pool in file
	 * order over the total weight (the caller owns the RNG). Returns
	 * "" when nothing weighs anything in this world - never an event
	 * whose weight is zero.
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	FString SelectEvent(const TMap<FString, double>& Standing, double Heat, int32 Day,
		double Roll) const;

	/**
	 * Select and tell the room: picks the event and broadcasts
	 * OnMorningEvent when something actually happened. Returns the
	 * picked id ("" = a quiet nothing).
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	FString TriggerMorning(const TMap<FString, double>& Standing, double Heat, int32 Day,
		double Roll);

	/**
	 * The phase-flow trigger: the day, heat and standing this director
	 * already knows, rolled against the stream, broadcast on the
	 * OnMorningEvent above. Bound to ULCPhaseManager's Morning below;
	 * callable for a manual replay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	FString RunMorning();

	/**
	 * Where the standing comes from while there is no faction system
	 * yet (interim state in the ULCSaveSystem sense: the owner binds
	 * later and nothing here changes shape).
	 */
	UFUNCTION(BlueprintCallable, Category = "Events|Interim")
	void SetStanding(const TMap<FString, double>& InStanding);

	/** One faction's standing, for the reaction that just moved it. */
	UFUNCTION(BlueprintCallable, Category = "Events|Interim")
	void SetStandingPoint(const FString& FactionId, double Value);

	UFUNCTION(BlueprintPure, Category = "Events|Interim")
	TMap<FString, double> GetStanding() const;

	/** What the last morning picked (invalid struct = nothing yet). */
	UFUNCTION(BlueprintPure, Category = "Events")
	FLCEventInfo GetLastEvent() const;

	/**
	 * Fix the morning's roll for a reproducible run (0 = a fresh seed
	 * from the clock). The engine owns the roll; this is the engine's
	 * hand on it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void SetRandomSeed(int32 Seed);

	/** Broadcast by TriggerMorning when the morning names an event. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLCMorningEvent OnMorningEvent;

private:
	/** ULCPhaseManager's Morning boundary is where the day opens. */
	UFUNCTION()
	void HandlePhaseChanged(ELCPhase OldPhase, ELCPhase NewPhase);

	/** Subsystem the day clock lives in (lazily: init order is free). */
	class ULCPhaseManager* PhaseManager() const;

	/** Interim heat until the heat meter owns it (ULCSaveSystem). */
	class ULCSaveSystem* SaveSystem() const;

	lc::EventLibrary Library;

	/** Interim standing (see SetStanding). */
	TMap<FString, double> StandingState;

	/** The event the last morning picked, kept for the summary UI. */
	FLCEventInfo LastEvent;

	/** The engine's roll: same seed, same morning. */
	FRandomStream Stream;
	bool Seeded = false;
};
