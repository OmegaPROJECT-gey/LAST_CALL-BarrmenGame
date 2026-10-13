// LAST CALL - customer reaction core (ARCHITECTURE.md, section 4).
//
// The second half of the F1 gate: "customer reacts". Engine-independent
// C++17 like the pour solver, so patience, shock and walk-out logic can be
// unit-tested without an install of Unreal. The UE-facing wrapper binds it
// to widgets, barks and animations once the engine is available.
//
// License: MIT (LICENSE).

#pragma once

#include "Bar/PourSolverCore.h"

namespace lc {

	// Tuning mirrors the "customer" section of Data/Balance/core.json;
	// the test suite asserts the two stay in sync.
	struct CustomerTuning
	{
		double drain_idle_per_s = 0.02;        // patience_drain_per_second_idle
		double drain_waiting_per_s = 0.05;     // patience_drain_per_second_waiting
		double refill_correct_order = 0.15;    // patience_refill_on_correct_order
		double ruined_penalty = 0.5;           // patience_penalty_ruined
		double bonus_critical = 0.3;           // mood_bonus_on_critical_combo
		double spill_patience_per_ml = 0.02;   // spill_patience_per_ml
		double burst_walkout_spill_ml = 120.0; // burst_walkout_spill_ml
		double shock_s = 3.0;                  // shock_seconds
		double band_pleased = 0.75;            // band_pleased
		double band_annoyed = 0.4;             // band_annoyed
	};

	struct CustomerState
	{
		double patience = 1.0; // 1.0 fresh, 0.0 gone
		double shock_s = 0.0;  // time left of the post-spill shock
		bool walked_out = false;
	};

	enum class Reaction
	{
		Content,   // patience high, nothing wrong
		Neutral,   // waiting, patience middling
		Annoyed,   // patience low, still standing
		Shocked,   // hit by a spill, reeling (shock_s > 0)
		WalkedOut  // patience hit zero, or burst spill
	};

	// A spill the customer watched happen: burn patience proportional to the
	// volume, enter the shock window; a burst spill ends the visit outright.
	void on_spill(CustomerState& customer, double spill_ml, const CustomerTuning& tuning);

	// A drink landed in front of them: quality moves patience
	// (GAME-DESIGN 3.4). Ruined at low patience walks the customer out.
	void on_serve(CustomerState& customer, ComboQuality quality, const CustomerTuning& tuning);

	// Patience delta for a combo quality - shared with the UI copy so
	// "Critical combo +30%" never lies about the numbers.
	double satisfaction_for(ComboQuality quality, const CustomerTuning& tuning);

	// Patience drains while they wait (waiting flag selects the harsher
	// drain of a full queue); the shock window decays.
	void tick(CustomerState& customer, double dt_s, bool waiting_in_queue,
		const CustomerTuning& tuning);

	// What the customer shows right now (drives the widget, bark and pose).
	Reaction reaction_for(const CustomerState& customer, const CustomerTuning& tuning);

} // namespace lc
