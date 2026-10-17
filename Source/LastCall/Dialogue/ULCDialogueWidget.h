// LAST CALL - the dialogue widget (ARCHITECTURE.md, section 3; milestone F3).
//
// The C++ half of the talk screen: it drives ULCDialogueRunner and hands
// the UI what to show - the current line and the choices, already
// localized through ULCLocalizationSystem (Data/Localization, EN/RU, the
// player's language). The widget is data and flow only: the visual
// layout (portraits, text styles, choice buttons) is the Blueprint
// subclass in the editor, so this class stays engine-verifiable from C++
// and the content work lands separately. Voice binding rides on top when
// the barks/voice content lands (F1/C2 open items).
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ULCDialogueWidget.generated.h"

class ULCDialogueRunner;

/** One choice as the screen shows it: the id to take, the text to show. */
USTRUCT(BlueprintType)
struct FLCDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Text;
};

/** The scene reached its end node - the talk screen may close itself. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLCDialogueFinished);

/**
 * The talk screen's data layer. StartTalk opens a scene, Reply/Continue
 * walk it, and GetText/GetChoices/IsAtEnd feed the layout; every mutation
 * refreshes the exposed state once and a finished scene broadcasts
 * OnDialogueFinished exactly once until the next StartTalk.
 */
UCLASS()
class LASTCALL_API ULCDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Open a scene from the talk menu. False for unknown/rejected scenes. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartTalk(const FString& SceneId);

	/** Take a choice by id. False when unknown or filtered by conditions. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool Reply(const FString& ChoiceId);

	/** Follow the linear `next`. False at an end node or without one. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool ContinueTalk();

	/** The current line, localized ("" before StartTalk). */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FText GetCurrentText() const;

	/** The speaker line ("who talks"), localized as the data carries it. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FText GetSpeaker() const;

	/** The choices the world currently allows, localized. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	TArray<FLCDialogueOption> GetChoices() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsAtEnd() const;

	/** Scene over (end node) - fired once per talk. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnLCDialogueFinished OnDialogueFinished;

private:
	/** Runner through the game instance (null before the world exists). */
	ULCDialogueRunner* Runner() const;

	/** Walk happened: re-read the node and fire the end once. */
	void RefreshNode();

	mutable ULCDialogueRunner* CachedRunner = nullptr;
	bool bFinishedBroadcast = false;
};
