// LAST CALL - pour solver core (ARCHITECTURE.md, section 4).
//
// This file is deliberately free of Unreal types: it is plain C++17 so the
// physics of pouring can be unit-tested without an engine install
// (tools/tests/pour, scripts/test-pour-solver.sh). The UE-facing wrapper
// (FLCPourSolver) binds this core to actors and Niagara in milestone F1.
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	// Tuning mirrors Data/Balance/core.json ("pour"); the test suite asserts
	// the two stay in sync.
	struct PourTuning
	{
		double min_angle_deg = 10.0;      // below this the bottle pours nothing
		double max_angle_deg = 70.0;      // saturates: tilting further adds nothing
		double base_flow_ml_per_s = 60.0; // at full tilt, viscosity 1
		double viscosity_default = 1.0;   // syrups > 1, water-like spirits < 1
		double overflow_spill_threshold = 1.0; // fraction of capacity that spills
	};

	// Tilt -> normalized flow in [0,1]: zero below min_angle, smooth ramp to
	// saturation at max_angle, divided by viscosity (thick pours slower).
	// This is the curve the analog input (stick / mouse drag) drives.
	double normalized_flow_from_tilt(double tilt_deg, double viscosity, const PourTuning& tuning);

	// Normalized flow -> millilitres per second.
	double flow_ml_per_s(double normalized_flow, const PourTuning& tuning);

	// What a glass knows while it is being filled (ARCHITECTURE 4: layered
	// volume, overflow, spill).
	struct GlassState
	{
		double capacity_ml = 150.0;
		double volume_ml = 0.0;
		double spill_ml = 0.0;
		bool overflowed = false;
	};

	// One fixed step of the solver (30 Hz with interpolation in the real game).
	void integrate(GlassState& glass, double normalized_flow, double dt_s, const PourTuning& tuning);

	// --- Cocktail-combo evaluation (GAME-DESIGN.md 3.4) ---------------------

	enum class ComboQuality
	{
		Ruined,   // wrong drink, or speed completely outside the window
		Fine,     // served, but the combo did not land
		Good,     // right sequence, right speed
		Critical  // right sequence, right speed, held steady
	};

	struct MixStep
	{
		std::string ingredient;
		double flow_min = 0.0;
		double flow_max = 1.0;
	};

	struct RecipeSpec
	{
		std::string id;
		std::vector<MixStep> steps;
		double continuity_break_s = 4.0; // a pause longer than this breaks the combo
	};

	struct PourSample
	{
		double time_s = 0.0;
		double flow = 0.0;        // normalized 0..1, from the solver
		std::string ingredient;   // what the bottle currently holds
	};

	struct ComboResult
	{
		ComboQuality quality = ComboQuality::Ruined;
		int steps_matched = 0;
		bool continuity_broken = false;
		bool wrong_ingredient = false;
		double mean_flow_error = 0.0; // average distance outside the flow window
	};

	// Quality is computed from solver output only - never from button presses
	// (PHILOSOPHY.md 3.3).
	ComboResult evaluate_combo(const RecipeSpec& recipe, const std::vector<PourSample>& samples);

} // namespace lc
