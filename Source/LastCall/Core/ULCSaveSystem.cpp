// LAST CALL - the save system. See header for the contract.

#include "Core/ULCSaveSystem.h"

#include "Core/BalanceCore.h"
#include "Core/LCPhaseManager.h"
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

	if (ULCPhaseManager* Clock = PhaseManager())
	{
		Clock->ApplyLoadedState(State.day, ULCPhaseManager::ToEngine(State.phase));
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

ULCPhaseManager* ULCSaveSystem::PhaseManager() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULCPhaseManager>() : nullptr;
}
