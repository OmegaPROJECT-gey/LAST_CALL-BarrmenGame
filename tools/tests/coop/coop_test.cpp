// LAST CALL - co-op roles core unit tests.
//
// Runs without Unreal: scripts/test-coop.sh compiles this against the
// roles core, the JSON reader and the harness. Drives the real file
// Data/Coop/roles.json, cross-checks the action vocabulary against
// Data/schemas/coop.schema.json and the crowd load against the venue
// ladder in Data/Venue/venue.json.

#include "Coop/CoopCore.h"

#include "Core/JsonCore.h"
#include "Venue/VenueCore.h"

#include "test_support.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::RoleLibrary;

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
		RoleLibrary library;
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

	std::string blocker_mentions(const RoleLibrary& library, int players,
		const std::vector<std::string>& roles)
	{
		return library.assign_blocker(players, roles);
	}

} // namespace

int main()
{
	// --- the real document --------------------------------------------------------
	const std::string doc_text = read_repo_file("Data/Coop/roles.json");
	RoleLibrary library;
	std::string error;
	const bool loaded = library.load(doc_text, &error);
	if (!loaded)
	{
		std::printf("  load error: %s\n", error.c_str());
	}
	expect(loaded, "Data/Coop/roles.json loads");
	expect(library.roles().size() == 4, "the crew is the four roles of GDD 5.5");
	expect(library.min_players() == 2 && library.max_players() == 4,
		"co-op runs 2-4 players, as GAME-DESIGN says");

	for (const lc::RoleSpec& role : library.roles())
	{
		expect(!role.name_en.empty() && !role.name_ru.empty(),
			"the role has a bilingual name");
		expect(!role.job_en.empty() && !role.job_ru.empty(),
			"the role has a bilingual job line");
		expect(!role.actions.empty(), "the role works at least one action");
		expect(role.crowd >= 1, "the role keeps up with bodies");
		for (const std::string& action : role.actions)
		{
			expect(std::find(lc::action_vocabulary().begin(),
				lc::action_vocabulary().end(), action) != lc::action_vocabulary().end(),
				"the action is in the known vocabulary");
		}
	}

	// --- vocabulary: code, schema and data agree ---------------------------------
	{
		lc::JsonValue schema;
		std::string parse_error;
		expect(json_parse(read_repo_file("Data/schemas/coop.schema.json"), schema,
			&parse_error),
			"Data/schemas/coop.schema.json parses for the vocabulary cross-check");
		std::vector<std::string> schema_actions;
		if (const lc::JsonValue* defs = schema.find("$defs"))
		{
			if (const lc::JsonValue* action_def = defs->find("action"))
			{
				if (const lc::JsonValue* enumeration = action_def->find("enum"))
				{
					for (const lc::JsonValue& value : enumeration->array)
					{
						if (value.kind == lc::JsonValue::Kind::String)
						{
							schema_actions.push_back(value.string);
						}
					}
				}
			}
		}
		std::vector<std::string> code_actions = lc::action_vocabulary();
		std::sort(schema_actions.begin(), schema_actions.end());
		std::sort(code_actions.begin(), code_actions.end());
		expect(!schema_actions.empty() && schema_actions == code_actions,
			"the schema enum and the code vocabulary are the same set");
	}

	// --- the job split: the bartender can't seat, the bouncer can't pour ---------
	expect(library.can("bartender", "pour") && library.can("bartender", "dialogue"),
		"the bartender pours and talks");
	expect(!library.can("bartender", "seat"),
		"the bartender cannot seat people (GDD 5.5: coordination is the design)");
	expect(library.can("waiter", "seat") && library.can("waiter", "relay"),
		"the waiter seats and relays");
	expect(!library.can("waiter", "pour"),
		"the waiter cannot pour");
	expect(library.can("bouncer", "door") && library.can("bouncer", "night_security"),
		"the bouncer works the door and the night");
	expect(!library.can("bouncer", "pour"),
		"the bouncer cannot pour (GDD 5.5)");
	expect(library.can("dj", "music") && library.can("dj", "cue"),
		"the dj runs the music and the cues");
	expect(!library.can("dj", "door"), "the dj does not work the door");
	expect(!library.can("bartender", "greet"), "an action outside the role is false");
	expect(!library.can("ghost", "pour"), "an unknown role cannot act");
	expect(library.find("bouncer") != nullptr && library.find("ghost") == nullptr,
		"find() by id");

	// --- crowd load: the crew against the venue ladder ----------------------------
	{
		lc::VenueLibrary venue;
		expect(venue.load(read_repo_file("Data/Venue/venue.json"), &error),
			"Data/Venue/venue.json loads for the crowd cross-check");

		const std::vector<std::string> quad = { "bartender", "waiter", "bouncer", "dj" };
		expect(library.crowd_capacity({ "ghost" }) == 0,
			"an unknown role carries nobody");

		// Every legal duo (someone must pour) clears the dive and the
		// VIP floor, none reaches the second floor; every legal trio
		// runs the second floor, none the casino; only the full crew
		// handles the underground casino. Retune Data/Coop and
		// Data/Venue together - this is the progression story.
		std::vector<std::string> ids;
		for (const lc::RoleSpec& role : library.roles())
		{
			ids.push_back(role.id);
		}
		int best_duo = 0;
		int worst_duo = -1;
		int best_trio = 0;
		int worst_trio = -1;
		for (std::size_t i = 0; i < ids.size(); ++i)
		{
			for (std::size_t j = i + 1; j < ids.size(); ++j)
			{
				const std::vector<std::string> crew = { ids[i], ids[j] };
				if (library.assign_blocker(2, crew).empty())
				{
					const int capacity = library.crowd_capacity(crew);
					best_duo = std::max(best_duo, capacity);
					worst_duo = worst_duo < 0 ? capacity : std::min(worst_duo, capacity);
				}
			}
		}
		for (std::size_t i = 0; i < ids.size(); ++i)
		{
			for (std::size_t j = i + 1; j < ids.size(); ++j)
			{
				for (std::size_t k = j + 1; k < ids.size(); ++k)
				{
					const std::vector<std::string> crew = { ids[i], ids[j], ids[k] };
					if (library.assign_blocker(3, crew).empty())
					{
						const int capacity = library.crowd_capacity(crew);
						best_trio = std::max(best_trio, capacity);
						worst_trio = worst_trio < 0 ? capacity : std::min(worst_trio, capacity);
					}
				}
			}
		}
		expect(worst_duo >= venue.tier(3)->capacity,
			"every legal duo clears the VIP floor");
		expect(best_duo < venue.tier(4)->capacity,
			"no legal duo reaches the second floor");
		expect(worst_trio >= venue.tier(4)->capacity,
			"every legal trio runs the second floor");
		expect(best_trio < venue.tier(5)->capacity,
			"no legal trio runs the casino");
		expect(library.crowd_capacity(quad) >= venue.tier(5)->capacity,
			"the full crew runs the casino");

		expect_near(library.crowd_load(0, quad), 0.0, 1e-9,
			"an empty room is no load");
		expect_near(library.crowd_load(51, quad), 1.0, 1e-9,
			"at capacity is exactly 1.0 - the crew at its limit");
		expect_near(library.crowd_load(102, quad), 2.0, 1e-9,
			"overrun doubles past 1.0 for the engine to scale on");
	}

	// --- a legal crew -------------------------------------------------------------
	expect(blocker_mentions(library, 2, { "bartender", "waiter" }).empty(),
		"bottle and plates: a legal two-player crew");
	expect(blocker_mentions(library, 3, { "bartender", "waiter", "dj" }).empty(),
		"a legal three-player crew");
	expect(blocker_mentions(library, 4,
		{ "bartender", "waiter", "bouncer", "dj" }).empty(),
		"all four roles: a legal four-player crew");

	// --- refusals, each named ------------------------------------------------------
	expect(!blocker_mentions(library, 1, { "bartender" }).empty(),
		"one player is below co-op");
	expect(!blocker_mentions(library, 5,
		{ "bartender", "waiter", "bouncer", "dj" }).empty(),
		"five players are above co-op");
	expect(blocker_mentions(library, 3, { "bartender", "waiter" })
		.find("one role per player") != std::string::npos,
		"a player without a role is refused");
	expect(blocker_mentions(library, 2, { "bartender", "bartender" })
		.find("twice") != std::string::npos,
		"a repeated role is refused");
	expect(blocker_mentions(library, 2, { "bartender", "ghost" })
		.find("unknown") != std::string::npos,
		"an unknown role is refused");
	expect(blocker_mentions(library, 2, { "waiter", "bouncer" })
		.find("must pour") != std::string::npos,
		"a crew with nobody to pour is refused");

	// --- strict loading ------------------------------------------------------------
	expect(refuses(replace_first(doc_text, "\"min\": 2", "\"min\": 1")),
		"a one-player minimum refused - co-op starts at two");
	expect(refuses(replace_first(doc_text, "\"min\": 2, \"max\": 4", "\"min\": 3, \"max\": 2")),
		"max below min refused");
	expect(refuses(replace_first(doc_text, "\"id\": \"waiter\"", "\"id\": \"bartender\"")),
		"a duplicate role id refused");
	expect(refuses(replace_first(doc_text, "\"ru\": \"Носка, рассаживание, уборка, передача заказов\"",
		"\"ru\": \"\"")),
		"a half-translated job line refused");
	expect(refuses(replace_first(doc_text, "\"combo\"", "\"juggling\"")),
		"an action outside the vocabulary refused");
	expect(refuses(replace_first(doc_text, "\"threat\", \"crowd\"", "\"door\", \"crowd\"")),
		"a repeated action refused");
	expect(refuses(replace_first(doc_text, "[\"music\", \"mood\", \"cue\"]", "[]")),
		"a role with no actions refused");
	expect(refuses(replace_first(doc_text, ",\n\t\"roles\"", ",\n\t\"secret\": 1,\n\t\"roles\"")),
		"an unknown field refused");
	expect(refuses(replace_first(doc_text, "\"players\":", "\"gamers\":")),
		"a document without `players` refused");
	expect(refuses(replace_first(doc_text, "\"crowd\": 10", "\"crowd\": 0")),
		"a zero crowd refused");
	expect(refuses(replace_first(doc_text, "\"crowd\": 15", "\"crowd\": \"many\"")),
		"a non-integer crowd refused");
	expect(refuses(replace_first(doc_text, ",\n\t\t\t\"crowd\": 15", "")),
		"a missing crowd refused");
	expect(refuses("not json"), "garbage refused");
	expect(refuses("[1,2]"), "an array document refused");

	return lc_test::report("co-op roles");
}
