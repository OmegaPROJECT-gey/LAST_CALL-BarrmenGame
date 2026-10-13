// LAST CALL - default game mode for the bar map.
// Thin by design: the day loop lives in ULCPhaseManager (a GameInstance
// subsystem), not in the game mode, so a phase swap never restarts actors.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LCBarGameMode.generated.h"

UCLASS()
class LASTCALL_API ALCBarGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALCBarGameMode();

	virtual void BeginPlay() override;
};
