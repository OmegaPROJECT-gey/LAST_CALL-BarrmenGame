// LAST CALL - dynamic events core unit tests (ROADMAP D3, GDD 5.2).
//
// Runs without Unreal: scripts/test-events.sh compiles this against the
// event core, the JSON reader, the faction registry (cross-check) and
// the harness. Drives the real file Data/Events/events.json.

#include "Events/EventCore.h"

#include "Social/SocialCore.h"

#include "test_support.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::EventLibrary;
	using lc::EventSpec;
	using lc::FactionLibrary;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	/** True when load() refuses this document and says why. */
	bool refuses(const std::string& text)
	{
		EventLibrary library;
		std::string error;
		return !library.load(text, &error) && !error.empty();
	}

	std::string replace_first(std::string text, const std::string& from,
		const std::string& to)
	{
		const std::size_t at = text.find(from);
		if (at != std::string::npos)
		{
			text.replace(at, from.size(), to);
		}
		return text;
	}

	/** The neutral world: nobody loves you, nobody hates you, day one. */
	std::vector<std::pair<std::string, double>> neutral_standing()
	{
		return { { "syndicate", 0.0 }, { "ring", 0.0 }, { "bureau", 0.0 },
			{ "station", 0.0 }, { "civilian", 0.0 } };
	}

} // namespace

int main()
{
	// --- the real document --------------------------------------------------------
	const std::string doc_text = read_repo_file("Data/Events/events.json");
	EventLibrary library;
	std::string error;
	const bool loaded = library.load(doc_text, &error);
	if (!loaded)
	{
		std::printf("  load error: %s\n", error.c_str());
	}
	expect(loaded, "Data/Events/events.json loads");
	expect(library.events().size() == 6, "the pool has its six mornings of GDD 5.2");

	// Structure: bilingual names, non-negative base, and every row the
	// design names (raid, rival opening, gang meetup, celebrity visit,
	// blackout) plus the quiet day that keeps the pool honest.
	for (const EventSpec& spec : library.events())
	{
		expect(!spec.name_en.empty() && !spec.name_ru.empty(),
			"every event has a bilingual name");
		expect(spec.weight_base >= 0.0, "a base weight is never negative");
	}
	expect(library.find("quiet_day") != nullptr, "the quiet day is in the pool");
	expect(library.find("raid") != nullptr, "the police raid is in the pool");
	expect(library.find("rival_opening") != nullptr, "the rival bar opening is in the pool");
	expect(library.find("gang_meetup") != nullptr, "the gang meetup is in the pool");
	expect(library.find("celebrity_visit") != nullptr, "the celebrity visit is in the pool");
	expect(library.find("blackout") != nullptr, "the blackout is in the pool");
	expect(library.find("no_such_event") == nullptr, "an unknown event reads null");
	expect(library.find("") == nullptr, "an empty id reads null");

	// --- cross-check: every biased faction exists --------------------------------
	{
		FactionLibrary factions;
		expect(factions.load(read_repo_file("Data/Factions/factions.json"), &error),
			"the faction registry loads for the cross-check");
		bool saw_bias = false;
		for (const EventSpec& spec : library.events())
		{
			for (const lc::EventStandingBias& bias : spec.standing_bias)
			{
				saw_bias = true;
				expect(factions.find(bias.faction) != nullptr,
					"an event's bias names a known faction");
			}
		}
		expect(saw_bias, "the pool is actually weighted by standing");
	}

	// --- weight arithmetic --------------------------------------------------------
	{
		const EventSpec* quiet = library.find("quiet_day");
		const EventSpec* raid = library.find("raid");
		expect(quiet && raid, "quiet day and raid exist to be weighed");
		if (quiet && raid)
		{
			// The quiet day weighs its base and nothing else.
			expect_near(library.weight(*quiet, neutral_standing(), 0.0, 1), 3.0, 1e-9,
				"the quiet day weighs its base weight");
			expect_near(library.weight(*quiet, { { "syndicate", 5.0 } }, 99.0, 40), 3.0, 1e-9,
				"the quiet day ignores standing, heat and days");

			// Raid: base + per_heat x heat + per_day x day - bureau standing.
			expect_near(library.weight(*raid, neutral_standing(), 0.0, 1), 0.3, 1e-9,
				"a cold raid on day one weighs base + first day");
			expect_near(library.weight(*raid, neutral_standing(), 1.0, 1), 4.3, 1e-9,
				"heat is how the raid finds you (0.2 + 4.0 + 0.1)");
			expect_near(library.weight(*raid, { { "bureau", -3.0 } }, 1.0, 1), 7.3, 1e-9,
				"a hostile Bureau warms the raid (-1.0 per point)");
			expect_near(library.weight(*raid, { { "bureau", 5.0 } }, 0.0, 1), 0.0, 1e-9,
				"a friendly Bureau prices the raid out of the morning");

			// Slopes are per day from 1; day 0 and negative days count as 1.
			expect_near(library.weight(*raid, neutral_standing(), 0.0, 0), 0.3, 1e-9,
				"day 0 counts as day 1");
			expect_near(library.weight(*raid, neutral_standing(), 0.0, -7), 0.3, 1e-9,
				"a negative day counts as day 1");
			expect_near(library.weight(*raid, neutral_standing(), 0.0, 11), 1.3, 1e-9,
				"per_day adds once per campaign day");

			// A faction the event never mentions does nothing.
			expect_near(library.weight(*raid, { { "ring", -4.0 } }, 0.0, 1), 0.3, 1e-9,
				"an unmentioned faction does not move this weight");
		}

		// Celebrity: two biases add up, standing is read per point.
		const EventSpec* celebrity = library.find("celebrity_visit");
		expect(celebrity != nullptr, "the celebrity visit exists");
		if (celebrity)
		{
			expect_near(library.weight(*celebrity,
				{ { "civilian", 2.0 }, { "ring", 1.0 } }, 0.0, 1), 2.5, 1e-9,
				"1.0 + 0.6 x 2 + 0.3 x 1");
		}

		// A slope can carry a row with a zero base: negative days never
		// take a weight below zero.
		lc::EventSpec slope_only;
		slope_only.id = "slope";
		slope_only.weight_base = 0.0;
		slope_only.per_heat = -1.0;
		expect_near(library.weight(slope_only, neutral_standing(), 5.0, 1), 0.0, 1e-9,
			"a weight clamps at zero, never below");
	}

	// --- selection ------------------------------------------------------------------
	{
		const std::vector<std::pair<std::string, double>> world = neutral_standing();
		// Neutral day one: 3.0 + 0.3 + 1.0 + 1.2 + 0.8 + 0.9 = 7.2.
		expect(library.select(world, 0.0, 1, 0.0) == "quiet_day",
			"roll 0 takes the first live row");
		expect(library.select(world, 0.0, 1, 3.15 / 7.2) == "raid",
			"a roll inside the raid's slice takes the raid");
		expect(library.select(world, 0.0, 1, 0.999999) == "gang_meetup",
			"a roll at the end takes the last live row");
		expect(library.select(world, 0.0, 1, 1.0) == "gang_meetup",
			"a roll of 1 clamps into the last slice");
		expect(library.select(world, 0.0, 1, -3.0) == "quiet_day",
			"a roll below 0 clamps into the first slice");

		// The pool always answers in this world, and the quiet day -
		// the heaviest row - stays the commonest morning.
		std::map<std::string, int> hits;
		for (int sample = 0; sample < 1000; ++sample)
		{
			const std::string pick = library.select(world, 0.0, 1,
				static_cast<double>(sample) / 1000.0);
			expect(!pick.empty() && library.find(pick) != nullptr,
				"every roll lands on a real event");
			++hits[pick];
		}
		bool quiet_is_commonest = true;
		for (const auto& entry : hits)
		{
			if (entry.first != "quiet_day" && entry.second > hits["quiet_day"])
			{
				quiet_is_commonest = false;
			}
		}
		expect(quiet_is_commonest, "the quiet day is the commonest morning");

		// Priced out: with the Bureau at +5 the raid weighs zero and no
		// roll anywhere in the pool may pick it.
		const std::vector<std::pair<std::string, double>> friendly =
			{ { "syndicate", 0.0 }, { "ring", 0.0 }, { "bureau", 5.0 },
				{ "station", 0.0 }, { "civilian", 0.0 } };
		bool raid_priced_out = true;
		for (int sample = 0; sample < 1000; ++sample)
		{
			if (library.select(friendly, 0.0, 1, static_cast<double>(sample) / 1000.0)
				== "raid")
			{
				raid_priced_out = false;
			}
		}
		expect(raid_priced_out, "an event at zero weight never fires");

		// Heat pulls the raid back in: at heat 8 it is the heaviest row.
		expect(library.select(world, 8.0, 1, 0.999) == "gang_meetup"
			|| library.find(library.select(world, 8.0, 1, 0.5)) != nullptr,
			"a hot morning still selects");
		{
			const EventSpec* hot_raid = library.find("raid");
			const EventSpec* quiet = library.find("quiet_day");
			if (hot_raid && quiet)
			{
				expect(library.weight(*hot_raid, world, 8.0, 1)
					> library.weight(*quiet, world, 8.0, 1),
					"heat makes the raid heavier than the quiet day");
			}
		}
	}

	// --- an empty pool ---------------------------------------------------------------
	{
		// One row whose only slope is off: nothing can happen, and the
		// engine is told so by an empty id rather than a bogus pick.
		EventLibrary dead;
		const std::string dead_doc =
			"{\"events\":[{\"id\":\"only_heat\",\"name\":{\"en\":\"Heat Only\","
			"\"ru\":\"Только жара\"},\"weight_base\":0.0,\"standing_bias\":[],"
			"\"per_heat\":0.0,\"per_day\":0.0}]}";
		// weight_base 0 with no slope at all is refused at load ...
		expect(refuses(dead_doc), "a row that can never fire is refused");
		// ... but a row that can fire in some world loads and answers "".
		const std::string slope_doc =
			"{\"events\":[{\"id\":\"only_heat\",\"name\":{\"en\":\"Heat Only\","
			"\"ru\":\"Только жара\"},\"weight_base\":0.0,\"standing_bias\":[],"
			"\"per_heat\":1.0,\"per_day\":0.0}]}";
		expect(dead.load(slope_doc, &error), "a slope-only row loads");
		expect(dead.select(neutral_standing(), 0.0, 1, 0.5).empty(),
			"a morning where nothing weighs anything selects nothing");
		expect(!dead.select(neutral_standing(), 3.0, 1, 0.5).empty(),
			"heat wakes the same row up");
	}

	// --- strict loading ---------------------------------------------------------------
	expect(refuses(replace_first(doc_text, "\"id\": \"raid\"", "\"id\": \"quiet_day\"")),
		"a duplicate event id refused");
	expect(refuses(replace_first(doc_text, "\"weight_base\": 0.2", "\"weight_base\": -1")),
		"a negative base weight refused");
	expect(refuses(replace_first(doc_text, "\"weight_base\": 3.0", "\"weight_base\": \"3\"")),
		"a string base weight refused");
	expect(refuses(replace_first(doc_text, "\"per_heat\": 4.0", "\"per_heat\": \"hot\"")),
		"a string heat slope refused");
	expect(refuses(replace_first(doc_text, "\"ru\": \"Тихий день\"", "\"ru\": \"\"")),
		"a half-translated name refused");
	expect(refuses(replace_first(doc_text, "\"en\": \"Blackout\"", "\"en\": \"\"")),
		"an empty English name refused");
	// Faction ids are named, not owned: the core checks the shape, the
	// registry cross-check above checks the name is real.
	expect(refuses(replace_first(doc_text, "\"faction\": \"bureau\"",
		"\"faction\": \"Bureau!\"")),
		"a malformed faction id refused");
	expect(refuses(replace_first(doc_text,
		"{ \"faction\": \"bureau\", \"per_point\": -1.0 }",
		"{ \"faction\": \"bureau\", \"per_point\": -1.0 }, "
		"{ \"faction\": \"bureau\", \"per_point\": 0.5 }")),
		"a faction biased twice in one event refused");
	expect(refuses(replace_first(doc_text,
		"{ \"faction\": \"station\", \"per_point\": -0.6 }",
		"{ \"faction\": \"station\", \"per_point\": \"-0.6\" }")),
		"a non-numeric per-point bias refused");
	expect(refuses(replace_first(doc_text,
		",\n\t\t\t\"per_day\": 0.0", ",\n\t\t\t\"per_day\": 0.0,\n\t\t\t\"mood\": 1")),
		"an unknown field refused");
	expect(refuses(replace_first(doc_text, "\"events\": [", "\"pool\": [")),
		"a document without `events` refused");
	expect(refuses(replace_first(doc_text, "\"weight_base\": 0.2,",
		"\"weight_base\": 0.2,\n\t\t\t\"surprise\": true,")),
		"an unknown field inside an event refused");
	expect(refuses("{"), "garbage refused");
	expect(refuses("[1,2]"), "an array document refused");
	expect(refuses(""), "an empty document refused");

	// A refusal leaves the previously loaded pool intact.
	{
		EventLibrary kept;
		expect(kept.load(doc_text, &error), "the real document loads again");
		const std::size_t before = kept.events().size();
		expect(!kept.load("not json", &error), "garbage is refused");
		expect(kept.events().size() == before,
			"a refused load leaves the previous pool intact");
	}

	return lc_test::report("dynamic events");
}
