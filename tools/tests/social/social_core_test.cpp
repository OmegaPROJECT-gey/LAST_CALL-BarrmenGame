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

	expect(library.reactions().size() == 68, "sixty-eight reaction rows");

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
			int count; // factions moved by this flag (some rows move two)
		};
		const Expected expected[] = {
			{"harris_told_breakin", "syndicate", -1.0, 1},
			{"harris_withheld_breakin", "syndicate", 1.0, 1},
			{"harris_quiet_report", "station", 1.0, 1},
			{"kane_first_fact_heard", "civilian", 1.0, 1},
			{"kane_knew_uncle", "bureau", 1.0, 1},
			{"kane_price_read", "civilian", 1.0, 1},
			{"shadow_deferred", "syndicate", -1.0, 1},
			// Beat 4's habit evenings: the scenes move flags only, the
			// table moves standing (C3 - baselines for the three gangs).
			{"syndicate_drop_taken", "syndicate", 1.0, 1},
			{"syndicate_drop_asked", "syndicate", -1.0, 1},
			{"syndicate_drop_refused", "syndicate", -1.0, 1},
			{"ring_delivery_accepted", "ring", 1.0, 1},
			{"ring_delivery_asked", "ring", -1.0, 1},
			{"ring_delivery_refused", "ring", -1.0, 1},
			{"station_understood", "station", 1.0, 1},
			{"station_blind_eye_refused", "station", -1.0, 1},
			// The three completed regular arcs: favor then fork (C3 batch 4).
			{"vera_launder_helped", "syndicate", 1.0, 1},
			{"vera_launder_refused", "syndicate", -1.0, 1},
			{"vera_book_burned", "syndicate", 1.0, 1},
			{"vera_book_sold", "syndicate", -2.0, 2},
			{"vera_book_sold", "bureau", 1.0, 2},
			{"sal_meeting_hosted", "ring", 1.0, 1},
			{"sal_meeting_refused", "ring", -1.0, 1},
			{"sal_crate_week", "ring", 1.0, 1},
			{"sal_crate_refused", "ring", -2.0, 1},
			{"agnes_lease_signed", "station", 1.0, 1},
			{"agnes_lease_refused", "station", -1.0, 1},
			{"agnes_covered", "station", 1.0, 1},
			{"agnes_envelope_sold", "station", -2.0, 2},
			{"agnes_envelope_sold", "bureau", 1.0, 2},
			// Wave 2 arcs: Dana (press/Bureau), Mack (door/Syndicate),
			// Denny (the ex-partner/The Room) - C3 batch 5.
			{"dana_quote_given", "civilian", 1.0, 1},
			{"mack_watch_ok", "civilian", 1.0, 1},
			{"denny_welcomed", "civilian", 1.0, 1},
			{"denny_wounded", "civilian", -1.0, 1},
			{"dana_uncle_talked", "bureau", 1.0, 1},
			{"dana_uncle_refused", "bureau", -1.0, 1},
			{"mack_sponsored", "syndicate", 1.0, 1},
			{"mack_sponsor_refused", "syndicate", -1.0, 1},
			{"denny_hired", "civilian", 1.0, 1},
			{"denny_refused", "civilian", -1.0, 1},
			{"dana_story_killed", "syndicate", 1.0, 1},
			{"dana_story_printed", "bureau", 1.0, 2},
			{"dana_story_printed", "civilian", -1.0, 2},
			{"dana_photo_bought", "syndicate", 1.0, 1},
			{"mack_named", "syndicate", -2.0, 2},
			{"mack_named", "bureau", 1.0, 2},
			{"mack_defended", "syndicate", 1.0, 1},
			{"denny_sold", "bureau", 1.0, 1},
			{"denny_protected", "civilian", 1.0, 1},
			// Wave 3 arcs: Amos (the preacher / The Room + Station),
			// Ivy (the hacker / The Ring vs the Bureau),
			// Walter (the uncle's partner / the archive index) - C3 batch 6.
			{"amos_sermon_ok", "civilian", 1.0, 1},
			{"amos_heckled", "civilian", -1.0, 1},
			{"amos_shelter_ok", "civilian", 1.0, 1},
			{"amos_shelter_refused", "civilian", -1.0, 1},
			{"amos_log_broken", "station", -1.0, 2},
			{"amos_log_broken", "civilian", 1.0, 2},
			{"amos_log_kept", "station", 1.0, 1},
			{"amos_log_mine", "civilian", -1.0, 1},
			{"ivy_wifi_ok", "ring", 1.0, 1},
			{"ivy_warned", "ring", -1.0, 1},
			{"ivy_drop_held", "ring", 1.0, 1},
			{"ivy_drop_refused", "ring", -1.0, 1},
			{"ivy_drop_opened", "ring", -2.0, 1},
			{"ivy_hidden", "ring", -2.0, 2},
			{"ivy_hidden", "civilian", 1.0, 2},
			{"ivy_handed", "ring", 1.0, 1},
			{"ivy_flipped", "bureau", 1.0, 2},
			{"ivy_flipped", "ring", -2.0, 2},
			{"walter_signature_poured", "civilian", 1.0, 1},
			{"walter_altered", "civilian", -1.0, 1},
			{"walter_index_sold", "bureau", 2.0, 1},
			{"walter_index_burned", "syndicate", -1.0, 4},
			{"walter_index_burned", "ring", -1.0, 4},
			{"walter_index_burned", "station", -1.0, 4},
			{"walter_index_burned", "bureau", -1.0, 4},
			{"walter_index_kept", "civilian", 1.0, 1},
		};
		for (const Expected& want : expected)
		{
			const lc::Reaction* row = library.reaction_for(want.flag);
			bool found = false;
			int moved = 0;
			if (row)
			{
				moved = static_cast<int>(row->standing.size());
				for (const auto& s : row->standing)
				{
					if (s.first == want.faction && s.second == want.delta)
					{
						found = true;
					}
				}
			}
			expect(row != nullptr && found && moved == want.count
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
