// LAST CALL - raid director core unit tests.
//
// Runs without Unreal: scripts/test-raid.sh compiles this against the
// raid core and the harness. Pure rules: the evidence count, the
// partner's capacity and the distraction score decide.

#include "Bar/RaidCore.h"

#include "test_support.h"

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::RaidState;
	using lc::RaidTuning;

	RaidState bar_with(int evidence, int partner_capacity)
	{
		RaidState state;
		state.partner_capacity = partner_capacity;
		for (int i = 0; i < evidence; ++i)
		{
			state.evidence.push_back("evidence_" + std::to_string(i));
		}
		return state;
	}

} // namespace

int main()
{
	const RaidTuning tuning;

	// Code and Data/Balance/core.json must agree on the tuning.
	expect_near(tuning.distraction_threshold,
		lc_test::balance_number("distraction_threshold"), 1e-9,
		"distraction_threshold synced with core.json");
	expect_near(tuning.fail_heat,
		lc_test::balance_number("fail_heat"), 1e-9,
		"fail_heat synced with core.json");
	expect_near(tuning.per_item_heat,
		lc_test::balance_number("per_item_heat"), 1e-9,
		"per_item_heat synced with core.json");
	expect_near(tuning.fail_fine,
		lc_test::balance_number("fail_fine"), 1e-9,
		"fail_fine synced with core.json");

	// --- a passed distraction check ---------------------------------------------------
	{
		const lc::RaidResult calm = lc::resolve_raid(bar_with(3, 1), 0.8, tuning);
		expect(calm.distraction_worked && calm.found == 0
			&& calm.hidden_by_partner == 1,
			"the check passes: they listen at the door, one item hidden");
		expect_near(calm.heat_delta, 0.0, 1e-9, "and nothing reaches the city");
		expect_near(calm.fine, 0.0, 1e-9, "and nobody pays a fine");
	}

	// --- a failed check over evidence the partner could not hide ----------------------
	{
		const lc::RaidResult caught = lc::resolve_raid(bar_with(3, 1), 0.2, tuning);
		expect(!caught.distraction_worked && caught.found == 2,
			"the check fails: two of three items are found");
		expect_near(caught.heat_delta, tuning.fail_heat + 2.0 * tuning.per_item_heat, 1e-9,
			"base heat plus per item");
		expect_near(caught.fine, 2.0 * tuning.fail_fine, 1e-9, "a fine per found item");
	}

	// --- the partner hides everything -------------------------------------------------
	{
		const lc::RaidResult hidden = lc::resolve_raid(bar_with(2, 5), 0.0, tuning);
		expect(!hidden.distraction_worked && hidden.found == 0
			&& hidden.hidden_by_partner == 2,
			"even a whiffed check finds nothing when the partner hid it all");
		expect_near(hidden.heat_delta, 0.0, 1e-9, "nothing found, nothing hot");
	}

	// --- an empty bar is just an uncomfortable evening ----------------------------------
	{
		const lc::RaidResult empty = lc::resolve_raid(bar_with(0, 0), 0.0, tuning);
		expect(!empty.distraction_worked && empty.found == 0,
			"a failed check over an empty bar finds nothing");
		expect_near(empty.heat_delta, 0.0, 1e-9, "no heat without evidence");
		expect_near(empty.fine, 0.0, 1e-9, "no fine without evidence");
	}

	// --- the threshold is exact ----------------------------------------------------------
	{
		const lc::RaidResult edge = lc::resolve_raid(bar_with(1, 0),
			tuning.distraction_threshold, tuning);
		expect(edge.distraction_worked && edge.found == 0,
			"sitting exactly on the threshold holds them at the door");
	}

	// --- guards --------------------------------------------------------------------------
	{
		RaidState negative;
		negative.partner_capacity = -3;
		const lc::RaidResult guarded = lc::resolve_raid(negative, 0.9, tuning);
		expect(guarded.hidden_by_partner >= 0, "a negative capacity floors at zero");
	}

	return lc_test::report("raid director");
}
