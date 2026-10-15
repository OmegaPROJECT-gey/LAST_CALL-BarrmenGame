// LAST CALL - save state core unit tests.
//
// Runs without Unreal: scripts/test-save-state.sh compiles this against
// Source/LastCall/Core/SaveStateCore.cpp with g++. The tier bound is
// cross-checked against the real growth ladder (Data/Venue/venue.json).

#include "Core/SaveStateCore.h"

#include "Venue/VenueCore.h"

#include "test_support.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::Phase;
	using lc::SaveState;

	bool same_state(const SaveState& a, const SaveState& b)
	{
		if (a.day != b.day || a.phase != b.phase || a.money != b.money
			|| a.heat != b.heat || a.venue_tier != b.venue_tier
			|| a.standing.size() != b.standing.size())
		{
			return false;
		}
		// The ledger is a set of rows: written sorted, compared as a set.
		auto sorted = [](std::vector<std::pair<std::string, double>> rows) {
			std::sort(rows.begin(), rows.end());
			return rows;
		};
		const auto left = sorted(a.standing);
		const auto right = sorted(b.standing);
		for (std::size_t index = 0; index < left.size(); ++index)
		{
			if (left[index].first != right[index].first
				|| left[index].second != right[index].second)
			{
				return false;
			}
		}
		return true;
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
		state.standing = { { "ring", -0.5 }, { "bureau", 1.0 }, { "syndicate", 2.0 } };

		const std::string text = lc::serialize(state);
		SaveState loaded;
		expect(lc::deserialize(text, loaded), "a fresh payload parses");
		expect(same_state(state, loaded), "round trip preserves the state");
		expect_near(loaded.heat, 0.25, 1e-9, "heat survives the round trip exactly");
		expect(loaded.venue_tier == 3, "the venue tier survives the round trip");
		expect(loaded.standing.size() == 3
			&& loaded.standing[0].first == "bureau"
			&& loaded.standing[0].second == 1.0
			&& loaded.standing[2].first == "syndicate"
			&& loaded.standing[2].second == 2.0,
			"the standing ledger survives, sorted by faction id");
		expect(lc::serialize(loaded) == text, "serialization is deterministic");
	}
	{
		// The default state is what a new game starts with.
		SaveState fresh;
		expect_near(static_cast<double>(fresh.money),
			lc_test::balance_number("starting_money"), 1e-9,
			"starting money in sync with Data/Balance/core.json");
		expect(fresh.venue_tier == lc::kMinVenueTier, "a new game starts in the dive");
		expect(fresh.standing.empty(), "a new game remembers nobody yet");
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(fresh), loaded) && same_state(fresh, loaded),
			"the default state round trips");
		expect(lc::serialize(fresh).find("standing=") != std::string::npos,
			"the ledger is always written, even when empty");
	}
	{
		// Same rows, different insertion order: same bytes (A5's
		// ledger is a set of rows, not a history of edits).
		SaveState a;
		a.standing = { { "ring", -1.0 }, { "bureau", 0.5 } };
		SaveState b;
		b.standing = { { "bureau", 0.5 }, { "ring", -1.0 } };
		expect(lc::serialize(a) == lc::serialize(b),
			"the ledger serializes sorted, whatever order it was filled in");
	}
	{
		// Negative standing is legal - cooling is a real outcome.
		SaveState state;
		state.standing = { { "syndicate", -2.0 } };
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(state), loaded)
			&& loaded.standing.size() == 1
			&& loaded.standing[0].second == -2.0,
			"negative standing (a cold faction) survives");
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
		expect(refuses("lc-save 2\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1"
			"\nstanding="),
			"an old payload (v2) is refused, not migrated");
	}
	expect(refuses("lc-save 3\nday=2\nphase=1\nmoney=100\nheat=0"),
		"a missing venue_tier is refused");
	{
		const std::string missing_heat =
			"lc-save 3\nday=2\nphase=1\nmoney=100\nvenue_tier=1\nstanding=";
		expect(refuses(missing_heat),
			"a missing field is refused");
	}
	{
		const std::string missing_standing =
			"lc-save 3\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1";
		expect(refuses(missing_standing),
			"a missing standing line is refused - v3 always writes it");
	}
	{
		const std::string unknown_field =
			"lc-save 3\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1"
			"\nstanding=\nmood=0.5";
		expect(refuses(unknown_field),
			"an unknown field is refused - same version, same shape");
	}
	{
		const std::string day_zero =
			"lc-save 3\nday=0\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nstanding=";
		expect(refuses(day_zero),
			"day 0 is refused - days are 1-based");
	}
	{
		const std::string phase_seven =
			"lc-save 3\nday=1\nphase=7\nmoney=0\nheat=0\nvenue_tier=1\nstanding=";
		expect(refuses(phase_seven),
			"an unknown phase is refused");
	}
	{
		const std::string negative_heat =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=-0.1\nvenue_tier=1\nstanding=";
		expect(refuses(negative_heat),
			"negative heat is refused");
	}
	{
		const std::string tier_zero =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=0\nstanding=";
		expect(refuses(tier_zero),
			"venue tier 0 is refused - ladders are 1-based");
	}
	{
		const std::string tier_six =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=6\nstanding=";
		expect(refuses(tier_six),
			"a tier above the ladder is refused");
	}
	{
		const std::string no_numbers =
			"lc-save 3\nday=two\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nstanding=";
		expect(refuses(no_numbers),
			"non-numeric values are refused");
	}
	{
		const std::string no_header =
			"day=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nstanding=";
		expect(refuses(no_header),
			"a payload without the header is refused");
	}
	// The ledger's own rules: ids, numbers, duplicates.
	{
		const std::string duplicate_faction =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nstanding=bureau:1,bureau:2";
		expect(refuses(duplicate_faction),
			"a faction stored twice is refused");
	}
	{
		const std::string bad_id =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nstanding=Bureau:1";
		expect(refuses(bad_id),
			"a malformed faction id is refused");
	}
	{
		const std::string no_number =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nstanding=bureau:";
		expect(refuses(no_number),
			"a standing without a value is refused");
	}
	{
		const std::string not_a_pair =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nstanding=bureau";
		expect(refuses(not_a_pair),
			"a standing entry without `id:value` is refused");
	}
	{
		const std::string bad_value =
			"lc-save 3\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nstanding=bureau:friendly";
		expect(refuses(bad_value),
			"a non-numeric standing is refused");
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
