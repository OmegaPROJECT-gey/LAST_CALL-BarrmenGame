// LAST CALL - customer persona core unit tests.
//
// Runs without Unreal: scripts/test-customer-library.sh compiles this
// against the customer core, the JSON reader and the test harness.
// Drives the real file Data/Customers/harris.json.

#include "Customers/CustomerCore.h"

#include "test_support.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::CustomerLibrary;
	using lc::CustomerRecord;
	using lc::PatronRead;
	using lc::ThreatBand;
	using lc::ThreatTuning;
	using lc::TellReadiness;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		CustomerLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real file on disk -----------------------------------------------------
	CustomerLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Customers/harris.json"), &error),
		"Data/Customers/harris.json loads");
	expect(library.ids().size() == 1, "one regular on the books");

	const CustomerRecord* harris = library.find("harris");
	expect(harris != nullptr, "Detective Harris is found");
	if (harris)
	{
		expect(harris->archetype == "regular_major" && harris->seat == "bar_stool_3",
			"archetype and seat read");
		expect(harris->name_en == "Detective Harris"
			&& harris->name_ru == "\u0414\u0435\u0442\u0435\u043a\u0442\u0438\u0432 \u0425\u0430\u0440\u0440\u0438\u0441",
			"bilingual name reads");

		expect(harris->has_schedule && harris->days.size() == 7
			&& harris->phases.size() == 1 && harris->phases[0] == "Shift"
			&& harris->arrival_hour == 18,
			"schedule: every day, Shift, 18:00");

		expect(harris->order == "whiskey_neat" && harris->glass == "rocks"
			&& harris->acceptable_substitutes.size() == 1
			&& harris->acceptable_substitutes[0] == "whiskey_on_ice",
			"taste: whiskey neat on the rocks, one substitute");

		expect(harris->persona.tell == "nervous"
			&& std::fabs(harris->persona.mood_baseline - 0.45) < 1e-9
			&& std::fabs(harris->persona.patience_baseline - 0.6) < 1e-9
			&& harris->persona.will_tell_story,
			"persona: nervous, baselines, has a story");
		expect(harris->faction == "bureau" && harris->arc == "act1_harris",
			"faction and arc read");
	}
	expect(library.find("no_such_customer") == nullptr, "unknown ids read null");

	{
		CustomerLibrary from_directory;
		std::vector<std::string> errors;
		const int loaded = from_directory.load_directory(
			lc_test::repo_root() + "/Data/Customers", &errors);
		expect(loaded == 3 && errors.empty(), "the Customers directory loads clean");
		expect(from_directory.ids().size() == 3, "three regulars on the books");

		const CustomerRecord* kane = from_directory.find("kane");
		expect(kane != nullptr, "Miss Kane is found");
		if (kane)
		{
			expect(kane->archetype == "regular_major" && kane->faction == "civilian"
				&& kane->order == "dry_martini" && kane->glass == "martini",
				"Kane: the martini, the booth, civilian");
			expect(kane->days.size() == 2 && kane->arrival_hour == 20
				&& kane->phases.size() == 1 && kane->phases[0] == "Shift",
				"Kane: Tuesdays and Fridays at twenty");
			expect(kane->persona.tell == "fake" && kane->persona.will_tell_story,
				"Kane: her tell is a performance — psychology_2 only");
		}

		const CustomerRecord* shadow = from_directory.find("shadow");
		expect(shadow != nullptr, "the boss is found");
		if (shadow)
		{
			expect(shadow->archetype == "regular_major" && shadow->faction == "syndicate"
				&& shadow->order == "black_manhattan" && shadow->glass == "rocks",
				"Shadow: the Black Manhattan, syndicate");
			expect(shadow->days.size() == 1 && shadow->days[0] == 4
				&& shadow->phases.size() == 1 && shadow->phases[0] == "Night"
				&& shadow->arrival_hour == 23,
				"Shadow: Thursdays only, Night phase");
			expect(shadow->persona.tell == "hostile" && !shadow->persona.will_tell_story,
				"Shadow: hostile tell, no free stories");
		}
	}

	// --- arrival ------------------------------------------------------------------------
	if (harris)
	{
		expect(lc::arrives_on(*harris, 3, "Shift"), "he comes on day 3 of the Shift");
		expect(!lc::arrives_on(*harris, 3, "Night"), "not in the Night");
		expect(!lc::arrives_on(*harris, 9, "Shift"), "day 9 is outside the week");
	}

	// --- reading tells (GAME-DESIGN 3.2) ------------------------------------------------
	{
		CustomerRecord nervous;
		nervous.persona.tell = "nervous";
		CustomerRecord fake;
		fake.persona.tell = "fake";

		const TellReadiness unskilled;
		expect(!lc::tell_revealed(nervous.persona, unskilled),
			"without skills the tell stays unread");
		expect(!lc::tell_revealed(fake.persona, unskilled),
			"the performance passes untrained");

		TellReadiness psychology_1;
		psychology_1.reveal_tells = true;
		expect(lc::tell_revealed(nervous.persona, psychology_1),
			"psychology_1 reads the honest tell");
		expect(!lc::tell_revealed(fake.persona, psychology_1),
			"a performed tell still passes psychology_1");

		TellReadiness psychology_2;
		psychology_2.reveal_lies = true;
		expect(lc::tell_revealed(fake.persona, psychology_2),
			"psychology_2 sees through the fake");

		// The library record itself is what the sensor reads from.
		if (harris)
		{
			expect(lc::tell_revealed(harris->persona, psychology_1),
				"Harris' nervous tell reads with psychology_1");
			expect(!lc::tell_revealed(harris->persona, unskilled),
				"...and stays hidden without it");
		}
	}

	// --- threat sensor --------------------------------------------------------------------
	{
		const ThreatTuning tuning;
		// Code and Data/Balance/core.json must agree on the tuning.
		expect_near(tuning.hostile_tell_weight,
			lc_test::balance_number("hostile_tell_weight"), 1e-9,
			"hostile_tell_weight synced with core.json");
		expect_near(tuning.low_mood_weight,
			lc_test::balance_number("low_mood_weight"), 1e-9,
			"low_mood_weight synced with core.json");
		expect_near(tuning.low_mood_threshold,
			lc_test::balance_number("low_mood_threshold"), 1e-9,
			"low_mood_threshold synced with core.json");
		expect_near(tuning.band_elevated,
			lc_test::balance_number("band_elevated"), 1e-9,
			"band_elevated synced with core.json");
		expect_near(tuning.band_danger,
			lc_test::balance_number("band_danger"), 1e-9,
			"band_danger synced with core.json");

		expect_near(lc::threat_score({}, tuning), 0.0, 1e-9, "an empty room is calm");
		expect(lc::threat_band(0.0, tuning) == ThreatBand::Calm, "score 0 is Calm");

		const std::vector<PatronRead> nervous_room = { { "nervous", 0.5 } };
		expect_near(lc::threat_score(nervous_room, tuning), 0.0, 1e-9,
			"a nervous patron with OK mood adds nothing");

		const std::vector<PatronRead> hostile_alone = { { "hostile", 0.5 } };
		const double hostile_score = lc::threat_score(hostile_alone, tuning);
		expect_near(hostile_score, tuning.hostile_tell_weight, 1e-9,
			"one hostile patron weighs hostile_tell_weight");
		expect(lc::threat_band(hostile_score, tuning) == ThreatBand::Elevated,
			"that band is Elevated");

		const std::vector<PatronRead> sour_pair = { { "nervous", 0.1 }, { "thirsty", 0.1 } };
		const double sour_score = lc::threat_score(sour_pair, tuning);
		expect(sour_score > 0.0 && lc::threat_band(sour_score, tuning) == ThreatBand::Calm,
			"two low-mood patrons are unsettling but calm");

		const std::vector<PatronRead> tipping = {
			{ "hostile", 0.5 }, { "nervous", 0.1 }, { "thirsty", 0.1 }
		};
		const double tipping_score = lc::threat_score(tipping, tuning);
		expect(lc::threat_band(tipping_score, tuning) == ThreatBand::Danger,
			"hostile plus a sour room reaches Danger");

		const std::vector<PatronRead> riot = {
			{ "hostile", 0.0 }, { "hostile", 0.0 }, { "hostile", 0.0 }
		};
		expect_near(lc::threat_score(riot, tuning), 1.0, 1e-9,
			"the score caps at 1");

		// A high-mood room with no hostile patron stays calm even below the
		// mood threshold test above: mood 1 contributes nothing.
		const std::vector<PatronRead> happy = { { "lonely", 1.0 } };
		expect_near(lc::threat_score(happy, tuning), 0.0, 1e-9, "a happy room scores 0");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing id refused");
	expect(refuses("{\"id\":\"X\"}"), "uppercase id refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5}}"),
		"missing faction refused");
	expect(refuses("{\"id\":\"x\",\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\"}"), "missing name refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"drunk\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\"}"), "unknown tell refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":1.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\"}"), "mood above 1 refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\"}"), "missing ru name refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"mafia\"}"), "unknown faction refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\"}"), "missing taste refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\",\"schedule\":{\"days\":[0],\"phases\":[\"Shift\"]}}"),
		"day 0 refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\",\"schedule\":{\"days\":[1],\"phases\":[\"Supper\"]}}"),
		"unknown phase refused");
	expect(refuses("{\"id\":\"x\",\"name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"taste\":{\"order\":\"o\",\"glass\":\"rocks\"},"
		"\"persona\":{\"tell\":\"nervous\",\"mood_baseline\":0.5,\"patience_baseline\":0.5},"
		"\"faction\":\"civilian\",\"schedule\":{\"days\":[1],\"phases\":[\"Shift\"],"
		"\"arrival_hour\":24}}"), "arrival hour 24 refused");
	expect(refuses("not json"), "garbage refused");

	{
		CustomerLibrary scratch;
		expect(scratch.load(read_repo_file("Data/Customers/harris.json"), &error),
			"the first copy loads");
		expect(!scratch.load(read_repo_file("Data/Customers/harris.json"), &error)
			&& !error.empty() && error.find("duplicate") != std::string::npos,
			"a duplicate customer is refused");
	}
	{
		CustomerLibrary scratch;
		std::vector<std::string> errors;
		expect(scratch.load_directory("/no/such/directory", &errors) == 0 && !errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("customer library");
}
