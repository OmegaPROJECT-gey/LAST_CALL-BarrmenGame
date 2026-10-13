#include "LCBarGameMode.h"

#include "Core/LCPhaseManager.h"
#include "GameFramework/GameInstance.h"
#include "LastCall.h"

ALCBarGameMode::ALCBarGameMode()
{
	// The bar is a place, not a shooter: no default weapon, no crosshair.
	bStartPlayersAsSpectator = false;
}

void ALCBarGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULCPhaseManager* Phases = GameInstance->GetSubsystem<ULCPhaseManager>())
		{
			UE_LOG(LogLastCall, Log, TEXT("Bar loaded on day %d, phase %d"),
				Phases->GetDay(), static_cast<int32>(Phases->GetPhase()));
		}
	}
}
