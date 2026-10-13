// LAST CALL - save state core unit tests.
//
// Runs without Unreal: scripts/test-save-state.sh compiles this against
// Source/LastCall/Core/SaveStateCore.cpp with g++.

#include "Core/SaveStateCore.h"

#include "test_support.h"

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::Phase;
	using lc::SaveState;

	bool same_state(const SaveState& a, const SaveState& b)
	{
		return a.day == b.day && a.phase == b.phase && a.money == b.money && a.heat == b.heat;
	}

	// True when the payload is rejected (scratch state, caller untouched).
	bool refuses(const std::string& text)
	{
		SaveState scratch;
		return !lc::deserialize(text, scratch);
	}

} // namespace

int main()
{
	// --- round trip ------------------------------------------------------------
	{
		SaveState state;
		state.day = 5;
		state.phase = Phase::Night;
		state.money = 1850;
		state.heat = 0.25;

		const std::string text = lc::serialize(state);
		SaveState loaded;
		expect(lc::deserialize(text, loaded), "a fresh payload parses");
		expect(same_state(state, loaded), "round trip preserves the state");
		expect_near(loaded.heat, 0.25, 1e-9, "heat survives the round trip exactly");
		expect(lc::serialize(loaded) == text, "serialization is deterministic");
	}
	{
		// The default state is what a new game starts with.
		SaveState fresh;
		expect_near(static_cast<double>(fresh.money),
			lc_test::balance_number("starting_money"), 1e-9,
			"starting money in sync with Data/Balance/core.json");
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(fresh), loaded) && same_state(fresh, loaded),
			"the default state round trips");
	}

	// --- loans: money below zero is legal --------------------------------------
	{
		SaveState state;
		state.money = -100;
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(state), loaded) && loaded.money == -100,
			"negative money (a loan) survives - the design has loans");
	}

	// --- refusals ----------------------------------------------------------------
	expect(refuses("total garbage"),
		"garbage is refused");
	expect(refuses(""),
		"empty payload is refused");
	{
		std::string text = lc::serialize(SaveState{});
		text[8] = '9'; // version digit -> "lc-save 9..."
		expect(refuses(text),
			"a foreign version is refused, not migrated");
	}
	{
		const std::string missing_heat =
			"lc-save 1\nday=2\nphase=1\nmoney=100";
		expect(refuses(missing_heat),
			"a missing field is refused");
	}
	{
		const std::string unknown_field =
			"lc-save 1\nday=2\nphase=1\nmoney=100\nheat=0\nmood=0.5";
		expect(refuses(unknown_field),
			"an unknown field is refused - same version, same shape");
	}
	{
		const std::string day_zero = "lc-save 1\nday=0\nphase=0\nmoney=0\nheat=0";
		expect(refuses(day_zero),
			"day 0 is refused - days are 1-based");
	}
	{
		const std::string phase_seven = "lc-save 1\nday=1\nphase=7\nmoney=0\nheat=0";
		expect(refuses(phase_seven),
			"an unknown phase is refused");
	}
	{
		const std::string negative_heat = "lc-save 1\nday=1\nphase=0\nmoney=0\nheat=-0.1";
		expect(refuses(negative_heat),
			"negative heat is refused");
	}
	{
		const std::string no_numbers = "lc-save 1\nday=two\nphase=0\nmoney=0\nheat=0";
		expect(refuses(no_numbers),
			"non-numeric values are refused");
	}
	{
		const std::string no_header = "day=1\nphase=0\nmoney=0\nheat=0";
		expect(refuses(no_header),
			"a payload without the header is refused");
	}
	{
		// A refused parse must not touch the output state.
		SaveState loaded;
		loaded.day = 7;
		expect(!lc::deserialize("garbage", loaded) && loaded.day == 7,
			"a refused parse leaves the caller's state untouched");
	}

	return lc_test::report("save state");
}
