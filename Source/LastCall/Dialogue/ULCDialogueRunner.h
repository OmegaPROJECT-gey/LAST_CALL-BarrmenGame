// LAST CALL - the dialogue runner wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of the dialogue system: a game-instance subsystem that
// drives the engine-independent lc::DialogueRunner over the scene list in
// Data/Dialogue/*.json. UI widgets bind to the current text key and the
// available choices; the game listens to OnDialogueEffect and routes mood
// to the customer, quests to the quest tracker, heat to the heat meter and
// items to the inventory (the core routes, the engine decides where).
//
// World state (flags, standing, skills, held items) lives here until the
// real systems wrap their cores (ConsequenceLog, StandingLedger, the save):
// F2/C2 UE work binds those in, the data contract does not change.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Dialogue/DialogueRunnerCore.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LCPhaseManager.h"
#include "ULCDialogueRunner.generated.h"

/** One choice the current node offers the player (a localization key, not text). */
USTRUCT(BlueprintType)
struct FLCDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString TextKey;
};

/**
 * A routed effect the engine must act on: mood -> the customer, quest ->
 * the tracker, heat -> the heat meter, item -> the inventory.
 */
USTRUCT(BlueprintType)
struct FLCDialogueEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString Type; // "mood" | "quest" | "heat" | "item"

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString Target; // customer id, quest id or item id

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString State; // quest state ("started", ...), empty otherwise

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	float Delta = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLCDialogueEffect, const FLCDialogueEffect&, Effect);

UCLASS()
class LASTCALL_API ULCDialogueRunner : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Validate the whole scene list with the core's load_directory - every
	 * tree in Data/Dialogue must parse or the refusal is logged. Returns
	 * the number of loaded trees (0 when the directory is missing).
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	int32 LoadScenes();

	/** Scene ids in Data/Dialogue, sorted - what the talk menu lists. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FString> GetSceneIds() const;

	/**
	 * Load a scene and start it: returns false for an unknown id, a
	 * refusal (logged) or a world the scene's entry conditions reject
	 * (the caller then simply does not offer this talk).
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartScene(const FString& SceneId);

	/** Current node. Empty keys when no scene runs. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FString GetCurrentTextKey() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FString GetCurrentSpeaker() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsAtEnd() const;

	/** Choices the world currently allows (conditions already applied). */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FLCDialogueChoice> GetAvailableChoices();

	/** Follow the linear `next`. False at an end node or without one. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool Advance();

	/** Take a choice by id. False when unknown or filtered out by conditions. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool Choose(const FString& ChoiceId);

	/** The scene's interrupt recording (empty when it has none). */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FString GetInterruptFlagIgnored() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FString GetInterruptFlagHeard() const;

	// --- interim world state: replaced by the real systems as F2/C2 land ---

	UFUNCTION(BlueprintCallable, Category = "Dialogue|World")
	void SetStoryFlag(const FString& Flag, bool bValue);

	UFUNCTION(BlueprintPure, Category = "Dialogue|World")
	bool HasStoryFlag(const FString& Flag) const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue|World")
	void AddFactionStanding(const FString& Faction, float Delta);

	UFUNCTION(BlueprintPure, Category = "Dialogue|World")
	float GetFactionStanding(const FString& Faction) const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue|World")
	void GrantSkill(const FString& SkillId);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|World")
	void GrantItem(const FString& ItemId);

	/** Fired for every routed effect produced by Start/Advance/Choose. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnLCDialogueEffect OnDialogueEffect;

private:
	/** The core: one active scene at a time, exactly as the tests drive it. */
	lc::DialogueRunner Runner;
	lc::DialogueWorld World;

	/** How many routed effects were already broadcast (drain window). */
	std::size_t EffectsSeen = 0;

	/** Broadcast the effects produced since the last drain. */
	void DrainEffects();

	/** ULCPhaseManager -> world.phase ("Morning", ...). */
	UFUNCTION()
	void HandlePhaseChanged(ELCPhase OldPhase, ELCPhase NewPhase);

	static FString PhaseName(ELCPhase InPhase);

	/** Scene ids are `[a-z0-9_]+` (the dialogue schema) - refuse anything else. */
	static bool IsSafeSceneId(const FString& SceneId);
};
