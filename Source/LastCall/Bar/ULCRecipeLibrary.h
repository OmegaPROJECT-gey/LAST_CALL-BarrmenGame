// LAST CALL - the recipe library wrapper (ARCHITECTURE.md, section 3).
//
// The engine half of the recipe system: a game-instance subsystem over the
// engine-independent lc::RecipeLibrary - the book the UI lists, the drink
// a customer orders, the wall of fragments the story reads. The pour
// solver gets its RecipeSpec through the core directly (FLCPourSolver);
// this class is the presentation and story layer.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Bar/FLCPourSolver.h"
#include "Bar/RecipeLibraryCore.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ULCRecipeLibrary.generated.h"

/** One fragment of the uncle's wall, as the story layer reads it. */
USTRUCT(BlueprintType)
struct FLCWallFragment
{
	GENERATED_BODY()

	/** 1-based position on the wall. */
	UPROPERTY(BlueprintReadOnly, Category = "Recipes")
	int32 Index = 0;

	/** How many fragments the wall promises in total. */
	UPROPERTY(BlueprintReadOnly, Category = "Recipes")
	int32 Of = 0;

	/** Which recipe on the wall hides this fragment. */
	UPROPERTY(BlueprintReadOnly, Category = "Recipes")
	FString RecipeId;

	/** Localization key of the fragment's text. */
	UPROPERTY(BlueprintReadOnly, Category = "Recipes")
	FString TextKey;

	/** Story flag that unlocks reading it; empty = always readable. */
	UPROPERTY(BlueprintReadOnly, Category = "Recipes")
	FString RevealFlag;
};

UCLASS()
class LASTCALL_API ULCRecipeLibrary : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem: the book is open before the first shift.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Reload Data/Recipes from disk. Refusals (bad JSON, duplicate ids,
	 * a broken wall) are logged; returns the number of loaded recipes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	int32 LoadRecipes();

	/** Every recipe id, in load order - what the book lists. */
	UFUNCTION(BlueprintPure, Category = "Recipes")
	TArray<FString> GetRecipeIds() const;

	UFUNCTION(BlueprintPure, Category = "Recipes")
	bool HasRecipe(const FString& RecipeId) const;

	/** Bilingual display name of a recipe ("" when unknown). */
	UFUNCTION(BlueprintPure, Category = "Recipes")
	FString GetNameEn(const FString& RecipeId) const;

	UFUNCTION(BlueprintPure, Category = "Recipes")
	FString GetNameRu(const FString& RecipeId) const;

	/** Glass id the recipe is built in ("rocks", "martini", ...). */
	UFUNCTION(BlueprintPure, Category = "Recipes")
	FString GetGlass(const FString& RecipeId) const;

	/** How many pour steps the recipe has. */
	UFUNCTION(BlueprintPure, Category = "Recipes")
	int32 GetStepCount(const FString& RecipeId) const;

	/** Effects the order layer pays out; false for an unknown recipe. */
	UFUNCTION(BlueprintPure, Category = "Recipes")
	bool GetEffects(const FString& RecipeId, float& MoodBonus,
		float& TipMultiplier, float& XpMultiplier) const;

	/**
	 * The uncle's wall: every secret fragment sorted by index. The caller
	 * gates each row on RevealFlag (the dialogue world's flags) - this
	 * class only reports what the data promises.
	 */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	TArray<FLCWallFragment> GetWallFragments() const;

	/**
	 * Rate a recorded pour against a recipe: quality from solver samples
	 * only, never from input (PHILOSOPHY.md 3.3). An unknown recipe reads
	 * Ruined with bWrongIngredient - you cannot serve what the book does
	 * not know.
	 */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	FLCComboResult EvaluateCombo(const FString& RecipeId,
		const TArray<FLCPourSample>& Samples) const;

private:
	lc::RecipeLibrary Library;
	std::vector<lc::SecretFragment> Fragments;
};
