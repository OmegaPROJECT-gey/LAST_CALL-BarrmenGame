// LAST CALL - the pour solver wrapper. See header for the contract.

#include "Bar/FLCPourSolver.h"

lc::PourTuning FLCPourSolver::ToTuning() const
{
	lc::PourTuning Tuning;
	Tuning.min_angle_deg = MinAngleDeg;
	Tuning.max_angle_deg = MaxAngleDeg;
	Tuning.base_flow_ml_per_s = BaseFlowMlPerS;
	Tuning.viscosity_default = ViscosityDefault;
	Tuning.overflow_spill_threshold = OverflowSpillThreshold;
	return Tuning;
}

void FLCPourSolver::ResetPour()
{
	Glass = lc::GlassState{};
	Glass.capacity_ml = CapacityMl;
	Samples.clear();
	CurrentIngredient.clear();
	TimeS = 0.0;
}

float FLCPourSolver::TiltToFlow(float TiltDeg, float Viscosity) const
{
	const lc::PourTuning Tuning = ToTuning();
	const double ViscosityUsed = Viscosity > 0.0f ? Viscosity : Tuning.viscosity_default;
	return static_cast<float>(
		lc::normalized_flow_from_tilt(TiltDeg, ViscosityUsed, Tuning));
}

float FLCPourSolver::FlowMlPerS(float NormalizedFlow) const
{
	return static_cast<float>(lc::flow_ml_per_s(NormalizedFlow, ToTuning()));
}

float FLCPourSolver::PourStep(float TiltDeg, float Viscosity, float DtSeconds)
{
	const lc::PourTuning Tuning = ToTuning();
	const double ViscosityUsed = Viscosity > 0.0f ? Viscosity : Tuning.viscosity_default;
	const double Flow = lc::normalized_flow_from_tilt(TiltDeg, ViscosityUsed, Tuning);

	const double Dt = DtSeconds > 0.0f ? DtSeconds : 0.0;
	lc::integrate(Glass, Flow, Dt, Tuning);

	lc::PourSample Sample;
	Sample.time_s = TimeS;
	Sample.flow = Flow;
	Sample.ingredient = CurrentIngredient;
	Samples.push_back(std::move(Sample));
	TimeS += Dt;

	return static_cast<float>(Flow);
}

void FLCPourSolver::SetCurrentIngredient(const FString& Ingredient)
{
	FTCHARToUTF8 Converter(*Ingredient);
	CurrentIngredient.assign(Converter.Get(), Converter.Length());
}

void FLCPourSolver::ClearSamples()
{
	Samples.clear();
	TimeS = 0.0;
}

float FLCPourSolver::GetVolumeMl() const
{
	return static_cast<float>(Glass.volume_ml);
}

float FLCPourSolver::GetSpillMl() const
{
	return static_cast<float>(Glass.spill_ml);
}

bool FLCPourSolver::HasOverflowed() const
{
	return Glass.overflowed;
}

float FLCPourSolver::GetFillFraction() const
{
	if (Glass.capacity_ml <= 0.0)
	{
		return 0.0f;
	}
	const double Fraction = Glass.volume_ml / Glass.capacity_ml;
	if (Fraction < 0.0)
	{
		return 0.0f;
	}
	return static_cast<float>(Fraction > 1.0 ? 1.0 : Fraction);
}

TArray<FLCPourSample> FLCPourSolver::GetSamples() const
{
	TArray<FLCPourSample> Result;
	Result.Reserve(static_cast<int32>(Samples.size()));
	for (const lc::PourSample& Sample : Samples)
	{
		FLCPourSample Row;
		Row.TimeS = static_cast<float>(Sample.time_s);
		Row.Flow = static_cast<float>(Sample.flow);
		Row.Ingredient = FString(UTF8_TO_TCHAR(Sample.ingredient.c_str()));
		Result.Add(Row);
	}
	return Result;
}
