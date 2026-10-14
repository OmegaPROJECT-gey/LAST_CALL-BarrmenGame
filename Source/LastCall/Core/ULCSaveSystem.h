// LAST CALL - the save system (milestone F2, ARCHITECTURE.md section 9).
//
// One slot, day granularity, undo-free: consequences stick (PHILOSOPHY 3.2).
// The payload is the engine-independent lc::SaveState text block
// (Core/SaveStateCore - versioned, strict, unit-tested): this subsystem
// only stores it in the engine's SaveGame slot and writes it exclusively
// at phase boundaries, when ULCPhaseManager asks (the core's
// save_requested flag, consumed at the boundary).
//
// Money and heat live here as interim state until their real owners (the
// wallet UI and the heat meter) wrap their cores - the save format does
// not change when they bind.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Core/SaveStateCore.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCSaveSystem.generated.h"

/**
 * The slot's contents: the core's text payload verbatim. Debuggable with
 * `cat`; deserialization is strict - a foreign or older payload is
 * refused, never silently migrated (a lost run is honest, a corrupted one
 * is not).
 */
UCLASS()
class LASTCALL_API ULCSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Save")
	FString Payload;
};

UCLASS()
class LASTCALL_API ULCSaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Serialize the current state into the single slot. Called from the
	 * phase boundary hook; Blueprint may call it too (manual save at a
	 * shop door - it changes nothing about where writes are legal: PHIL 8).
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool WriteSave();

	/**
	 * Read the slot and apply it to the day clock. Returns false for a
	 * missing or refused payload (logged); the state stays untouched then.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadSave();

	/** Is there a slot to continue? */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSave() const;

	/** Drop the slot (a new game). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave();

	// --- interim state: replaced by the real systems when they land ---

	/** Balance: starting_money (Data/Balance/core.json). */
	UFUNCTION(BlueprintCallable, Category = "Save|Interim")
	void SetMoney(int32 InMoney);

	UFUNCTION(BlueprintPure, Category = "Save|Interim")
	int32 GetMoney() const;

	/** Police attention, only grows; persisted as the core's heat. */
	UFUNCTION(BlueprintCallable, Category = "Save|Interim")
	void SetHeat(float InHeat);

	UFUNCTION(BlueprintCallable, Category = "Save|Interim")
	void AddHeat(float Delta);

	UFUNCTION(BlueprintPure, Category = "Save|Interim")
	float GetHeat() const;

private:
	int32 Money = 2000; // SaveState default: balance starting_money
	float Heat = 0.0f;

	/** Lazily fetched: subsystem init order between the two is not fixed. */
	class ULCPhaseManager* PhaseManager() const;
};
