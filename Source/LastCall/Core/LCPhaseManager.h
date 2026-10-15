// LAST CALL - the phase clock.
// ULCPhaseManager is the ONLY owner of the current phase of the day
// (docs/ARCHITECTURE.md, section 2). Subsystems declare the phases they live
// in and are enabled/disabled on transition; UI, music and AI listen to
// FOnPhaseChanged. Phase boundaries are the only loading points in the game
// (PHILOSOPHY.md, section 8) and the only place the save file is written.
//
// The state machine itself is the engine-independent lc::DayClock
// (Core/DayScheduleCore - unit-tested as F2 groundwork); this class is the
// thin UE half: Blueprint API, delegates, and the save hook at boundaries.

#pragma once

#include "CoreMinimal.h"
#include "Core/DayScheduleCore.h"
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

	/** Current phase. */
	UFUNCTION(BlueprintPure, Category = "Day")
	ELCPhase GetPhase() const;

	/** Current in-game day, 1-based. */
	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetDay() const;

	/**
	 * Hospital days the story still sits out (B1, GAME-DESIGN 4): the
	 * next Summary -> Morning boundary jumps over all of them at once.
	 */
	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetHospitalDays() const;

	/**
	 * Death is consequences (GAME-DESIGN 4): the run lands in the
	 * hospital for `Days` days and resumes later - the stay is spent in
	 * one jump at the next day boundary and rides into the save. 0 or
	 * less clears a stay already set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Day")
	void AdmitToHospital(int32 Days);

	/**
	 * Request a transition. Skipping forward within the day is allowed
	 * (GAME-DESIGN 2: phases are skippable); backward jumps and no-op
	 * requests are refused - the day moves forward only, consequences
	 * stick (PHILOSOPHY 3.2). Summary -> Morning of the next day is only
	 * reachable through AdvancePhase or Tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Day")
	bool RequestPhase(ELCPhase NewPhase);

	/** Convenience: advance one phase along the loop. */
	UFUNCTION(BlueprintCallable, Category = "Day")
	bool AdvancePhase();

	/**
	 * Advance time: a timed phase (morning prep, summary) auto-advances
	 * when its duration runs out; untimed phases (shift, night) just
	 * accumulate. Returns how many boundaries fired.
	 */
	UFUNCTION(BlueprintCallable, Category = "Day")
	int32 Tick(float DtSeconds);

	/**
	 * Share of the current phase's time spent (>= 1 means done); untimed
	 * phases report 0 - the shift ends when the bar closes, not by clock.
	 */
	UFUNCTION(BlueprintPure, Category = "Day")
	float GetPhaseProgress() const;

	/** Fired after the phase actually changed. Bind UI, music, AI here. */
	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnLCPhaseChanged OnPhaseChanged;

	/** Fired when Summary wraps into Morning: a new in-game day starts. */
	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnLCDayAdvanced OnDayAdvanced;

	/**
	 * Apply a loaded save without writing one back (ULCSaveSystem on
	 * boot): resets the phase timers, restores the hospital stay and
	 * broadcasts the change when the phase is one. Plain C++ - the save
	 * subsystem calls it, not Blueprint.
	 */
	void ApplyLoadedState(int32 InDay, ELCPhase InPhase, int32 InHospitalDaysLeft);

	/** ELCPhase <-> lc::Phase, for code talking to the cores. */
	static ELCPhase ToEngine(lc::Phase InPhase);
	static lc::Phase ToCore(ELCPhase InPhase);

private:
	/** The state machine itself: the engine half runs the tested core. */
	lc::DayClock Clock;
	lc::DayScheduleSpec Spec;

	/** Common tail of every accepted transition: day log, save, delegates. */
	void FinishTransition(const lc::PhaseTransition& Transition);

	/**
	 * Phase boundary hook: this is where ULCSaveSystem writes
	 * (ARCHITECTURE.md 9) - consumes the core's save_requested flag.
	 */
	void NotifyPhaseBoundary();
};
