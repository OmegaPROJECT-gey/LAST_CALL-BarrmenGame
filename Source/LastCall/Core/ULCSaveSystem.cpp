// LAST CALL - the save system. See header for the contract.

#include "Core/ULCSaveSystem.h"

#include "Core/BalanceCore.h"
#include "Core/LCPhaseManager.h"
#include "Dialogue/ULCDialogueRunner.h"
#include "Kismet/GameplayStatics.h"
#include "LastCall.h"
#include "Misc/Paths.h"

namespace
{
	/** One slot (PHILOSOPHY 3.2). */
	constexpr const TCHAR* kSlotName = TEXT("lastcall");

	std::string PayloadToUtf8(const FString& Text)
	{
		FTCHARToUTF8 Converter(*Text);
		return std::string(Converter.Get(), Converter.Length());
	}

	/** The interim ledger in the shape the payload core writes. */
	std::vector<std::pair<std::string, double>> StandingRows(
		const TMap<FString, double>& Rows)
	{
		std::vector<std::pair<std::string, double>> Converted;
		Converted.reserve(Rows.Num());
		for (const TPair<FString, double>& Entry : Rows)
		{
			FTCHARToUTF8 Converter(*Entry.Key);
			Converted.emplace_back(std::string(Converter.Get(), Converter.Length()),
				Entry.Value);
		}
		return Converted;
	}
} // namespace

void ULCSaveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Where a run starts comes from the balance file (economy.starting_
	// money); a refusal keeps the compiled default, which the unit tests
	// pin to the same file.
	lc::BalanceData Balance;
	const FString BalancePath = FPaths::Combine(
		FPaths::ProjectDir(), TEXT("Data"), TEXT("Balance"), TEXT("core.json"));
	FTCHARToUTF8 Converter(*BalancePath);
	std::string BalanceError;
	if (lc::load_balance_file(std::string(Converter.Get(), Converter.Length()),
		Balance, &BalanceError))
	{
		Money = Balance.starting_money;
	}
	else
	{
		UE_LOG(LogLastCall, Warning, TEXT("Balance refused (%s) - starting money stays %d"),
			UTF8_TO_TCHAR(BalanceError.c_str()), Money);
	}

	UE_LOG(LogLastCall, Verbose, TEXT("SaveSystem initialized (%s a slot, money %d)"),
		HasSave() ? TEXT("with") : TEXT("without"), Money);
}

bool ULCSaveSystem::WriteSave()
{
	ULCPhaseManager* Clock = PhaseManager();
	if (!Clock)
	{
		UE_LOG(LogLastCall, Warning, TEXT("WriteSave refused: no phase clock"));
		return false;
	}

	lc::SaveState State;
	State.day = Clock->GetDay();
	State.phase = ULCPhaseManager::ToCore(Clock->GetPhase());
	State.money = Money;
	State.heat = static_cast<double>(Heat);
	State.venue_tier = VenueTier;
	State.hospital_days = Clock->GetHospitalDays(); // B1: the stay rides out
	State.standing = StandingRows(Standing);
	if (ULCDialogueRunner* Runner = DialogueRunner())
	{
		State.flags = Runner->StoryFlags(); // C2: the dialogue consequences
	}
	else
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("WriteSave: no dialogue runner - story flags written empty"));
	}

	ULCSaveGame* Save = Cast<ULCSaveGame>(
		UGameplayStatics::CreateSaveGameObject(ULCSaveGame::StaticClass()));
	if (!Save)
	{
		UE_LOG(LogLastCall, Error, TEXT("WriteSave refused: no save object"));
		return false;
	}
	Save->Payload = FString(UTF8_TO_TCHAR(lc::serialize(State).c_str()));

	if (!UGameplayStatics::SaveGameToSlot(Save, kSlotName, 0))
	{
		UE_LOG(LogLastCall, Warning, TEXT("WriteSave failed: slot '%s' not written"), kSlotName);
		return false;
	}
	return true;
}

bool ULCSaveSystem::LoadSave()
{
	if (!UGameplayStatics::DoesSaveGameExist(kSlotName, 0))
	{
		UE_LOG(LogLastCall, Log, TEXT("LoadSave: slot '%s' is empty"), kSlotName);
		return false;
	}

	USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(kSlotName, 0);
	ULCSaveGame* Save = Raw ? Cast<ULCSaveGame>(Raw) : nullptr;
	if (!Save)
	{
		UE_LOG(LogLastCall, Warning, TEXT("LoadSave refused: slot '%s' is not ours"), kSlotName);
		return false;
	}

	lc::SaveState State;
	if (!lc::deserialize(PayloadToUtf8(Save->Payload), State))
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("LoadSave refused: payload failed strict parse (run stays untouched)"));
		return false;
	}

	// Parse first, mutate second: a refused load must change nothing.
	Money = State.money;
	Heat = static_cast<float>(State.heat);
	VenueTier = State.venue_tier;
	Standing.Empty();
	for (const std::pair<std::string, double>& Row : State.standing)
	{
		Standing.Add(FString(UTF8_TO_TCHAR(Row.first.c_str())), Row.second);
	}
	if (ULCDialogueRunner* Runner = DialogueRunner())
	{
		Runner->SetStoryFlags(State.flags); // C2: a load replaces the story
	}
	else
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("LoadSave: no dialogue runner - story flags not applied"));
	}

	if (ULCPhaseManager* Clock = PhaseManager())
	{
		Clock->ApplyLoadedState(State.day, ULCPhaseManager::ToEngine(State.phase),
			State.hospital_days);
	}
	else
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("LoadSave: no phase clock - day/phase not applied, money/heat are"));
	}
	return true;
}

bool ULCSaveSystem::HasSave() const
{
	return UGameplayStatics::DoesSaveGameExist(kSlotName, 0);
}

bool ULCSaveSystem::DeleteSave()
{
	if (!HasSave())
	{
		return false;
	}
	return UGameplayStatics::DeleteGameInSlot(kSlotName, 0);
}

void ULCSaveSystem::SetMoney(int32 InMoney)
{
	Money = InMoney;
}

int32 ULCSaveSystem::GetMoney() const
{
	return Money;
}

void ULCSaveSystem::SetHeat(float InHeat)
{
	Heat = InHeat < 0.0f ? 0.0f : InHeat;
}

void ULCSaveSystem::AddHeat(float Delta)
{
	SetHeat(Heat + Delta);
}

float ULCSaveSystem::GetHeat() const
{
	return Heat;
}

void ULCSaveSystem::SetVenueTier(int32 InVenueTier)
{
	VenueTier = FMath::Clamp(InVenueTier, lc::kMinVenueTier, lc::kMaxVenueTier);
}

int32 ULCSaveSystem::GetVenueTier() const
{
	return VenueTier;
}

void ULCSaveSystem::SetStanding(const TMap<FString, double>& InStanding)
{
	Standing = InStanding;
}

void ULCSaveSystem::SetStandingPoint(const FString& FactionId, double Value)
{
	Standing.Add(FactionId, Value);
}

TMap<FString, double> ULCSaveSystem::GetStanding() const
{
	return Standing;
}

ULCPhaseManager* ULCSaveSystem::PhaseManager() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULCPhaseManager>() : nullptr;
}

ULCDialogueRunner* ULCSaveSystem::DialogueRunner() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULCDialogueRunner>() : nullptr;
}
