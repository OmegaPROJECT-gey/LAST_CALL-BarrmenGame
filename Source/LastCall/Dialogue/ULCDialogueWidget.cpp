// LAST CALL - the dialogue widget implementation. See header for the contract.

#include "Dialogue/ULCDialogueWidget.h"

#include "Core/ULCLocalizationSystem.h"
#include "Dialogue/ULCDialogueRunner.h"
#include "Engine/GameInstance.h"

ULCDialogueRunner* ULCDialogueWidget::Runner() const
{
	if (!CachedRunner)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			CachedRunner = GameInstance->GetSubsystem<ULCDialogueRunner>();
		}
	}
	return CachedRunner;
}

bool ULCDialogueWidget::StartTalk(const FString& SceneId)
{
	ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr || !RunnerPtr->StartScene(SceneId))
	{
		return false;
	}

	bFinishedBroadcast = false;
	RefreshNode();
	return true;
}

bool ULCDialogueWidget::Reply(const FString& ChoiceId)
{
	ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr || !RunnerPtr->Choose(ChoiceId))
	{
		return false;
	}
	RefreshNode();
	return true;
}

bool ULCDialogueWidget::ContinueTalk()
{
	ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr || !RunnerPtr->Advance())
	{
		return false;
	}
	RefreshNode();
	return true;
}

FText ULCDialogueWidget::GetCurrentText() const
{
	const ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr)
	{
		return FText::GetEmpty();
	}

	const ULCLocalizationSystem* Localization = nullptr;
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		Localization = GameInstance->GetSubsystem<ULCLocalizationSystem>();
	}
	return Localization
		? Localization->GetText(RunnerPtr->GetCurrentTextKey())
		: FText::GetEmpty();
}

FText ULCDialogueWidget::GetSpeaker() const
{
	const ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr)
	{
		return FText::GetEmpty();
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const ULCLocalizationSystem* Localization =
		GameInstance ? GameInstance->GetSubsystem<ULCLocalizationSystem>() : nullptr;
	return Localization
		? Localization->GetText(RunnerPtr->GetCurrentSpeaker())
		: FText::GetEmpty();
}

TArray<FLCDialogueOption> ULCDialogueWidget::GetChoices() const
{
	TArray<FLCDialogueOption> Options;
	ULCDialogueRunner* RunnerPtr = Runner();
	if (!RunnerPtr)
	{
		return Options;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const ULCLocalizationSystem* Localization =
		GameInstance ? GameInstance->GetSubsystem<ULCLocalizationSystem>() : nullptr;

	const TArray<FLCDialogueChoice> Choices = RunnerPtr->GetAvailableChoices();
	Options.Reserve(Choices.Num());
	for (const FLCDialogueChoice& Choice : Choices)
	{
		FLCDialogueOption Option;
		Option.Id = Choice.Id;
		Option.Text = Localization
			? Localization->GetText(Choice.TextKey)
			: FText::GetEmpty();
		Options.Add(MoveTemp(Option));
	}
	return Options;
}

bool ULCDialogueWidget::IsAtEnd() const
{
	const ULCDialogueRunner* RunnerPtr = Runner();
	return RunnerPtr ? RunnerPtr->IsAtEnd() : false;
}

void ULCDialogueWidget::RefreshNode()
{
	ULCDialogueRunner* RunnerPtr = Runner();
	if (RunnerPtr && !bFinishedBroadcast && RunnerPtr->IsAtEnd())
	{
		bFinishedBroadcast = true;
		OnDialogueFinished.Broadcast();
	}
}
