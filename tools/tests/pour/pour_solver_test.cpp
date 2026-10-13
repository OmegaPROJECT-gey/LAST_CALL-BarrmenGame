// LAST CALL - pour solver unit tests.
//
// Runs without Unreal: scripts/test-pour-solver.sh compiles this against
// Source/LastCall/Bar/PourSolverCore.cpp with plain g++. The same file is
// compiled as an UE Automation test later (ARCHITECTURE.md, section 10).

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Bar/PourSolverCore.h"

namespace {

	int failures = 0;
	int checks = 0;

	void expect(bool condition, const char* what)
	{
		++checks;
		if (!condition)
		{
			++failures;
			std::printf("FAIL: %s\n", what);
		}
	}

	void expect_near(double actual, double expected, double tolerance, const char* what)
	{
		++checks;
		if (std::fabs(actual - expected) > tolerance)
		{
			++failures;
			std::printf("FAIL: %s (actual %.4f, expected %.4f)\n", what, actual, expected);
		}
	}

	// Walk up to the repository root (marked by LastCall.uproject).
	std::string repo_root()
	{
		std::string dir = ".";
		for (int depth = 0; depth < 8; ++depth)
		{
			std::ifstream probe(dir + "/LastCall.uproject");
			if (probe.good())
			{
				return dir;
			}
			dir += "/..";
		}
		return ".";
	}

	// Pull one numeric value out of Data/Balance/core.json without a JSON
	// library: enough for the sync checks below.
	double balance_number(const std::string& key)
	{
		const std::string path = repo_root() + "/Data/Balance/core.json";
		std::ifstream file(path);
		if (!file)
		{
			return -1.0;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		const std::string text = buffer.str();
		const std::string needle = "\"" + key + "\"";
		const std::size_t at = text.find(needle);
		if (at == std::string::npos)
		{
			return -1.0;
		}
		const std::size_t colon = text.find(':', at);
		if (colon == std::string::npos)
		{
			return -1.0;
		}
		try
		{
			return std::stod(text.substr(colon + 1));
		}
		catch (...)
		{
			return -1.0;
		}
	}

	lc::RecipeSpec noir_recipe()
	{
		lc::RecipeSpec recipe;
		recipe.id = "signature_noir";
		recipe.continuity_break_s = 4.0;
		recipe.steps = {
			{ "whiskey_rye", 0.35, 0.75 },
			{ "coffee_liqueur", 0.25, 0.6 },
			{ "orange_oil", 0.1, 0.3 },
		};
		return recipe;
	}

	std::vector<lc::PourSample> perfect_pour()
	{
		// Flow held at the middle of every window, 30 Hz, no pauses.
		std::vector<lc::PourSample> samples;
		double t = 0.0;
		const double dt = 1.0 / 30.0;
		auto push = [&](const char* ingredient, double flow, int count) {
			for (int i = 0; i < count; ++i)
			{
				samples.push_back({ t, flow, ingredient });
				t += dt;
			}
		};
		push("whiskey_rye", 0.55, 30);
		push("coffee_liqueur", 0.42, 25);
		push("orange_oil", 0.20, 15);
		return samples;
	}

} // namespace

int main()
{
	const lc::PourTuning tuning;

	// --- flow model ----------------------------------------------------------
	expect_near(lc::normalized_flow_from_tilt(5.0, 1.0, tuning), 0.0, 1e-9,
		"below min tilt the bottle pours nothing");
	expect_near(lc::normalized_flow_from_tilt(90.0, 1.0, tuning),
		lc::normalized_flow_from_tilt(70.0, 1.0, tuning), 1e-9,
		"flow saturates at max tilt");
	expect(lc::normalized_flow_from_tilt(30.0, 1.0, tuning)
			< lc::normalized_flow_from_tilt(50.0, 1.0, tuning),
		"flow grows with tilt");
	expect(lc::normalized_flow_from_tilt(50.0, 1.0, tuning)
			< lc::normalized_flow_from_tilt(69.0, 1.0, tuning),
		"flow keeps growing until saturation");
	expect_near(lc::normalized_flow_from_tilt(50.0, 2.0, tuning),
		0.5 * lc::normalized_flow_from_tilt(50.0, 1.0, tuning), 1e-9,
		"double viscosity halves the flow");
	expect_near(lc::flow_ml_per_s(1.0, tuning), tuning.base_flow_ml_per_s, 1e-9,
		"normalized flow 1 is the base flow");

	// --- glass integration ---------------------------------------------------
	{
		lc::GlassState glass;
		glass.capacity_ml = 100.0;
		const double dt = 1.0 / 30.0;
		for (int i = 0; i < 60; ++i) // 2 s at full flow = 120 ml into a 100 ml glass
		{
			lc::integrate(glass, 1.0, dt, tuning);
		}
		expect_near(glass.volume_ml, 100.0, 1e-6, "glass fills to capacity, no further");
		expect(glass.spill_ml > 0.0, "overpour becomes spill");
		expect(glass.overflowed, "overflow flag set");
		expect_near(glass.spill_ml, 20.0, 1.0, "spill is roughly the excess");
	}
	{
		lc::GlassState glass;
		glass.capacity_ml = 150.0;
		for (int i = 0; i < 30; ++i) // 1 s at full flow = 60 ml
		{
			lc::integrate(glass, 1.0, 1.0 / 30.0, tuning);
		}
		expect(!glass.overflowed && glass.spill_ml == 0.0, "no spill inside capacity");
		expect_near(glass.volume_ml, 60.0, 1e-6, "volume tracks the incoming flow");
	}

	// --- combo evaluation ----------------------------------------------------
	{
		const auto result = lc::evaluate_combo(noir_recipe(), perfect_pour());
		expect(result.quality == lc::ComboQuality::Critical, "perfect pour is a critical combo");
		expect(result.steps_matched == 3, "all three steps matched");
		expect(!result.continuity_broken, "no continuity break in a steady pour");
	}
	{
		auto samples = perfect_pour();
		for (auto& sample : samples)
		{
			sample.ingredient = "vodka"; // wrong bottle entirely
		}
		const auto result = lc::evaluate_combo(noir_recipe(), samples);
		expect(result.quality == lc::ComboQuality::Ruined, "wrong ingredient ruins the drink");
		expect(result.wrong_ingredient, "wrong ingredient reported");
	}
	{
		auto samples = perfect_pour();
		for (auto& sample : samples)
		{
			sample.flow = 0.98; // pouring like a fire hose
		}
		const auto result = lc::evaluate_combo(noir_recipe(), samples);
		expect(result.quality == lc::ComboQuality::Ruined, "speed far outside the window ruins it");
		expect(result.steps_matched == 0, "no step matches at wild speed");
	}
	{
		auto samples = perfect_pour();
		// Insert a 6 s pause between step 1 and step 2 (recipe allows 4).
		std::vector<lc::PourSample> paused;
		for (const auto& sample : samples)
		{
			if (sample.ingredient == "coffee_liqueur" && paused.back().ingredient == "whiskey_rye")
			{
				paused.back().time_s += 6.0; // a hole larger than the break window
			}
			paused.push_back(sample);
		}
		const auto result = lc::evaluate_combo(noir_recipe(), paused);
		expect(result.continuity_broken, "a pause longer than the window breaks continuity");
		expect(result.quality != lc::ComboQuality::Critical, "a broken combo never lands critical");
	}
	{
		auto samples = perfect_pour();
		samples.push_back({ samples.back().time_s + 1.0 / 30.0, 0.5, "gin" });
		const auto result = lc::evaluate_combo(noir_recipe(), samples);
		expect(result.quality == lc::ComboQuality::Ruined, "an extra ingredient after the recipe ruins it");
	}

	// --- balance sync (Data/Balance/core.json <-> PourTuning defaults) -------
	expect_near(balance_number("min_angle_deg"), tuning.min_angle_deg, 1e-9,
		"min_angle_deg in sync with Data/Balance/core.json");
	expect_near(balance_number("max_angle_deg"), tuning.max_angle_deg, 1e-9,
		"max_angle_deg in sync with Data/Balance/core.json");
	expect_near(balance_number("viscosity_default"), tuning.viscosity_default, 1e-9,
		"viscosity_default in sync with Data/Balance/core.json");
	expect_near(balance_number("overflow_spill_threshold"), tuning.overflow_spill_threshold, 1e-9,
		"overflow_spill_threshold in sync with Data/Balance/core.json");
	expect_near(balance_number("flow_ml_per_s"), tuning.base_flow_ml_per_s, 1e-9,
		"flow_ml_per_s in sync with Data/Balance/core.json");

	std::printf("%d checks, %d failure(s)\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
