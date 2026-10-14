// LAST CALL - crafting core unit tests.
//
// Runs without Unreal: scripts/test-crafting.sh compiles this against the
// crafting core, the liquid core, the JSON reader and the harness.
// Drives the real files Data/Crafting/crafting.json and Data/Liquids.

#include "Crafting/CraftingCore.h"

#include "Bar/LiquidCore.h"

#include "test_support.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::CraftLibrary;
	using lc::CraftOutcome;
	using lc::CraftSpec;
	using lc::CraftTuning;
	using lc::LiquidLibrary;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		CraftLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real files on disk ------------------------------------------------------
	CraftLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Crafting/crafting.json"), &error),
		"Data/Crafting/crafting.json loads");
	expect(library.ids().size() == 2, "two craftable rows: an infusion and a syrup");

	const CraftSpec* coffee = library.find("infusion_coffee");
	expect(coffee != nullptr, "the coffee infusion is on the bench");
	if (coffee)
	{
		expect(coffee->kind == "infusion" && coffee->inputs.size() == 2
			&& coffee->inputs[0].item == "coffee_liqueur"
			&& coffee->inputs[0].amount == 2.0
			&& coffee->inputs[1].item == "whiskey_rye"
			&& coffee->inputs[1].amount == 1.0,
			"infusion: two coffee liqueur and one rye in");
		expect(std::fabs(coffee->time_minutes - 45.0) < 1e-9
			&& std::fabs(coffee->strength - 0.55) < 1e-9,
			"infusion targets: 45 minutes, 0.55 strength");
		expect(coffee->output_item == "infusion_coffee" && coffee->output_amount == 1
			&& coffee->failure_item == "swill",
			"infusion outputs itself, failures give swill");
	}
	const CraftSpec* syrup = library.find("syrup_orange");
	expect(syrup != nullptr && syrup->kind == "syrup"
		&& std::fabs(syrup->time_minutes - 20.0) < 1e-9,
		"the orange syrup steeps 20 minutes");
	expect(library.find("no_such_craft") == nullptr, "unknown crafts read null");

	// --- cross-check: every item a craft touches must be a known liquid ---------------
	{
		LiquidLibrary liquids;
		std::vector<std::string> errors;
		expect(liquids.load_directory(lc_test::repo_root() + "/Data/Liquids", &errors) >= 1
			&& errors.empty(),
			"Data/Liquids loads");
		for (const CraftSpec& spec : library.specs())
		{
			for (const lc::CraftInput& input : spec.inputs)
			{
				expect(liquids.find(input.item) != nullptr,
					"inputs of a craft are liquids on the shelf");
			}
			expect(liquids.find(spec.output_item) != nullptr,
				"crafted outputs are liquids too");
			expect(liquids.find(spec.failure_item) != nullptr,
				"failure items are liquids (dumpable)");
		}
	}

	// --- attempts --------------------------------------------------------------------
	if (coffee)
	{
		const CraftTuning tuning;

		// Code and Data/Balance/core.json must agree on the tuning.
		expect_near(tuning.perfect_time_minutes,
			lc_test::balance_number("perfect_time_minutes"), 1e-9,
			"perfect_time_minutes synced with core.json");
		expect_near(tuning.perfect_strength,
			lc_test::balance_number("perfect_strength"), 1e-9,
			"perfect_strength synced with core.json");
		expect_near(tuning.usable_time_minutes,
			lc_test::balance_number("usable_time_minutes"), 1e-9,
			"usable_time_minutes synced with core.json");
		expect_near(tuning.usable_strength,
			lc_test::balance_number("usable_strength"), 1e-9,
			"usable_strength synced with core.json");
		expect_near(tuning.sloppy_quality,
			lc_test::balance_number("sloppy_quality"), 1e-9,
			"sloppy_quality synced with core.json");

		// Dead on target.
		const lc::CraftResult clean = lc::attempt_craft(*coffee, 45.0, 0.55, tuning);
		expect(clean.outcome == CraftOutcome::Perfect
			&& clean.item == "infusion_coffee" && clean.amount == 1
			&& std::fabs(clean.quality - 1.0) < 1e-9,
			"on target: a clean infusion at quality 1");

		// Inside the perfect tolerances (2 min, 0.15 strength).
		const lc::CraftResult near_miss = lc::attempt_craft(*coffee, 46.5, 0.45, tuning);
		expect(near_miss.outcome == CraftOutcome::Perfect
			&& near_miss.item == "infusion_coffee",
			"within tolerances: still clean");

		// Past perfect, inside usable: the right liquid, just sloppy.
		const lc::CraftResult sloppy = lc::attempt_craft(*coffee, 49.0, 0.55, tuning);
		expect(sloppy.outcome == CraftOutcome::Usable
			&& sloppy.item == "infusion_coffee"
			&& std::fabs(sloppy.quality - tuning.sloppy_quality) < 1e-9,
			"oversteeped: usable, quality is the sloppy one");

		// Wildly off: swill, and it is still something (dumpable or cheap).
		const lc::CraftResult wreck = lc::attempt_craft(*coffee, 90.0, 1.0, tuning);
		expect(wreck.outcome == CraftOutcome::Failed
			&& wreck.item == "swill" && wreck.amount == 1
			&& std::fabs(wreck.quality) < 1e-9,
			"ruined: swill, quality 0 - dumpable, never a dead end");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{\"crafts\":[{\"kind\":\"infusion\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "missing id refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"moonshine\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "unknown kind refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "empty inputs refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":0}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "zero amount refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":0,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "zero time refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":1.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "strength above 1 refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "missing ru name refused");
	expect(refuses("{\"crafts\":[{\"id\":\"x\",\"kind\":\"infusion\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"},"
		"{\"id\":\"x\",\"kind\":\"syrup\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"inputs\":[{\"item\":\"stout\",\"amount\":1}],\"time_minutes\":10,\"strength\":0.5,"
		"\"output_item\":\"x\",\"failure_item\":\"swill\"}]}"), "duplicate id refused");
	expect(refuses("{\"crafts\":[]}"), "empty crafts refused");
	expect(refuses("not json"), "garbage refused");

	{
		CraftLibrary scratch;
		std::vector<std::string> dir_errors;
		expect(scratch.load_directory("/no/such/directory", &dir_errors) == 0
			&& !dir_errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("crafting");
}
