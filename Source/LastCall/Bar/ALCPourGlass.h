// LAST CALL - the pour glass actor (ARCHITECTURE.md, section 4).
//
// The receiving half of the pour: the glass body on the bar top. The
// bottle actor pushes each solver step's fill here (the solver owns the
// glass's volume, overflow and spill); this class keeps the numbers the
// liquid material and the UI read - fill 0..1 for the rim material,
// millilitres for the recipe layer, spill for the decal and the customer
// shock when they land.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCPourGlass.generated.h"

/** The glass body: what the pour solver decided it holds, ready for the material. */
UCLASS()
class LASTCALL_API ALCPourGlass : public AActor
{
	GENERATED_BODY()

public:
	/** Fill 0..1 - the liquid/rise material reads this every frame. */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float FillFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float VolumeMl = 0.0f;

	/** Total spill since the pour started - the decal and the shock read it. */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float SpillMl = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	bool bOverflowed = false;

	/** Push the solver's view of the glass in (the bottle actor calls this). */
	UFUNCTION(BlueprintCallable, Category = "Pour")
	void ApplyFill(float InFillFraction, float InVolumeMl, float InSpillMl,
		bool bInOverflowed);

	/** Back to an empty glass on the bar (the next drink starts here). */
	UFUNCTION(BlueprintCallable, Category = "Pour")
	void ClearGlass();
};
