// LAST CALL - social consequences core unit tests.
//
// Runs without Unreal: scripts/test-social.sh compiles this against the
// social core, the consequence log, the JSON reader and the harness.
// Drives the real file Data/Factions/factions.json.

#include "Social/SocialCore.h"

#include "Core/ConsequenceLogCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;

	using lc::ConsequenceLog;
	using lc::FactionLibrary;
	using lc::StandingLedger;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		FactionLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real file on disk ---------------------------------------------------------
	FactionLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Factions/factions.json"), &error),
		"Data/Factions/factions.json loads");
	expect(library.ids().size() == 5, "five factions on the books");

	// The same five the customer schema's faction enum allows.
	expect(library.find("syndicate") != nullptr, "syndicate exists");
	expect(library.find("ring") != nullptr, "ring exists");
	expect(library.find("station") != nullptr, "station exists");
	{
		const lc::FactionSpec* spec = library.find("bureau");
		expect(spec != nullptr, "bureau exists");
		if (spec)
		{
			expect(!spec->name_en.empty() && !spec->name_ru.empty(),
				"bureau is bilingual");
			expect(spec->initial_standing == 0.0, "bureau starts neutral");
		}
	}
	expect(library.find("no_such_faction") == nullptr, "unknown factions read null");

	expect(library.reactions().size() == 19, "nineteen reaction rows");

	const lc::Reaction* case_reaction = library.reaction_for("heard_harris_case");
	expect(case_reaction != nullptr, "the detective's case has a reaction");
	if (case_reaction)
	{
		expect(case_reaction->standing.size() == 1
			&& case_reaction->standing[0].first == "syndicate"
			&& case_reaction->standing[0].second == -1.0,
			"hearing the case cools the Syndicate");
		expect(!case_reaction->note_en.empty() && !case_reaction->note_ru.empty(),
			"reaction notes are bilingual");
	}

	// The city's delayed reactions (C3 batch 1) - each complements the
	// direct faction_standing effects the scenes fire themselves: the NPC
	// reacts now, the room reacts days later (PHILOSOPHY 3.2).
	{
		struct Expected
		{
			const char* flag;
			const char* faction;
			double delta;
		};
		const Expected expected[] = {
			{"harris_told_breakin", "syndicate", -1.0},
			{"harris_withheld_breakin", "syndicate", 1.0},
			{"harris_quiet_report", "station", 1.0},
			{"kane_first_fact_heard", "civilian", 1.0},
			{"kane_knew_uncle", "bureau", 1.0},
			{"kane_price_read", "civilian", 1.0},
			{"shadow_deferred", "syndicate", -1.0},
			// Beat 4's habit evenings: the scenes move flags only, the
			// table moves standing (C3 - baselines for the three gangs).
			{"syndicate_drop_taken", "syndicate", 1.0},
			{"syndicate_drop_asked", "syndicate", -1.0},
			{"syndicate_drop_refused", "syndicate", -1.0},
			{"ring_delivery_accepted", "ring", 1.0},
			{"ring_delivery_asked", "ring", -1.0},
			{"ring_delivery_refused", "ring", -1.0},
			{"station_understood", "station", 1.0},
			{"station_blind_eye_refused", "station", -1.0},
		};
		for (const Expected& want : expected)
		{
			const lc::Reaction* row = library.reaction_for(want.flag);
			expect(row != nullptr && row->standing.size() == 1
				&& row->standing[0].first == want.faction
				&& row->standing[0].second == want.delta
				&& !row->note_en.empty() && !row->note_ru.empty(),
				want.flag);
		}
	}
	expect(library.reaction_for("no_such_flag") == nullptr, "unknown flags read null");

	// --- ledger ------------------------------------------------------------------------
	{
		StandingLedger ledger(library);
		expect(ledger.standing("bureau") == 0.0, "the ledger starts neutral");

		expect(ledger.apply_flag("heard_harris_case") == 1, "one faction moves");
		expect(ledger.standing("syndicate") == -1.0, "Syndicate standing dropped");
		expect(ledger.applied("heard_harris_case"), "the flag is remembered");

		expect(ledger.apply_flag("heard_harris_case") == 0, "the reaction fires once");
		expect(ledger.standing("syndicate") == -1.0, "standing did not stack");

		expect(ledger.apply_flag("no_such_flag") == 0, "flags without tables do nothing");

		ledger.set_standing("bureau", 2.0);
		expect(ledger.standing("bureau") == 2.0, "standing can be set (saves/restores)");
		expect(ledger.standing("nope") == 0.0, "unknown factions read 0");
	}

	// --- two-faction reaction ------------------------------------------------------------
	{
		StandingLedger ledger(library);
		expect(ledger.apply_flag("ignored_harris_meeting") == 2, "two factions move at once");
		expect(ledger.standing("bureau") == -1.0 && ledger.standing("civilian") == -1.0,
			"ignoring a guest costs Bureau and Room");
	}

	// --- the ledger eats the consequence log ---------------------------------------------
	{
		ConsequenceLog log;
		expect(log.record("heard_harris_meeting", 1, "Shift"), "log records hearing it out");
		expect(log.record("poured_harris_before_talk", 1, "Shift"), "log records the pour");
		expect(log.record("poured_harris_before_talk", 3, "Shift"),
			"another day, same flag - new entry");

		StandingLedger ledger(library);
		const int moved = ledger.apply_log(log);
		expect(moved == 2, "two reactions fired across two distinct flags");
		expect(ledger.standing("bureau") == 1.0, "the Bureau warms up");
		expect(ledger.standing("civilian") == 1.0, "the Room warms up");

		expect(ledger.apply_log(log) == 0, "replaying the log changes nothing");
		expect(ledger.standing("bureau") == 1.0, "standing did not stack");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing factions refused");
	expect(refuses("{\"factions\":{}}"), "empty factions refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\"},"
		"\"initial_standing\":0}}}"), "missing ru name refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\",\"ru\":\"B\"}}}}"),
		"missing initial_standing refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\",\"ru\":\"B\"},"
		"\"initial_standing\":0}},"
		"\"reactions\":[{\"flag\":\"x\",\"standing\":{\"nope\":1},"
		"\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"reaction to an unknown faction refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\",\"ru\":\"B\"},"
		"\"initial_standing\":0}},"
		"\"reactions\":[{\"flag\":\"x\",\"standing\":{\"bureau\":1},"
		"\"note\":{\"en\":\"n\",\"ru\":\"n\"}},"
		"{\"flag\":\"x\",\"standing\":{\"bureau\":-1},"
		"\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"duplicate reaction flag refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\",\"ru\":\"B\"},"
		"\"initial_standing\":0}},"
		"\"reactions\":[{\"flag\":\"x\",\"standing\":{},"
		"\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"empty standing map refused");
	expect(refuses("{\"factions\":{\"bureau\":{\"display_name\":{\"en\":\"B\",\"ru\":\"B\"},"
		"\"initial_standing\":0}},"
		"\"reactions\":[{\"flag\":\"x\",\"standing\":{\"bureau\":1},"
		"\"note\":{\"en\":\"n\"}}]}"),
		"half-translated note refused");
	expect(refuses("not json"), "garbage refused");

	return lc_test::report("social consequences");
}
