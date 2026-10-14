// LAST CALL - balance core unit tests.
// Data/Balance/core.json must parse through lc::load_balance and land on
// exactly the compiled defaults the other suites pin their tuning to:
// the engine reads the same numbers the tests check.
// License: MIT (LICENSE).

#include <string>

#include "Core/BalanceCore.h"
#include "Core/SaveStateCore.h"
#include "test_support.h"

using lc_test::expect;
using lc_test::expect_near;

namespace
{
	// A document with every section the loader demands, and only those.
	const char* kMinimal =
		"{\"phases\":{\"morning_seconds\":1.5,\"shift_seconds\":0,\"night_seconds\":0,"
		"\"summary_seconds\":2},"
		"\"pour\":{\"flow_ml_per_s\":50,\"overflow_spill_threshold\":1.0,"
		"\"flow_curve\":{\"min_angle_deg\":5,\"max_angle_deg\":60,\"viscosity_default\":1.0}},"
		"\"economy\":{\"starting_money\":500}}";

	// Swap one substring of the minimal document; a miss leaves it intact,
	// which the refusal checks would notice as an unexpected success.
	std::string variant(const std::string& from, const std::string& to)
	{
		std::string doc = kMinimal;
		const std::size_t at = doc.find(from);
		if (at != std::string::npos)
		{
			doc.replace(at, from.size(), to);
		}
		return doc;
	}

	bool refused(const std::string& doc)
	{
		lc::BalanceData data;
		std::string error;
		return !lc::load_balance(doc, data, &error) && !error.empty();
	}
} // namespace

int main()
{
	// --- the real file parses and lands on the compiled defaults --------
	{
		lc::BalanceData data;
		std::string error;
		const std::string path = lc_test::repo_root() + "/Data/Balance/core.json";
		expect(lc::load_balance_file(path, data, &error) && error.empty(),
			"Data/Balance/core.json loads clean");

		const lc::DayScheduleSpec phases;
		expect_near(data.phases.morning_seconds, phases.morning_seconds, 1e-9,
			"phases: morning matches the code default");
		expect_near(data.phases.shift_seconds, phases.shift_seconds, 1e-9,
			"phases: shift matches the code default");
		expect_near(data.phases.night_seconds, phases.night_seconds, 1e-9,
			"phases: night matches the code default");
		expect_near(data.phases.summary_seconds, phases.summary_seconds, 1e-9,
			"phases: summary matches the code default");
		expect_near(data.phases.morning_seconds,
			lc_test::balance_number("morning_seconds"), 1e-9,
			"phases: morning matches the raw file");

		const lc::PourTuning pour;
		expect_near(data.pour.min_angle_deg, pour.min_angle_deg, 1e-9,
			"pour: min angle matches the code default");
		expect_near(data.pour.max_angle_deg, pour.max_angle_deg, 1e-9,
			"pour: max angle matches the code default");
		expect_near(data.pour.viscosity_default, pour.viscosity_default, 1e-9,
			"pour: viscosity matches the code default");
		expect_near(data.pour.base_flow_ml_per_s, pour.base_flow_ml_per_s, 1e-9,
			"pour: flow matches the code default");
		expect_near(data.pour.overflow_spill_threshold, pour.overflow_spill_threshold,
			1e-9, "pour: spill threshold matches the code default");
		expect_near(data.pour.base_flow_ml_per_s,
			lc_test::balance_number("flow_ml_per_s"), 1e-9,
			"pour: flow matches the raw file");

		const lc::SaveState fresh;
		expect(data.starting_money == fresh.money,
			"economy: starting money matches the SaveState default");
		expect_near(static_cast<double>(data.starting_money),
			lc_test::balance_number("starting_money"), 1e-9,
			"economy: starting money matches the raw file");
	}

	// --- a minimal document ---------------------------------------------
	{
		lc::BalanceData data;
		std::string error;
		expect(lc::load_balance(kMinimal, data, &error) && error.empty(),
			"a minimal document loads");
		expect_near(data.phases.morning_seconds, 1.5, 1e-9, "minimal: morning parsed");
		expect_near(data.phases.summary_seconds, 2.0, 1e-9, "minimal: summary parsed");
		expect_near(data.pour.min_angle_deg, 5.0, 1e-9, "minimal: pour angle parsed");
		expect_near(data.pour.base_flow_ml_per_s, 50.0, 1e-9, "minimal: pour flow parsed");
		expect(data.starting_money == 500, "minimal: starting money parsed");

		// Unknown sections are the schema's business, not the loader's.
		expect(lc::load_balance(variant("{\"phases",
			"{\"zzz\":{\"whatever\":1},\"phases"), data, &error),
			"unknown sections are ignored");
	}

	// --- refusals --------------------------------------------------------
	{
		expect(refused("not json at all"), "garbage is refused");
		expect(refused("[1, 2, 3]"), "a non-object document is refused");
		expect(refused("{}"), "an empty document is refused");

		expect(refused("{\"phases\":{\"morning_seconds\":1,\"shift_seconds\":0,"
			"\"night_seconds\":0,\"summary_seconds\":2}}"),
			"phases without pour is refused");
		expect(refused("{\"pour\":{\"flow_ml_per_s\":50,\"overflow_spill_threshold\":1.0,"
			"\"flow_curve\":{\"min_angle_deg\":5,\"max_angle_deg\":60,"
			"\"viscosity_default\":1.0}},\"economy\":{\"starting_money\":500}}"),
			"pour without phases is refused");
		expect(refused(variant("\"flow_curve\":", "\"no_curve\":")),
			"a missing flow curve is refused");
		expect(refused(variant("\"flow_ml_per_s\":50,", "")),
			"a missing flow rate is refused");
		expect(refused(variant("\"starting_money\":500", "")),
			"a missing starting money is refused");

		expect(refused(variant("morning_seconds\":1.5", "morning_seconds\":-1")),
			"a negative duration is refused");
		expect(refused(variant("min_angle_deg\":5", "min_angle_deg\":60")),
			"min angle >= max angle is refused");
		expect(refused(variant("viscosity_default\":1.0", "viscosity_default\":0")),
			"a zero viscosity is refused");
		expect(refused(variant("starting_money\":500", "starting_money\":-5")),
			"negative starting money is refused");
		expect(refused(variant("starting_money\":500", "starting_money\":\"lots\"")),
			"a non-integer starting money is refused");
	}

	return lc_test::report("balance core");
}
