// LAST CALL - debt runs core unit tests.
//
// Runs without Unreal: scripts/test-debt.sh compiles this against the
// debt core, the faction library, the JSON reader and the harness.
// Drives the real files Data/Debts/debtors.json and Data/Factions.

#include "Night/DebtCore.h"

#include "Social/SocialCore.h"

#include "test_support.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::DebtAttempt;
	using lc::DebtResolution;
	using lc::DebtTuning;
	using lc::DebtorLibrary;
	using lc::DebtorRecord;
	using lc::FactionLibrary;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		DebtorLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

	DebtAttempt approach(const std::string& name, double score, bool violent = false)
	{
		DebtAttempt attempt;
		attempt.approach = name;
		attempt.score = score;
		attempt.violent = violent;
		return attempt;
	}

} // namespace

int main()
{
	// --- the real files on disk ---------------------------------------------------------
	DebtorLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Debts/debtors.json"), &error),
		"Data/Debts/debtors.json loads");
	expect(library.ids().size() == 3, "three debtors on the books");

	const DebtorRecord* pete = library.find("pete_low");
	expect(pete != nullptr, "Pete the Low is found");
	if (pete)
	{
		expect(pete->block == "docks" && pete->faction == "civilian" && pete->owed == 40
			&& pete->collateral == "pocket_watch",
			"Pete: docks, civilian, owes 40, watch is the collateral");
		expect(!pete->name_en.empty() && !pete->name_ru.empty(), "debtors are bilingual");
	}
	const DebtorRecord* ruby = library.find("ruby");
	expect(ruby != nullptr && ruby->collateral.empty() && ruby->faction == "ring",
		"Ruby owes the Ring and keeps nothing worth seizing");
	expect(library.find("no_such_debtor") == nullptr, "unknown debtors read null");

	// Cross-check: every debtor answers to a faction that exists.
	{
		FactionLibrary factions;
		expect(factions.load(read_repo_file("Data/Factions/factions.json"), &error),
			"the faction registry loads");
		for (const DebtorRecord& record : library.debtors())
		{
			expect(factions.find(record.faction) != nullptr,
				"a debtor's faction is a known faction");
		}
	}

	// --- tuning vs balance ------------------------------------------------------------------
	{
		const DebtTuning tuning;
		expect_near(tuning.partial_ratio, lc_test::balance_number("partial_ratio"), 1e-9,
			"partial_ratio synced with core.json");
		expect_near(tuning.fight_loot_ratio, lc_test::balance_number("fight_loot_ratio"), 1e-9,
			"fight_loot_ratio synced with core.json");
		expect_near(tuning.take_heat, lc_test::balance_number("take_heat"), 1e-9,
			"take_heat synced with core.json");
		expect_near(tuning.fight_heat, lc_test::balance_number("fight_heat"), 1e-9,
			"fight_heat synced with core.json");
		expect_near(tuning.violence_heat, lc_test::balance_number("violence_heat"), 1e-9,
			"violence_heat synced with core.json");
		expect_near(tuning.violence_standing, lc_test::balance_number("violence_standing"), 1e-9,
			"violence_standing synced with core.json");
	}

	if (pete)
	{
		const DebtTuning tuning;

		// Talk him dry: paid in full, quiet.
		const lc::DebtOutcome paid = lc::resolve_debt(*pete, approach("talk", 0.6), tuning);
		expect(paid.resolution == DebtResolution::Paid && paid.collected == 40
			&& std::fabs(paid.heat_delta) < 1e-9 && std::fabs(paid.standing_delta) < 1e-9,
			"a passed talk collects in full and changes nothing else");

		// Near miss: half and he remembers you kindly enough.
		const lc::DebtOutcome partial = lc::resolve_debt(*pete, approach("talk", 0.25), tuning);
		expect(partial.resolution == DebtResolution::Partial && partial.collected == 20,
			"a near miss talks out half (40 * 0.5)");

		// Whiffed: he walks.
		const lc::DebtOutcome escaped = lc::resolve_debt(*pete, approach("talk", 0.1), tuning);
		expect(escaped.resolution == DebtResolution::Escaped && escaped.collected == 0,
			"a whiffed talk collects nothing");

		// Intimidation fails and he stands up to you.
		const lc::DebtOutcome fight = lc::resolve_debt(*pete, approach("intimidate", 0.1), tuning);
		expect(fight.resolution == DebtResolution::Fight && fight.collected == 10,
			"failed intimidation: a fight, looted at 0.25 (40 * 0.25)");
		expect_near(fight.heat_delta, tuning.fight_heat, 1e-9, "a fight makes noise");

		// Intimidation passes: full, still quiet.
		const lc::DebtOutcome scared = lc::resolve_debt(*pete, approach("intimidate", 0.9), tuning);
		expect(scared.resolution == DebtResolution::Paid && scared.collected == 40,
			"a passed intimidation collects in full");

		// Taking something: partial, collateral, reported.
		const lc::DebtOutcome seized = lc::resolve_debt(*pete, approach("take", 0.0), tuning);
		expect(seized.resolution == DebtResolution::Partial && seized.collected == 20
			&& seized.collateral == "pocket_watch",
			"taking: half the debt and the watch");
		expect_near(seized.heat_delta, tuning.take_heat, 1e-9, "seizing gets reported");

		// Violence is worth money today, costs tomorrow.
		const lc::DebtOutcome armed = lc::resolve_debt(*pete, approach("talk", 0.6, true), tuning);
		expect(armed.resolution == DebtResolution::Paid && armed.collected == 40,
			"an armed talk still collects in full");
		expect_near(armed.heat_delta, tuning.violence_heat, 1e-9,
			"but doing it armed raises heat");
		expect_near(armed.standing_delta, tuning.violence_standing, 1e-9,
			"and every faction cools off");

		// Unknown approach reads as an escape.
		const lc::DebtorRecord nobody = *pete;
		const lc::DebtOutcome confused = lc::resolve_debt(nobody, approach("sing", 1.0), tuning);
		expect(confused.resolution == DebtResolution::Escaped && confused.collected == 0,
			"an unknown approach collects nothing");
	}

	// --- strict refusals -----------------------------------------------------------------------
	expect(refuses("{\"debtors\":[]}"), "empty debtors refused");
	expect(refuses("{\"debtors\":[{\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":1,"
		"\"talk_resist\":0.5,\"intimidate_resist\":0.5}]}"), "missing id refused");
	expect(refuses("{\"debtors\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":-1,"
		"\"talk_resist\":0.5,\"intimidate_resist\":0.5}]}"), "negative owed refused");
	expect(refuses("{\"debtors\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":1,"
		"\"talk_resist\":1.5,\"intimidate_resist\":0.5}]}"), "talk resistance above 1 refused");
	expect(refuses("{\"debtors\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":1,"
		"\"talk_resist\":0.5,\"intimidate_resist\":0.5}]}"), "missing ru name refused");
	expect(refuses("{\"debtors\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":1,"
		"\"talk_resist\":0.5,\"intimidate_resist\":0.5},"
		"{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"block\":\"docks\",\"faction\":\"civilian\",\"owed\":1,"
		"\"talk_resist\":0.5,\"intimidate_resist\":0.5}]}"), "duplicate id refused");
	expect(refuses("not json"), "garbage refused");

	{
		DebtorLibrary scratch;
		std::vector<std::string> dir_errors;
		expect(scratch.load_directory("/no/such/directory", &dir_errors) == 0
			&& !dir_errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("debt runs");
}
