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
			|| a.hospital_days != b.hospital_days
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
		// The story flags are a set of rows too: written sorted,
		// compared as a set.
		if (a.flags.size() != b.flags.size())
		{
			return false;
		}
		auto flags_sorted = [](std::vector<std::pair<std::string, bool>> rows) {
			std::sort(rows.begin(), rows.end());
			return rows;
		};
		const auto left_flags = flags_sorted(a.flags);
		const auto right_flags = flags_sorted(b.flags);
		for (std::size_t index = 0; index < left_flags.size(); ++index)
		{
			if (left_flags[index] != right_flags[index])
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
		state.hospital_days = 2; // died on day 5, the stay rides along
		state.standing = { { "ring", -0.5 }, { "bureau", 1.0 }, { "syndicate", 2.0 } };
		state.flags = { { "read_uncle_wall", true }, { "kane_trust", false } };

		const std::string text = lc::serialize(state);
		SaveState loaded;
		expect(lc::deserialize(text, loaded), "a fresh payload parses");
		expect(same_state(state, loaded), "round trip preserves the state");
		expect_near(loaded.heat, 0.25, 1e-9, "heat survives the round trip exactly");
		expect(loaded.venue_tier == 3, "the venue tier survives the round trip");
		expect(loaded.hospital_days == 2, "the hospital stay survives the round trip");
		expect(loaded.standing.size() == 3
			&& loaded.standing[0].first == "bureau"
			&& loaded.standing[0].second == 1.0
			&& loaded.standing[2].first == "syndicate"
			&& loaded.standing[2].second == 2.0,
			"the standing ledger survives, sorted by faction id");
		expect(loaded.flags.size() == 2
			&& loaded.flags[0].first == "kane_trust"
			&& !loaded.flags[0].second
			&& loaded.flags[1].first == "read_uncle_wall"
			&& loaded.flags[1].second,
			"the story flags survive, sorted by flag id");
		expect(lc::serialize(loaded) == text, "serialization is deterministic");
	}
	{
		// The default state is what a new game starts with.
		SaveState fresh;
		expect_near(static_cast<double>(fresh.money),
			lc_test::balance_number("starting_money"), 1e-9,
			"starting money in sync with Data/Balance/core.json");
		expect(fresh.venue_tier == lc::kMinVenueTier, "a new game starts in the dive");
		expect(fresh.hospital_days == 0, "a new game never sat in hospital");
		expect(fresh.standing.empty(), "a new game remembers nobody yet");
		expect(fresh.flags.empty(), "a new game has recorded no story yet");
		SaveState loaded;
		expect(lc::deserialize(lc::serialize(fresh), loaded) && same_state(fresh, loaded),
			"the default state round trips");
		expect(lc::serialize(fresh).find("standing=") != std::string::npos,
			"the ledger is always written, even when empty");
		expect(lc::serialize(fresh).find("\nflags=") != std::string::npos,
			"the story flags are always written, even when empty");
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
		SaveState flags_a;
		flags_a.flags = { { "zulu", true }, { "alpha", false } };
		SaveState flags_b;
		flags_b.flags = { { "alpha", false }, { "zulu", true } };
		expect(lc::serialize(flags_a) == lc::serialize(flags_b),
			"story flags serialize sorted, whatever order they were set in");
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
			"\nhospital=0\nstanding="),
			"an old payload (v2) is refused, not migrated");
		expect(refuses("lc-save 3\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding="),
			"an old payload (v3) is refused, not migrated");
		expect(refuses("lc-save 4\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding="),
			"the previous payload (v4) is refused, not migrated");
	}
	expect(refuses("lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
		"\nstanding=\nflags="),
		"a missing hospital line is refused");
	{
		const std::string negative_hospital =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=-1\nstanding=\nflags=";
		expect(refuses(negative_hospital),
			"a negative stay is refused - days cannot run backwards");
	}
	{
		const std::string duplicate_hospital =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nhospital=2\nstanding=\nflags=";
		expect(refuses(duplicate_hospital),
			"a stay stored twice is refused");
	}
	expect(refuses("lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
		"\nhospital=0\nstanding="),
		"a missing flags line is refused - v5 always writes it");
	{
		const std::string bad_flag_value =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=\nflags=read_uncle_wall:maybe";
		expect(refuses(bad_flag_value),
			"a flag without true/false is refused");
	}
	{
		const std::string duplicate_flag =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=\nflags=kane_trust:true,kane_trust:false";
		expect(refuses(duplicate_flag),
			"a flag stored twice is refused");
	}
	{
		const std::string bad_flag_id =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=\nflags=KaneTrust:true";
		expect(refuses(bad_flag_id),
			"a malformed flag id is refused");
	}
	expect(refuses("lc-save 5\nday=2\nphase=1\nmoney=100\nheat=0\nhospital=0\nflags="),
		"a missing venue_tier is refused");
	{
		const std::string missing_heat =
			"lc-save 5\nday=2\nphase=1\nmoney=100\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(missing_heat),
			"a missing field is refused");
	}
	{
		const std::string missing_standing =
			"lc-save 5\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1\nhospital=0\nflags=";
		expect(refuses(missing_standing),
			"a missing standing line is refused - the ledger line is always written");
	}
	{
		const std::string unknown_field =
			"lc-save 5\nday=2\nphase=1\nmoney=100\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=\nflags=\nmood=0.5";
		expect(refuses(unknown_field),
			"an unknown field is refused - same version, same shape");
	}
	{
		const std::string day_zero =
			"lc-save 5\nday=0\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(day_zero),
			"day 0 is refused - days are 1-based");
	}
	{
		const std::string phase_seven =
			"lc-save 5\nday=1\nphase=7\nmoney=0\nheat=0\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(phase_seven),
			"an unknown phase is refused");
	}
	{
		const std::string negative_heat =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=-0.1\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(negative_heat),
			"negative heat is refused");
	}
	{
		const std::string tier_zero =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=0\nhospital=0\nstanding=\nflags=";
		expect(refuses(tier_zero),
			"venue tier 0 is refused - ladders are 1-based");
	}
	{
		const std::string tier_six =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=6\nhospital=0\nstanding=\nflags=";
		expect(refuses(tier_six),
			"a tier above the ladder is refused");
	}
	{
		const std::string no_numbers =
			"lc-save 5\nday=two\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(no_numbers),
			"non-numeric values are refused");
	}
	{
		const std::string no_header =
			"day=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1\nhospital=0\nstanding=\nflags=";
		expect(refuses(no_header),
			"a payload without the header is refused");
	}
	// The ledger's own rules: ids, numbers, duplicates.
	{
		const std::string duplicate_faction =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=bureau:1,bureau:2\nflags=";
		expect(refuses(duplicate_faction),
			"a faction stored twice is refused");
	}
	{
		const std::string bad_id =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=Bureau:1\nflags=";
		expect(refuses(bad_id),
			"a malformed faction id is refused");
	}
	{
		const std::string no_number =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=bureau:\nflags=";
		expect(refuses(no_number),
			"a standing without a value is refused");
	}
	{
		const std::string not_a_pair =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=bureau\nflags=";
		expect(refuses(not_a_pair),
			"a standing entry without `id:value` is refused");
	}
	{
		const std::string bad_value =
			"lc-save 5\nday=1\nphase=0\nmoney=0\nheat=0\nvenue_tier=1"
			"\nhospital=0\nstanding=bureau:friendly\nflags=";
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
