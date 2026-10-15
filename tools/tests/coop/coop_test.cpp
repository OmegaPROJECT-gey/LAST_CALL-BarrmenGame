// LAST CALL - co-op roles core unit tests.
//
// Runs without Unreal: scripts/test-coop.sh compiles this against the
// roles core, the JSON reader and the harness. Drives the real file
// Data/Coop/roles.json and cross-checks the action vocabulary against
// Data/schemas/coop.schema.json.

#include "Coop/CoopCore.h"

#include "Core/JsonCore.h"

#include "test_support.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

	using lc_test::expect;

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
	expect(refuses("not json"), "garbage refused");
	expect(refuses("[1,2]"), "an array document refused");

	return lc_test::report("co-op roles");
}
