// LAST CALL - night mode core unit tests.
//
// Runs without Unreal: scripts/test-night.sh compiles this against the
// night core, the consequence log, the JSON reader and the harness.
// Drives the real file Data/Night/activities.json.

#include "Night/NightCore.h"

#include "Core/ConsequenceLogCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::ConsequenceLog;
	using lc::NightLibrary;
	using lc::NightTuning;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		NightLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real file on disk --------------------------------------------------------
	NightLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Night/activities.json"), &error),
		"Data/Night/activities.json loads");
	expect(library.ids().size() == 3, "three night activities");

	const lc::NightActivity* to_go = library.find("orders_to_go");
	expect(to_go != nullptr && to_go->required_flag.empty(),
		"orders to go are always open");
	const lc::NightActivity* debts = library.find("debt_runs");
	expect(debts != nullptr && debts->required_flag.empty(),
		"debt runs are always open");
	const lc::NightActivity* investigations = library.find("investigations");
	expect(investigations != nullptr
		&& investigations->required_flag == "heard_harris_case",
		"investigations open after the detective's case");
	expect(to_go != nullptr && !to_go->name_en.empty() && !to_go->name_ru.empty(),
		"activities are bilingual");
	expect(library.find("no_such_activity") == nullptr, "unknown activities read null");

	// --- gating by the consequence log ---------------------------------------------------
	{
		ConsequenceLog log;
		const std::vector<std::string> fresh = lc::available_activities(library, log);
		expect(fresh.size() == 2 && fresh[0] == "orders_to_go" && fresh[1] == "debt_runs",
			"a fresh player gets to-go orders and debt runs");

		expect(log.record("heard_harris_case", 1, "Shift"), "the case gets heard");
		const std::vector<std::string> opened = lc::available_activities(library, log);
		expect(opened.size() == 3 && opened[2] == "investigations",
			"hearing the case opens investigations");

		if (investigations)
		{
			expect(lc::available(*investigations, log), "and the activity reads open");
		}
	}

	// --- death is consequences -------------------------------------------------------------
	{
		const NightTuning tuning;

		// Code and Data/Balance/core.json must agree on the tuning.
		expect_near(tuning.death_heat, lc_test::balance_number("death_heat"), 1e-9,
			"death_heat synced with core.json");
		expect_near(tuning.death_standing, lc_test::balance_number("death_standing"), 1e-9,
			"death_standing synced with core.json");
		expect_near(static_cast<double>(tuning.death_hospital_days),
			lc_test::balance_number("death_hospital_days"), 1e-9,
			"death_hospital_days synced with core.json");

		const lc::DeathConsequences dead = lc::death_consequences(tuning);
		expect_near(dead.heat, 3.0, 1e-9, "dying raises heat");
		expect_near(dead.standing, -2.0, 1e-9, "dying cools every faction");
		expect(dead.hospital_days == 2, "two days in hospital");

		expect(lc::resume_day(5, tuning) == 7, "died on day 5, back on day 7");
		expect(lc::resume_day(1, tuning) == 3, "died on day 1, back on day 3");
		expect(lc::resume_day(0, tuning) == 3, "day floors at 1 before hospital days");
	}

	// --- strict refusals -----------------------------------------------------------------------
	expect(refuses("{\"activities\":[]}"), "empty activities refused");
	expect(refuses("{\"activities\":[{\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"required_flag\":\"\"}]}"), "missing id refused");
	expect(refuses("{\"activities\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\"},"
		"\"required_flag\":\"\"}]}"), "missing ru name refused");
	expect(refuses("{\"activities\":[{\"id\":\"x\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"}}]}"), "missing required_flag refused");
	expect(refuses("{\"activities\":[{\"id\":\"x\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"required_flag\":\"Bad-Flag\"}]}"), "malformed flag id refused");
	expect(refuses("{\"activities\":[{\"id\":\"x\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},\"required_flag\":\"a\"},"
		"{\"id\":\"x\","
		"\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},\"required_flag\":\"\"}]}"),
		"duplicate activity id refused");
	expect(refuses("not json"), "garbage refused");

	{
		NightLibrary scratch;
		std::vector<std::string> dir_errors;
		expect(scratch.load_directory("/no/such/directory", &dir_errors) == 0
			&& !dir_errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("night mode");
}
