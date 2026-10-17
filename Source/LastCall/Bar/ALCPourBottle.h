// LAST CALL - the pour bottle actor (ARCHITECTURE.md, section 4).
//
// The world half of F1's pour: the bottle body the player tilts. It holds
// the Blueprint-serializable FLCPourSolver (the struct "bottle / stream /
// glass actors hold and drive from C++"), steps it at the core's fixed
// 30 Hz and pushes the fill into the glass actor so the rim material
// rises; the normalized flow it reports is what the Niagara stream reads
// when the stream actor lands. The tilt is an ordinary property for now
// - the pawn input binding (D2) drives it when it lands.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Bar/FLCPourSolver.h"
#include "GameFramework/Actor.h"
#include "ALCPourBottle.generated.h"

class ALCPourGlass;

/** The bottle body: tilt it, it pours through the solver into the glass. */
UCLASS()
class LASTCALL_API ALCPourBottle : public AActor
{
	GENERATED_BODY()

public:
	ALCPourBottle();

	// AActor: fixed 30 Hz solver steps over the frame time.
	virtual void TickActor(float DeltaSeconds, ELevelTick TickType,
		FActorTickFunction& ThisTickFunction) override;

	/** The tilt the input drives (degrees; the solver's min/max angle curve). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float TiltDeg = 0.0f;

	/** What the bottle pours (liquids.json id; feeds the combo samples). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	FString Ingredient;

	/** Thick pours run slower (syrups > 1, water-like spirits < 1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float Viscosity = 1.0f;

	/** The glass this bottle pours into; null = the pour only warms the solver. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	TObjectPtr<ALCPourGlass> GlassActor;

	/** The solver the stream and the glass read (combo evaluation runs on it). */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	FLCPourSolver Solver;

	/** Normalized flow 0..1 of the last step - the Niagara rate reads it. */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float CurrentFlow = 0.0f;

	/** Millilitres per second of the last step - the stream's real speed. */
	UFUNCTION(BlueprintPure, Category = "Pour")
	float GetFlowMlPerS() const;

	/** Everything recorded since the last reset - feed to combo evaluation. */
	UFUNCTION(BlueprintPure, Category = "Pour")
	TArray<FLCPourSample> GetSamples() const;

	/** Empty the glass, drop the samples - a new drink starts here. */
	UFUNCTION(BlueprintCallable, Category = "Pour")
	void ResetPour();

	/** Which ingredient the bottle holds now (sample records read it). */
	UFUNCTION(BlueprintCallable, Category = "Pour")
	void SetIngredient(const FString& InIngredient);

private:
	/** One fixed solver step, then hand the new fill to the glass actor. */
	void StepSolver(float StepSeconds);

	float PourAccumulator = 0.0f;
};
