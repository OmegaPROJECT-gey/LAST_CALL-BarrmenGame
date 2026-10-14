// LAST CALL - liquid core unit tests.
//
// Runs without Unreal: scripts/test-liquids.sh compiles this against the
// liquid core, the pour solver, the JSON reader and the test harness.
// Drives the real file Data/Liquids/liquids.json.

#include "Bar/LiquidCore.h"
#include "Bar/PourSolverCore.h"

#include "test_support.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::LayeredGlass;
	using lc::LiquidLibrary;
	using lc::LiquidSpec;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		LiquidLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

	// One liquid document with the given entry body (a full JSON object body).
	std::string entry(const std::string& body)
	{
		return "{\"liquids\":{\"x\":{" + body + "}}}";
	}

	const std::string kOkBody =
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":1.0,\"density\":1.0,\"foam\":0.0,\"color\":\"#123456\"";

	// `kOkBody` with one field replaced (fields appear exactly once each).
	std::string body_without(const std::string& dropped_key)
	{
		std::string body = kOkBody;
		const std::size_t at = body.find("\"" + dropped_key + "\"");
		if (at == std::string::npos)
		{
			return body;
		}
		const std::size_t end = body.find(',', at);
		return body.substr(0, at) + (end == std::string::npos ? "" : body.substr(end + 1));
	}

} // namespace

int main()
{
	// --- the real file on disk -----------------------------------------------------
	LiquidLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Liquids/liquids.json"), &error),
		"Data/Liquids/liquids.json loads");
	expect(library.ids().size() == 8, "eight liquids on tap (five stock + three crafted)");

	const LiquidSpec* whiskey = library.find("whiskey_rye");
	expect(whiskey != nullptr, "whiskey_rye is stocked");
	if (whiskey)
	{
		expect_near(whiskey->viscosity, 1.2, 1e-9, "whiskey viscosity");
		expect_near(whiskey->density, 0.94, 1e-9, "whiskey density");
		expect(whiskey->foam_fraction == 0.0 && whiskey->color == "#a8631e",
			"whiskey does not foam, right colour");
		expect(whiskey->name_ru == "\u0420\u0436\u0430\u043d\u043e\u0439 \u0432\u0438\u0441\u043a\u0438",
			"bilingual name reads");
	}
	const LiquidSpec* stout = library.find("stout");
	expect(stout != nullptr, "stout is stocked");
	expect(stout != nullptr && std::fabs(stout->foam_fraction - 0.35) < 1e-9,
		"stout foams 35%");
	expect(library.find("no_such_liquid") == nullptr, "unknown ids read null");

	{
		LiquidLibrary directory_library;
		std::vector<std::string> errors;
		const int loaded = directory_library.load_directory(
			lc_test::repo_root() + "/Data/Liquids", &errors);
		expect(loaded >= 5 && errors.empty(), "the Liquids directory loads clean");
	}

	// --- layering: heavy sinks, light floats -----------------------------------------
	{
		LayeredGlass glass;
		glass.configure(150.0, lc_test::balance_number("foam_decay_per_s"));

		const LiquidSpec* coffee = library.find("coffee_liqueur");
		const LiquidSpec* orange = library.find("orange_oil");
		expect(whiskey && coffee && orange, "the three recipe liquids are found");

		expect_near(glass.pour(*whiskey, 50.0, 0.0), 50.0, 1e-9, "whiskey pour taken");
		expect_near(glass.pour(*coffee, 30.0, 1.0), 30.0, 1e-9, "liqueur pour taken");
		expect_near(glass.pour(*orange, 20.0, 2.0), 20.0, 1e-9, "oil pour taken");

		const auto& layers = glass.layers();
		expect(layers.size() == 3, "three strata");
		expect(layers[0].liquid_id == "coffee_liqueur"
			&& layers[1].liquid_id == "whiskey_rye"
			&& layers[2].liquid_id == "orange_oil",
			"bottom -> top: dense liqueur, whiskey, floating oil");
		expect_near(glass.volume_ml(), 100.0, 1e-9, "100 ml of liquid");

		// Same liquid merges into its own stratum instead of stacking.
		glass.pour(*whiskey, 30.0, 3.0);
		expect(glass.layers().size() == 3
			&& std::fabs(glass.volume_ml() - 130.0) < 1e-9,
			"re-pouring whiskey merges into its stratum");
		expect(glass.spills().empty(), "no spills below capacity");
	}

	// --- overflow becomes a recorded spill --------------------------------------------
	{
		LayeredGlass glass;
		glass.configure(150.0, lc_test::balance_number("foam_decay_per_s"));
		const LiquidSpec* coffee = library.find("coffee_liqueur");
		glass.pour(*whiskey, 120.0, 0.0);
		const double taken = glass.pour(*coffee, 50.0, 4.5);
		expect_near(taken, 30.0, 1e-9, "only the room that is left is taken");
		expect(glass.spills().size() == 1, "one spill event recorded");
		if (!glass.spills().empty())
		{
			expect_near(glass.spills()[0].volume_ml, 20.0, 1e-9, "20 ml spilled");
			expect_near(glass.spills()[0].time_s, 4.5, 1e-9, "spill stamped in time");
			expect(glass.spills()[0].liquid_id == "coffee_liqueur", "spill names the liquid");
		}
		expect(glass.pour(*whiskey, 10.0, 5.0) == 0.0 && glass.spills().size() == 2,
			"a full glass refuses everything and still records it");
	}

	// --- foam head: grows on pour, dissipates on tick -----------------------------------
	{
		LayeredGlass glass;
		const double decay = lc_test::balance_number("foam_decay_per_s");
		expect(decay > 0.0, "foam decay is tuned in Data/Balance/core.json");
		glass.configure(150.0, decay);
		const LiquidSpec* soda = library.find("soda_water");
		expect(soda != nullptr, "soda_water is stocked");
		glass.pour(*soda, 40.0, 0.0);
		expect_near(glass.foam_ml(), 20.0, 1e-9, "soda foams half its volume");
		glass.tick(1.0);
		expect_near(glass.foam_ml(), 20.0 * (1.0 - decay), 1e-6, "one second of decay");
		glass.tick(1000.0); // long enough to zero out any decay rate
		expect(glass.foam_ml() == 0.0, "eventually the head is gone");

		// Foam eats into capacity: 140 ml of soda leaves room for only 10 ml of foam.
		LayeredGlass tight;
		tight.configure(150.0, decay);
		tight.pour(*soda, 140.0, 0.0); // 140 liquid + 70 foam -> capped
		expect_near(tight.foam_ml(), 10.0, 1e-9, "foam clipped to the free capacity");
		expect_near(tight.free_ml(), 0.0, 1e-9, "no room left");
	}

	// --- viscosity feeds the pour solver ------------------------------------------------
	{
		const lc::PourTuning tuning;
		const LiquidSpec* coffee = library.find("coffee_liqueur");
		const LiquidSpec* soda = library.find("soda_water");
		const double flow_whiskey = lc::normalized_flow_from_tilt(40.0, whiskey->viscosity, tuning);
		const double flow_coffee = lc::normalized_flow_from_tilt(40.0, coffee->viscosity, tuning);
		const double flow_soda = lc::normalized_flow_from_tilt(40.0, soda->viscosity, tuning);
		expect(flow_coffee < flow_whiskey && flow_whiskey < flow_soda,
			"at the same tilt: thick liqueur < whiskey < soda");
		expect(flow_coffee > 0.0, "the thick pour still moves");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing liquids refused");
	expect(refuses("{\"liquids\":{}}"), "empty liquids refused");
	expect(refuses("{\"liquids\":{\"x\":{}}}"), "empty entry refused");
	expect(refuses(entry(body_without("viscosity"))), "missing viscosity refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":0.0,\"density\":1.0,\"foam\":0.0,\"color\":\"#123456\"")),
		"zero viscosity refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":1.0,\"density\":0.1,\"foam\":0.0,\"color\":\"#123456\"")),
		"underweight density refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":1.0,\"density\":3.0,\"foam\":0.0,\"color\":\"#123456\"")),
		"impossible density refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":1.0,\"density\":1.0,\"foam\":2.0,\"color\":\"#123456\"")),
		"foam fraction above 1 refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\"},"
		"\"viscosity\":1.0,\"density\":1.0,\"foam\":0.0,\"color\":\"#123456\"")),
		"missing ru name refused");
	expect(refuses(entry("\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"viscosity\":1.0,\"density\":1.0,\"foam\":0.0,\"color\":\"red\"")),
		"bad colour refused");
	expect(refuses("not json"), "garbage refused");

	{
		LiquidLibrary scratch;
		std::vector<std::string> errors;
		expect(scratch.add(read_repo_file("Data/Liquids/liquids.json"), &errors),
			"a document loads through add()");
		expect(!scratch.add(read_repo_file("Data/Liquids/liquids.json"), &errors)
			&& !errors.empty() && errors.back().find("duplicate") != std::string::npos,
			"a duplicate document is refused with an error");
	}
	{
		LiquidLibrary scratch;
		std::vector<std::string> errors;
		expect(scratch.load_directory("/no/such/directory", &errors) == 0 && !errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("liquid core");
}
