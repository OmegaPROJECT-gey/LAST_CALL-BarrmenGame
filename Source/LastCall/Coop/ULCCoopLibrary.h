// LAST CALL - the co-op roles wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of D2: a game-instance subsystem over the
// engine-independent lc::RoleLibrary - what a lobby or pause screen
// reads about the crew. The rules (bounds 2-4, one role per player,
// someone must pour, the job split, crowd against the venue ladder)
// stay in Coop/CoopCore where scripts/test-coop.sh checks them without
// Unreal; this class loads Data/Coop from disk and hands Blueprint the
// crew's rows, the gate on every input binding and the load arithmetic.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Coop/CoopCore.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCCoopLibrary.generated.h"

/** One role of the crew as the lobby reads it. */
USTRUCT(BlueprintType)
struct FLCRoleInfo
{
	GENERATED_BODY()

	/** Role id ("bartender", "waiter", "bouncer", "dj"). */
	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	FString Id;

	/** Bilingual role name, straight from Data/Coop. */
	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	FString NameEn;

	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	FString NameRu;

	/** Bilingual job line - what this player works. */
	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	FString JobEn;

	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	FString JobRu;

	/** The role's actions (pour, seat, door, ...) - the input gate. */
	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	TArray<FString> Actions;

	/** Bodies this role keeps up with; the crew's crowd load sums them. */
	UPROPERTY(BlueprintReadOnly, Category = "Coop")
	int32 Crowd = 0;
};

UCLASS()
class LASTCALL_API ULCCoopLibrary : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: the crew is known before the lobby asks.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Reload Data/Coop/roles.json from disk. Refusals (bad JSON, a
	 * duplicate id, an action outside the vocabulary) are logged;
	 * returns the number of loaded roles (0 when refused).
	 */
	UFUNCTION(BlueprintCallable, Category = "Coop")
	int32 LoadRoles();

	/** All role rows, in load order, for the lobby list. */
	UFUNCTION(BlueprintPure, Category = "Coop")
	TArray<FLCRoleInfo> GetRoles() const;

	UFUNCTION(BlueprintPure, Category = "Coop")
	TArray<FString> GetRoleIds() const;

	/**
	 * The input gate: does this role work this action? Unknown role
	 * or action reads false - do not bind.
	 */
	UFUNCTION(BlueprintPure, Category = "Coop")
	bool CanRoleDo(const FString& RoleId, const FString& Action) const;

	/**
	 * Is a session of `PlayerCount` players taking exactly `RoleIds`
	 * legal? Returns "" when legal, else the refusal (bounds, one
	 * role per player, duplicates, unknown roles, nobody to pour).
	 */
	UFUNCTION(BlueprintPure, Category = "Coop")
	FString AssignBlocker(int32 PlayerCount, const TArray<FString>& RoleIds) const;

	/** Bodies this crew keeps up with; an illegal crew reads 0. */
	UFUNCTION(BlueprintPure, Category = "Coop")
	int32 CrowdCapacity(const TArray<FString>& RoleIds) const;

	/**
	 * crowd / capacity: 0 idle, 1.0 at the crew's limit, above it
	 * overrun - the engine scales mood and the room's music on it.
	 */
	UFUNCTION(BlueprintPure, Category = "Coop")
	double CrowdLoad(int32 Crowd, const TArray<FString>& RoleIds) const;

	/** Co-op bounds (Data/Coop players): GDD 5.5 says 2-4. */
	UFUNCTION(BlueprintPure, Category = "Coop")
	int32 GetMinPlayers() const;

	UFUNCTION(BlueprintPure, Category = "Coop")
	int32 GetMaxPlayers() const;

private:
	lc::RoleLibrary Library;
};
