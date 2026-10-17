// LAST CALL - the pour bottle actor implementation. See header for the contract.

#include "Bar/ALCPourBottle.h"

#include "Bar/ALCPourGlass.h"

namespace
{
	// The core integrates at a fixed 30 Hz (PourSolverCore); the actor
	// accumulates frame time and never runs more than five steps in one
	// frame, so a hitch cannot flood the samples.
	constexpr float SolverStepSeconds = 1.0f / 30.0f;
	constexpr int32 MaxStepsPerFrame = 5;
} // namespace

ALCPourBottle::ALCPourBottle()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALCPourBottle::TickActor(float DeltaSeconds, ELevelTick TickType,
	FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaSeconds, TickType, ThisTickFunction);

	PourAccumulator = FMath::Min(PourAccumulator + DeltaSeconds,
		MaxStepsPerFrame * SolverStepSeconds);
	while (PourAccumulator >= SolverStepSeconds)
	{
		StepSolver(SolverStepSeconds);
		PourAccumulator -= SolverStepSeconds;
	}
}

void ALCPourBottle::StepSolver(float StepSeconds)
{
	CurrentFlow = Solver.PourStep(TiltDeg, Viscosity, StepSeconds);

	if (GlassActor)
	{
		// The solver owns the glass state; the actor only mirrors it for
		// the material, the recipe layer and (later) the spill decal.
		GlassActor->ApplyFill(Solver.GetFillFraction(), Solver.GetVolumeMl(),
			Solver.GetSpillMl(), Solver.HasOverflowed());
	}
}

float ALCPourBottle::GetFlowMlPerS() const
{
	return Solver.FlowMlPerS(CurrentFlow);
}

TArray<FLCPourSample> ALCPourBottle::GetSamples() const
{
	return Solver.GetSamples();
}

void ALCPourBottle::ResetPour()
{
	Solver.ResetPour();
	PourAccumulator = 0.0f;
	CurrentFlow = 0.0f;

	if (GlassActor)
	{
		GlassActor->ApplyFill(0.0f, 0.0f, 0.0f, false);
	}
}

void ALCPourBottle::SetIngredient(const FString& InIngredient)
{
	Ingredient = InIngredient;
	Solver.SetCurrentIngredient(InIngredient);
}
