// LAST CALL - the pour solver wrapper (ARCHITECTURE.md, section 4).
//
// The UE-facing half of the pour: a Blueprint-serializable struct the
// bottle / stream / glass actors hold and drive from C++. Physics and
// combo quality stay in the engine-independent core (PourSolverCore -
// unit-tested without Unreal); this struct only translates tilt input and
// dt into the core's state and hands the samples back for evaluation.
// The struct's methods are plain C++ on purpose - UHT does not allow
// UFUNCTION in a struct; the pour actor/component (F1) exposes them to
// Blueprint when it lands. Niagara reads the flow values for the stream;
// the glass reads volume and spill for the fill material.
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Bar/PourSolverCore.h"
#include "FLCPourSolver.generated.h"

/** Combo quality as the UI reads it (GAME-DESIGN.md 3.4). */
UENUM(BlueprintType)
enum class ELCComboQuality : uint8
{
	Ruined,   // wrong drink, or speed outside the window
	Fine,     // served, but the combo did not land
	Good,     // right sequence, right speed
	Critical  // right sequence, right speed, held steady
};

/** One recorded pour sample (solver output - quality reads these only). */
USTRUCT(BlueprintType)
struct FLCPourSample
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float TimeS = 0.0f;

	/** Normalized flow 0..1 at recording time. */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float Flow = 0.0f;

	/** What the bottle held at that moment (ingredient id). */
	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	FString Ingredient;
};

/** The recipe book's view of one pour step, for combo evaluation. */
USTRUCT(BlueprintType)
struct FLCPourStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	FString Ingredient;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float FlowMin = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float FlowMax = 1.0f;
};

/** Why the pour landed or did not (the serve screen shows this). */
USTRUCT(BlueprintType)
struct FLCComboResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	ELCComboQuality Quality = ELCComboQuality::Ruined;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	int32 StepsMatched = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	bool bContinuityBroken = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	bool bWrongIngredient = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pour")
	float MeanFlowError = 0.0f;
};

USTRUCT(BlueprintType)
struct FLCPourSolver
{
	GENERATED_BODY()

	// --- tuning: mirrors Data/Balance/core.json ("pour"); the unit tests
	// assert code and data stay in sync. The balance file binds here when
	// the engine-side loader lands (F2); until then the defaults are it.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float MinAngleDeg = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float MaxAngleDeg = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float BaseFlowMlPerS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float ViscosityDefault = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float OverflowSpillThreshold = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pour")
	float CapacityMl = 150.0f;

	/** Empty the glass, drop the samples, keep tuning and capacity. */
	void ResetPour();

	/** Tilt -> normalized flow 0..1 (the stick/mouse curve). */
	float TiltToFlow(float TiltDeg, float Viscosity) const;

	/** Normalized flow -> millilitres per second (Niagara rate). */
	float FlowMlPerS(float NormalizedFlow) const;

	/**
	 * One fixed step: integrate the glass, record a sample, advance time.
	 * Returns the normalized flow - what the stream particle rate reads.
	 * `Viscosity` <= 0 uses the tuning default.
	 */
	float PourStep(float TiltDeg, float Viscosity, float DtSeconds);

	/** What the bottle currently holds (sample records read it). */
	void SetCurrentIngredient(const FString& Ingredient);

	void ClearSamples();

	float GetVolumeMl() const;

	float GetSpillMl() const;

	bool HasOverflowed() const;

	/** Fill 0..1 - the glass rim material reads this. */
	float GetFillFraction() const;

	/** Everything recorded since Reset/Clear - feed to EvaluateCombo. */
	TArray<FLCPourSample> GetSamples() const;

	// The core state stays outside UPROPERTY: it is transient solver data,
	// rebuilt from the tuning fields on every step.
private:
	lc::PourTuning ToTuning() const;

	lc::GlassState Glass;
	std::vector<lc::PourSample> Samples;
	std::string CurrentIngredient;
	double TimeS = 0.0;
};
