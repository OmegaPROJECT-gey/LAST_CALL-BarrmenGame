// LAST CALL - destructibility core unit tests.
//
// Runs without Unreal: scripts/test-interactions.sh compiles this against
// the interaction core, the JSON reader and the harness.
// Drives the real file Data/Interactions/interactions.json.

#include "Bar/InteractionCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;

	using lc::HeldObject;
	using lc::InteractionLibrary;
	using lc::InteractionSpec;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		InteractionLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

	// One interaction document with the given id/kinds/damage/sturdiness/phases.
	std::string interaction(const std::string& id, const std::string& kinds,
		const std::string& damage, const std::string& sturdiness, const std::string& phases)
	{
		return "{\"interactions\":[{\"id\":\"" + id + "\","
			"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
			"\"kinds\":[" + kinds + "],"
			"\"damage\":" + damage + ",\"sturdiness\":" + sturdiness + ","
			"\"phases\":[" + phases + "]}]}";
	}

} // namespace

int main()
{
	// --- the real file on disk -------------------------------------------------------
	InteractionLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Interactions/interactions.json"), &error),
		"Data/Interactions/interactions.json loads");
	expect(library.ids().size() == 4, "four objects in the room");

	const InteractionSpec* bottle = library.find("bottle_rye");
	expect(bottle != nullptr, "the rye bottle is on the shelf");
	if (bottle)
	{
		expect(bottle->kinds.size() == 2 && lc::is_weapon(*bottle),
			"bottles are weapons");
		expect(bottle->damage == 12.0 && bottle->sturdiness == 3,
			"bottle: 12 damage, 3 swings");
		expect(lc::usable_in(*bottle, "Shift") && lc::usable_in(*bottle, "Night"),
			"the bottle serves both shift and night");
		expect(!lc::usable_in(*bottle, "Morning"), "and only those two");
		expect(!lc::is_cover(*bottle) && !lc::is_extinguisher(*bottle),
			"bottles extinguish nothing");
	}

	const InteractionSpec* stool = library.find("barstool");
	expect(stool != nullptr && lc::is_weapon(*stool) && lc::is_cover(*stool),
		"a stool swings and hides behind");

	const InteractionSpec* table = library.find("table_heavy");
	expect(table != nullptr && !lc::is_weapon(*table) && lc::is_cover(*table),
		"the heavy table is cover, not a weapon");
	expect(table != nullptr && table->damage == 0.0, "it hits for nothing");

	const InteractionSpec* siphon = library.find("siphon");
	expect(siphon != nullptr && lc::is_extinguisher(*siphon)
		&& !lc::is_weapon(*siphon) && !lc::is_cover(*siphon),
		"the siphon puts fires out and nothing else");
	expect(siphon != nullptr && siphon->sturdiness == 6, "six squirts per charge");

	expect(library.find("no_such_object") == nullptr, "unknown objects read null");

	// --- best weapon among what is held ------------------------------------------------
	{
		const lc::InteractionSpec* best = lc::best_weapon(library,
			{ "table_heavy", "bottle_rye", "siphon" }, "Night");
		expect(best != nullptr && best->id == "bottle_rye",
			"out of table, bottle and siphon, the bottle swings hardest");

		const lc::InteractionSpec* heaviest = lc::best_weapon(library,
			{ "barstool", "bottle_rye" }, "Shift");
		expect(heaviest != nullptr && heaviest->id == "bottle_rye", "12 beats 8");

		expect(lc::best_weapon(library, { "table_heavy", "siphon" }, "Night") == nullptr,
			"no weapon held reads null");
		expect(lc::best_weapon(library, {}, "Night") == nullptr, "empty hands read null");
		expect(lc::best_weapon(library, { "no_such_object" }, "Night") == nullptr,
			"unknown ids are skipped");
	}

	// --- durability ----------------------------------------------------------------------
	{
		const InteractionSpec* found = library.find("bottle_rye");
		if (found)
		{
			HeldObject held = lc::pick_up(*found);
			expect(held.id == "bottle_rye" && held.sturdiness_left == 3,
				"picked up at full sturdiness");
			expect(lc::use_once(held) && held.sturdiness_left == 2, "swing one holds");
			expect(lc::use_once(held) && held.sturdiness_left == 1, "swing two holds");
			expect(!lc::use_once(held) && held.sturdiness_left == 0,
				"swing three breaks it");
			expect(!lc::use_once(held), "a broken bottle swings no more");
		}
	}

	// --- strict refusals --------------------------------------------------------------------
	expect(refuses("{\"interactions\":[{\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"kinds\":[\"weapon\"],\"damage\":1,\"sturdiness\":1,\"phases\":[\"Shift\"]}]}"),
		"missing id refused");
	expect(refuses(interaction("x", "\"grenade\"", "1", "1", "\"Shift\"")),
		"unknown kind refused");
	expect(refuses(interaction("x", "\"weapon\"", "-1", "1", "\"Shift\"")),
		"negative damage refused");
	expect(refuses(interaction("x", "\"weapon\"", "1", "0", "\"Shift\"")),
		"zero sturdiness refused");
	expect(refuses(interaction("x", "\"weapon\"", "1", "1", "\"Morning\"")),
		"morning phase refused (shift and night only)");
	expect(refuses(interaction("x", "\"weapon\"", "1", "1", "")),
		"empty phases refused");
	expect(refuses(interaction("x", "\"weapon\"", "1", "1", "\"Shift\",\"Shift\"")),
		"duplicate phase refused");
	expect(refuses("{\"interactions\":[{\"id\":\"x\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"\"},"
		"\"kinds\":[\"weapon\"],\"damage\":1,\"sturdiness\":1,\"phases\":[\"Shift\"]}]}"),
		"empty ru name refused");
	expect(refuses("{\"interactions\":[]}"), "empty interactions refused");
	expect(refuses("not json"), "garbage refused");

	{
		InteractionLibrary scratch;
		std::vector<std::string> dir_errors;
		expect(scratch.load_directory("/no/such/directory", &dir_errors) == 0
			&& !dir_errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("destructibility");
}
