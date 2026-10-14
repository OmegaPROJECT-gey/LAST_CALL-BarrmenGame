// LAST CALL - the dialogue runner wrapper. See header for the contract.

#include "Dialogue/ULCDialogueRunner.h"

#include "HAL/FileManager.h"
#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	/** FString -> the cores' UTF-8 std::string. */
	std::string ToUtf8(const FString& Text)
	{
		FTCHARToUTF8 Converter(*Text);
		return std::string(Converter.Get(), Converter.Length());
	}

	FString FromUtf8(const std::string& Text)
	{
		return FString(UTF8_TO_TCHAR(Text.c_str()));
	}

	FString DialogueDirectory()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Dialogue"));
	}
} // namespace

void ULCDialogueRunner::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// The phase clock owns the day; the world's `phase` conditions read it.
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULCPhaseManager* PhaseManager = GameInstance->GetSubsystem<ULCPhaseManager>())
		{
			World.phase = ToUtf8(PhaseName(PhaseManager->GetPhase()));
			PhaseManager->OnPhaseChanged.AddDynamic(this, &ULCDialogueRunner::HandlePhaseChanged);
			UE_LOG(LogLastCall, Verbose, TEXT("DialogueRunner bound to the phase clock (%s)"),
				*PhaseName(PhaseManager->GetPhase()));
		}
	}
}

int32 ULCDialogueRunner::LoadScenes()
{
	std::vector<std::string> Errors;
	const int Loaded = Runner.load_directory(ToUtf8(DialogueDirectory()), &Errors);
	for (const std::string& Error : Errors)
	{
		UE_LOG(LogLastCall, Warning, TEXT("Dialogue scene refused: %s"),
			*FromUtf8(Error));
	}
	UE_LOG(LogLastCall, Log, TEXT("Dialogue scene list: %d trees loaded, %d refused"),
		Loaded, static_cast<int32>(Errors.size()));
	return Loaded;
}

TArray<FString> ULCDialogueRunner::GetSceneIds() const
{
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(DialogueDirectory(), TEXT("*.json")),
		true, false);

	TArray<FString> Ids;
	for (const FString& File : Files)
	{
		const FString Id = FPaths::GetBaseFilename(File);
		if (IsSafeSceneId(Id))
		{
			Ids.Add(Id);
		}
	}
	Ids.Sort();
	return Ids;
}

bool ULCDialogueRunner::StartScene(const FString& SceneId)
{
	if (!IsSafeSceneId(SceneId))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Refusing scene id '%s' (expected [a-z0-9_])"),
			*SceneId);
		return false;
	}

	FString Text;
	const FString Path = FPaths::Combine(DialogueDirectory(), SceneId + TEXT(".json"));
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Scene '%s' not found at %s"), *SceneId, *Path);
		return false;
	}

	std::string Error;
	if (!Runner.load(ToUtf8(Text), &Error))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Scene '%s' refused: %s"), *SceneId,
			*FromUtf8(Error));
		return false;
	}

	EffectsSeen = 0;
	if (!Runner.start(World))
	{
		// Entry conditions reject this talk for the current world - the
		// caller simply does not offer it. Not an error.
		UE_LOG(LogLastCall, Log, TEXT("Scene '%s' not available in the current world"),
			*SceneId);
		return false;
	}

	DrainEffects();
	UE_LOG(LogLastCall, Log, TEXT("Scene '%s' started (%s)"), *SceneId,
		*FromUtf8(Runner.current_speaker()));
	return true;
}

FString ULCDialogueRunner::GetCurrentTextKey() const
{
	return FromUtf8(Runner.current_text_key());
}

FString ULCDialogueRunner::GetCurrentSpeaker() const
{
	return FromUtf8(Runner.current_speaker());
}

bool ULCDialogueRunner::IsAtEnd() const
{
	return Runner.at_end();
}

TArray<FLCDialogueChoice> ULCDialogueRunner::GetAvailableChoices()
{
	TArray<FLCDialogueChoice> Result;
	for (const int Index : Runner.available_choices(World))
	{
		const lc::DialogueChoiceData* Data = Runner.choice(Index);
		if (!Data)
		{
			continue; // cannot happen: the index came from available_choices
		}
		FLCDialogueChoice Choice;
		Choice.Id = FromUtf8(Data->id);
		Choice.TextKey = FromUtf8(Data->text_key);
		Result.Add(Choice);
	}
	return Result;
}

bool ULCDialogueRunner::Advance()
{
	if (!Runner.advance(World))
	{
		return false;
	}
	DrainEffects();
	return true;
}

bool ULCDialogueRunner::Choose(const FString& ChoiceId)
{
	if (!Runner.choose(ToUtf8(ChoiceId), World))
	{
		return false;
	}
	DrainEffects();
	return true;
}

FString ULCDialogueRunner::GetInterruptFlagIgnored() const
{
	return FromUtf8(Runner.interrupt_flag_ignored());
}

FString ULCDialogueRunner::GetInterruptFlagHeard() const
{
	return FromUtf8(Runner.interrupt_flag_heard());
}

void ULCDialogueRunner::SetStoryFlag(const FString& Flag, bool bValue)
{
	World.set_flag(ToUtf8(Flag), bValue);
}

bool ULCDialogueRunner::HasStoryFlag(const FString& Flag) const
{
	return World.has_flag(ToUtf8(Flag));
}

void ULCDialogueRunner::AddFactionStanding(const FString& Faction, float Delta)
{
	World.add_standing(ToUtf8(Faction), static_cast<double>(Delta));
}

float ULCDialogueRunner::GetFactionStanding(const FString& Faction) const
{
	return static_cast<float>(World.standing_of(ToUtf8(Faction)));
}

void ULCDialogueRunner::GrantSkill(const FString& SkillId)
{
	World.skills.push_back(ToUtf8(SkillId));
}

void ULCDialogueRunner::GrantItem(const FString& ItemId)
{
	World.items.push_back(ToUtf8(ItemId));
}

void ULCDialogueRunner::DrainEffects()
{
	const std::vector<lc::AppliedEffect>& All = Runner.applied();
	for (; EffectsSeen < All.size(); ++EffectsSeen)
	{
		const lc::AppliedEffect& Effect = All[EffectsSeen];
		FLCDialogueEffect Payload;
		Payload.Type = FromUtf8(Effect.type);
		Payload.Target = FromUtf8(Effect.target);
		Payload.State = FromUtf8(Effect.state);
		Payload.Delta = static_cast<float>(Effect.delta);
		OnDialogueEffect.Broadcast(Payload);
	}
}

void ULCDialogueRunner::HandlePhaseChanged(ELCPhase /*OldPhase*/, ELCPhase NewPhase)
{
	World.phase = ToUtf8(PhaseName(NewPhase));
	UE_LOG(LogLastCall, Verbose, TEXT("Dialogue world phase -> %s"), *PhaseName(NewPhase));
}

FString ULCDialogueRunner::PhaseName(ELCPhase InPhase)
{
	switch (InPhase)
	{
	case ELCPhase::Morning: return TEXT("Morning");
	case ELCPhase::Shift:   return TEXT("Shift");
	case ELCPhase::Night:   return TEXT("Night");
	case ELCPhase::Summary: return TEXT("Summary");
	default:                return TEXT("Morning");
	}
}

bool ULCDialogueRunner::IsSafeSceneId(const FString& SceneId)
{
	if (SceneId.IsEmpty())
	{
		return false;
	}
	for (const TCHAR Character : SceneId)
	{
		const bool bLower = Character >= TCHAR('a') && Character <= TCHAR('z');
		const bool bDigit = Character >= TCHAR('0') && Character <= TCHAR('9');
		if (!bLower && !bDigit && Character != TCHAR('_'))
		{
			return false;
		}
	}
	return true;
}
