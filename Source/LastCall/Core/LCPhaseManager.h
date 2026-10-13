// LAST CALL - the phase clock.
// ULCLocalPhaseManager is the ONLY owner of the current phase of the day
// (docs/ARCHITECTURE.md, section 2). Subsystems declare the phases they live
// in and are enabled/disabled on transition; UI, music and AI listen to
// FOnPhaseChanged. Phase boundaries are the only loading points in the game
// (PHILOSOPHY.md, section 8) and the only place the save file is written.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LCPhaseManager.generated.h"

/** The four phases of one in-game day (GAME-DESIGN.md, section 2). */
UENUM(BlueprintType)
enum class ELCPhase : uint8
{
	Morning,	// Prep: purchasing, upgrades, crafting
	Shift,		// Day/Evening: pouring, customers, social
	Night,		// After closing: action/stealth
	Summary		// Tips, XP, faction standing, heat -> next day
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLCPhaseChanged, ELCPhase, OldPhase, ELCPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLCDayAdvanced, int32, NewDay);

UCLASS()
class LASTCALL_API ULCPhaseManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Current phase. Never set directly outside this class - use RequestPhase. */
	UFUNCTION(BlueprintPure, Category = "Day")
	ELCPhase GetPhase() const { return Phase; }

	/** Current in-game day, 1-based. */
	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetDay() const { return Day; }

	/**
	 * Request a transition. Returns false for an illegal jump (phases only move
	 * forward within a day; Summary wraps to Morning of the next day and writes
	 * the save at the boundary).
	 */
	UFUNCTION(BlueprintCallable, Category = "Day")
	bool RequestPhase(ELCPhase NewPhase);

	/** Convenience: advance one phase along the loop. */
	UFUNCTION(BlueprintCallable, Category = "Day")
	bool AdvancePhase();

	/** Fired after the phase actually changed. Bind UI, music, AI here. */
	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnLCPhaseChanged OnPhaseChanged;

	/** Fired when Summary wraps into Morning: a new in-game day starts. */
	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnLCDayAdvanced OnDayAdvanced;

private:
	UPROPERTY(VisibleAnywhere, Category = "Day")
	ELCPhase Phase = ELCPhase::Morning;

	UPROPERTY(VisibleAnywhere, Category = "Day")
	int32 Day = 1;

	/** Next phase in the loop; Summary -> Morning of Day+1. */
	static ELCPhase NextPhase(ELCPhase InPhase);

	/**
	 * Phase boundary hook: this is where ULCSaveSystem writes (ARCHITECTURE.md 9).
	 * F0 records the intent; the save system itself lands in milestone F2.
	 */
	void NotifyPhaseBoundary(ELCPhase OldPhase, ELCPhase NewPhase);
};
