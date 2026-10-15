// LAST CALL - save state core unit tests.
//
// Runs without Unreal: scripts/test-save-state.sh compiles this against
// Source/LastCall/Core/SaveStateCore.cpp with g++. The tier bound is
// cross-checked against the real growth ladder (Data/Venue/venue.json).

#include "Core/SaveStateCore.h"

#include "Venue/VenueCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>
#include <string>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::Phase;
	using lc::SaveState;

	bool same_state(const SaveState& a, const SaveState& b)
	{
		return a.day == b.day && a.phase == b.phase && a.money == b.money
			&& a.heat == b.heat && a.venue_tier == b.venue_tier;
	}

	// True when the payload is rejected (scratch state, caller untouched).
	bool refuses(const std::string& text)
	{
		SaveState scratch;
		return !lc::deserialize(text, scratch);
	}

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
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
		state.venue_tier = 3;

		const std::string text = lc::serialize(state);
		SaveState loaded;
		expect(lc::deserialize(text, loaded), "a fresh payload parses");
		expect(same_state(state, loaded), "round trip preserves the state");
		expect_near(loaded.heat, 0.25, 1e-9, "heat survives the round trip exactly");
		expect(loaded.venue_tier == 3, "the venue tier survives the round trip");
		expect(lc::serialize(loaded) == text, "serialization is deterministic");
	}
	{
		// The default state is what a new game starts with.
		SaveState fresh;
		expect_near(static_cast<double>(fresh.money),
			lc_test::balance_number("starting_money"), 1e-9,
			"starting money in sync with Data/Balance/core.json");
		expect(fresh.venue_tier == lc::kMinVenueTier, "a new game starts in the dive");
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

	// --- the tier bound agrees with the growth ladder --------------------------
	{
		lc::VenueLibrary venue;
		std::string error;
		expect(venue.load(read_repo_file("Data/Venue/venue.json"), &error),
			"Data/Venue/venue.json loads for the tier-bound cross-check");
		expect(lc::kMaxVenueTier == venue.count(),
			"the save's top tier is Data/Venue's ladder length");
		SaveState top;
		top.venue_tier = lc::kMaxVenueTier;
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(top), loaded)
			&& loaded.venue_tier == lc::kMaxVenueTier,
			"the casino tier round trips");
	}

	// --- refusals ----------------------------------------------------------------
	expect(refuses("total garbage"),
		"garbage is refused");
	expect(refuses(""),
		"empty payload is refused");
	{
		const std::string text = lc::serialize(SaveState{});
		std::string foreign = text;
		foreign[8] = '9'; // version digit -> "lc-save 9..."
		expect(foreign != text && refuses(foreign),
			"a foreign version is refused, not migrated");
	}
	expect(refuses("lc-save 2\nday=2\nphase=1\nmoney=100\nheat=0"),
		"a missing venue_tier is refused");
	{
		const std::string missing_heat =
			"lc-save 2\nday=2\nphase=1\nmoney=100\nvenue_tier=1";
		expect(refuses(missing_heat),
			"a missing field is refused");
	}
	{
		const std::string unknown_field =
			"lc-save 2\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1\nmood=0.5";
		expect(refuses(unknown_field),
			"an unknown field is refused - same version, same shape");
	}
	{
		const std::string day_zero =
			"lc-save 2\nday=0\nphase=0\nmoney=0\nheat=0\nvenue_tier=1";
		expect(refuses(day_zero),
			"day 0 is refused - days are 1-based");
	}
	{
		const std::string phase_seven =
			"lc-save 2\nday=1\nphase=7\nmoney=0\nheat=0\nvenue_tier=1";
		expect(refuses(phase_seven),
			"an unknown phase is refused");
	}
	{
		const std::string negative_heat =
			"lc-save 2\nday=1\nphase=0\nmoney=0\nheat=-0.1\nvenue_tier=1";
		expect(refuses(negative_heat),
			"negative heat is refused");
	}
	{
		const std::string tier_zero =
			"lc-save 2\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=0";
		expect(refuses(tier_zero),
			"venue tier 0 is refused - ladders are 1-based");
	}
	{
		const std::string tier_six =
			"lc-save 2\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=6";
		expect(refuses(tier_six),
			"a tier above the ladder is refused");
	}
	{
		const std::string no_numbers =
			"lc-save 2\nday=two\nphase=0\nmoney=0\nheat=0\nvenue_tier=1";
		expect(refuses(no_numbers),
			"non-numeric values are refused");
	}
	{
		const std::string no_header = "day=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1";
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
