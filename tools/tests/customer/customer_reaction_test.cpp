// LAST CALL - customer reaction core unit tests.
//
// Runs without Unreal: scripts/test-customer-reaction.sh compiles this
// against Source/LastCall/Customers/CustomerReactionCore.cpp with g++.

#include "Bar/PourSolverCore.h"
#include "Customers/CustomerReactionCore.h"

#include "test_support.h"

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	lc::Reaction patience_reaction(double patience)
	{
		lc::CustomerState customer;
		customer.patience = patience;
		return lc::reaction_for(customer, lc::CustomerTuning{});
	}

} // namespace

int main()
{
	const lc::CustomerTuning tuning;

	// --- resting state and bands -------------------------------------------
	{
		lc::CustomerState customer;
		expect_near(customer.patience, 1.0, 1e-9, "fresh customer starts at full patience");
		expect(lc::reaction_for(customer, tuning) == lc::Reaction::Content,
			"fresh customer is content");
	}
	expect(patience_reaction(0.6) == lc::Reaction::Neutral, "mid patience reads neutral");
	expect(patience_reaction(0.35) == lc::Reaction::Annoyed, "low patience reads annoyed");

	// --- patience drain ------------------------------------------------------
	{
		lc::CustomerState customer;
		lc::tick(customer, 10.0, false, tuning);
		expect_near(customer.patience, 0.8, 1e-9, "idle drain follows patience_drain_per_second_idle");
		lc::tick(customer, 39.0, false, tuning); // 49 s total: 0.02 patience left
		expect(!customer.walked_out && customer.patience > 0.0,
			"still standing just before patience runs out");
		expect(lc::reaction_for(customer, tuning) == lc::Reaction::Annoyed,
			"last stretch of patience reads annoyed");
		lc::tick(customer, 1.5, false, tuning);
		expect(customer.walked_out, "patience hitting zero walks them out");
		expect(lc::reaction_for(customer, tuning) == lc::Reaction::WalkedOut,
			"walked out is final");
	}
	{
		lc::CustomerState customer;
		lc::tick(customer, 10.0, true, tuning);
		expect_near(customer.patience, 0.5, 1e-9,
			"waiting in a queue drains harder (patience_drain_per_second_waiting)");
	}

	// --- spills ---------------------------------------------------------------
	{
		lc::CustomerState customer;
		lc::on_spill(customer, 10.0, tuning);
		expect_near(customer.patience, 0.8, 1e-9, "10 ml spill burns 10 * spill_patience_per_ml");
		expect_near(customer.shock_s, 3.0, 1e-9, "spill opens the shock window");
		expect(lc::reaction_for(customer, tuning) == lc::Reaction::Shocked,
			"immediately after a spill they are shocked");
		lc::tick(customer, 3.5, false, tuning);
		expect(lc::reaction_for(customer, tuning) == lc::Reaction::Neutral,
			"shock wears off and the band shows again");
	}
	{
		lc::CustomerState customer;
		lc::on_spill(customer, 150.0, tuning); // > burst_walkout_spill_ml
		expect(customer.walked_out, "a burst spill ends the visit on the spot");
	}
	{
		lc::CustomerState customer;
		lc::tick(customer, 25.0, false, tuning); // patience 0.5
		lc::on_spill(customer, 25.0, tuning);    // -0.5
		expect(customer.walked_out, "spill on already low patience walks them out");
	}
	{
		lc::CustomerState customer;
		lc::on_spill(customer, -5.0, tuning);
		expect(!customer.walked_out && customer.shock_s == 0.0,
			"negative spill is ignored (defensive)");
	}

	// --- serves ---------------------------------------------------------------
	{
		lc::CustomerState customer;
		customer.patience = 0.5;
		lc::on_serve(customer, lc::ComboQuality::Critical, tuning);
		expect_near(customer.patience, 0.8, 1e-9, "critical combo lifts patience by bonus_critical");
	}
	{
		lc::CustomerState customer;
		customer.patience = 0.5;
		lc::on_serve(customer, lc::ComboQuality::Good, tuning);
		expect_near(customer.patience, 0.65, 1e-9, "good combo lifts patience by refill_on_correct_order");
	}
	{
		lc::CustomerState customer;
		customer.patience = 0.5;
		lc::on_serve(customer, lc::ComboQuality::Fine, tuning);
		expect_near(customer.patience, 0.5, 1e-9, "fine serve changes nothing");
	}
	{
		lc::CustomerState customer;
		customer.patience = 0.6;
		lc::on_serve(customer, lc::ComboQuality::Ruined, tuning);
		expect_near(customer.patience, 0.1, 1e-9, "ruined drink costs patience_penalty_ruined");
		expect(!customer.walked_out, "ruined drink alone is not a walk-out at high patience");
	}
	{
		lc::CustomerState customer;
		customer.patience = 0.5;
		lc::on_serve(customer, lc::ComboQuality::Ruined, tuning);
		expect(customer.walked_out, "ruined drink on half patience walks them out");
	}
	{
		lc::CustomerState customer;
		lc::on_serve(customer, lc::ComboQuality::Critical, tuning);
		expect_near(customer.patience, 1.0, 1e-9, "patience never exceeds full");
		customer.walked_out = true;
		lc::on_serve(customer, lc::ComboQuality::Critical, tuning);
		expect_near(customer.patience, 1.0, 1e-9, "nobody is served after walking out");
	}

	// --- satisfaction mapping -------------------------------------------------
	expect_near(lc::satisfaction_for(lc::ComboQuality::Critical, tuning), 0.3, 1e-9,
		"critical satisfaction matches tuning");
	expect_near(lc::satisfaction_for(lc::ComboQuality::Good, tuning), 0.15, 1e-9,
		"good satisfaction matches tuning");
	expect_near(lc::satisfaction_for(lc::ComboQuality::Fine, tuning), 0.0, 1e-9,
		"fine satisfaction is neutral");
	expect_near(lc::satisfaction_for(lc::ComboQuality::Ruined, tuning), -0.5, 1e-9,
		"ruined satisfaction is the penalty");

	// --- balance sync (Data/Balance/core.json "customer") ---------------------
	expect_near(lc_test::balance_number("patience_drain_per_second_idle"),
		tuning.drain_idle_per_s, 1e-9, "drain idle in sync with core.json");
	expect_near(lc_test::balance_number("patience_drain_per_second_waiting"),
		tuning.drain_waiting_per_s, 1e-9, "drain waiting in sync with core.json");
	expect_near(lc_test::balance_number("patience_refill_on_correct_order"),
		tuning.refill_correct_order, 1e-9, "refill on correct order in sync with core.json");
	expect_near(lc_test::balance_number("patience_penalty_ruined"),
		tuning.ruined_penalty, 1e-9, "ruined penalty in sync with core.json");
	expect_near(lc_test::balance_number("mood_bonus_on_critical_combo"),
		tuning.bonus_critical, 1e-9, "critical bonus in sync with core.json");
	expect_near(lc_test::balance_number("spill_patience_per_ml"),
		tuning.spill_patience_per_ml, 1e-9, "spill burn in sync with core.json");
	expect_near(lc_test::balance_number("burst_walkout_spill_ml"),
		tuning.burst_walkout_spill_ml, 1e-9, "burst walkout in sync with core.json");
	expect_near(lc_test::balance_number("shock_seconds"),
		tuning.shock_s, 1e-9, "shock window in sync with core.json");
	expect_near(lc_test::balance_number("band_pleased"),
		tuning.band_pleased, 1e-9, "pleased band in sync with core.json");
	expect_near(lc_test::balance_number("band_annoyed"),
		tuning.band_annoyed, 1e-9, "annoyed band in sync with core.json");

	return lc_test::report("customer reaction");
}
