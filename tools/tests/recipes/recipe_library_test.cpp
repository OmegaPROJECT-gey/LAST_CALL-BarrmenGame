// LAST CALL - recipe library core unit tests.
//
// Runs without Unreal: scripts/test-recipes.sh compiles this against the
// cores it depends on (pour solver, order effects, JSON reader) with g++.

#include "Bar/RecipeLibraryCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::ComboQuality;
	using lc::PourSample;
	using lc::RecipeLibrary;
	using lc::RecipeRecord;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		RecipeRecord scratch;
		std::string error;
		return !lc::load_recipe(text, scratch, &error) && !error.empty();
	}

	// Flow held mid-window at 30 Hz per step, straight from the recipe.
	std::vector<PourSample> perfect_pour(const RecipeRecord& recipe)
	{
		std::vector<PourSample> samples;
		double t = 0.0;
		const double dt = 1.0 / 30.0;
		for (std::size_t i = 0; i < recipe.steps.size(); ++i)
		{
			const double mid = 0.5 * (recipe.steps[i].flow_min + recipe.steps[i].flow_max);
			const int count = 20;
			for (int j = 0; j < count; ++j)
			{
				samples.push_back({ t, mid, recipe.steps[i].ingredient });
				t += dt;
			}
		}
		return samples;
	}

} // namespace

int main()
{
	// --- the real file on disk -----------------------------------------------------
	{
		const std::string text = read_repo_file("Data/Recipes/signature_noir.json");
		RecipeRecord recipe;
		std::string error;
		expect(lc::load_recipe(text, recipe, &error), "Data/Recipes/signature_noir.json loads");
		expect(recipe.id == "signature_noir" && recipe.glass == "rocks", "id and glass");
		expect(recipe.steps.size() == 3 && recipe.layers.size() == 3, "three steps, three layers");
		expect(recipe.steps[0].ingredient == "whiskey_rye"
			&& recipe.steps[1].ingredient == "coffee_liqueur"
			&& recipe.steps[2].ingredient == "orange_oil", "ingredients in order");
		expect_near(recipe.steps[0].flow_min, 0.35, 1e-9, "first window low");
		expect_near(recipe.steps[0].flow_max, 0.75, 1e-9, "first window high");
		expect(recipe.layers[0] == "base" && recipe.layers[1] == "modifier"
			&& recipe.layers[2] == "accent", "layers in order");
		expect_near(recipe.continuity_break_s, 4.0, 1e-9, "continuity window");
		expect_near(recipe.effects.mood_bonus, 0.25, 1e-9, "mood bonus");
		expect_near(recipe.effects.tip_multiplier, 1.4, 1e-9, "tip multiplier");
		expect_near(recipe.effects.xp_multiplier, 1.2, 1e-9, "xp multiplier");
		expect(recipe.act1_hook, "the story hook flag reads");

		// Data -> solver, end to end: a steady pour of this recipe is critical.
		const lc::RecipeSpec spec = lc::to_solver_spec(recipe);
		const lc::ComboResult result = lc::evaluate_combo(spec, perfect_pour(recipe));
		expect(result.quality == ComboQuality::Critical,
			"a steady pour of the loaded recipe earns a critical combo");
		expect(result.steps_matched == 3, "all three steps matched");
	}

	// --- strict refusals --------------------------------------------------------------
	expect(refuses("{}"), "empty document refused");
	expect(refuses("{\"id\":\"x\"}"), "missing glass refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\"}"), "missing steps refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\",\"steps\":[]}"), "empty steps refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\","
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[0.1,0.2,0.3]}]}"),
		"three-element window refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\","
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[0.8,0.3]}]}"),
		"min above max refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\","
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[0.5,1.5]}]}"),
		"window above 1 refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\","
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[\"lo\",\"hi\"]}]}"),
		"non-numeric window refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\",\"continuity_break_seconds\":-1,"
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[0.1,0.2]}]}"),
		"negative continuity refused");
	expect(refuses("{\"id\":\"x\",\"glass\":\"rocks\",\"effects\":{\"tip_multiplier\":0},"
		"\"steps\":[{\"ingredient\":\"a\",\"flow_window\":[0.1,0.2]}]}"),
		"zero tip multiplier refused");
	expect(refuses("not json at all"), "garbage refused");

	// --- library behavior ---------------------------------------------------------------
	{
		RecipeLibrary library;
		std::vector<std::string> errors;
		const int loaded = library.load_directory(
			lc_test::repo_root() + "/Data/Recipes", &errors);
		expect(loaded >= 1 && errors.empty(), "Data/Recipes loads clean");
		const RecipeRecord* noir = library.find("signature_noir");
		expect(noir != nullptr, "signature_noir is findable");
		expect(!library.find("no_such_recipe"), "unknown ids read null");
		const std::vector<std::string> ids = library.ids();
		expect(!ids.empty() && ids[0] == "signature_noir", "ids in load order");

		// Duplicate ids are refused with an error, not silently dropped.
		const std::string text = read_repo_file("Data/Recipes/signature_noir.json");
		errors.clear();
		expect(!library.add(text, &errors) && !errors.empty()
			&& errors.back().find("duplicate") != std::string::npos,
			"a duplicate id is refused with an error");
	}
	{
		RecipeLibrary library;
		std::vector<std::string> errors;
		expect(library.load_directory("/no/such/directory", &errors) == 0 && !errors.empty(),
			"a missing directory reports an error");

		// A step without a layer defaults to "base".
		errors.clear();
		expect(library.add("{\"id\":\"x\",\"glass\":\"rocks\",\"steps\":"
			"[{\"ingredient\":\"a\",\"flow_window\":[0.1,0.2]}]}", &errors), "minimal recipe loads");
		const RecipeRecord* x = library.find("x");
		expect(x && x->layers.size() == 1 && x->layers[0] == "base",
			"a missing layer defaults to base");
	}

	return lc_test::report("recipe library");
}
