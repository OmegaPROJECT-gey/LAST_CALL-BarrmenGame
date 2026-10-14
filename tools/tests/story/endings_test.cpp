// LAST CALL - endings core unit tests.
//
// Runs without Unreal: scripts/test-endings.sh compiles this against the
// endings core, the JSON reader, the social core (faction cross-check)
// and the harness. Drives the real file Data/Endings/endings.json.

#include "Story/EndingsCore.h"

#include "Core/JsonCore.h"
#include "Social/SocialCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

	using lc_test::expect;

	using lc::EndingChoice;
	using lc::EndingLibrary;
	using lc::JsonValue;
	using lc::json_parse;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	/** True when load() refuses this document and says why. */
	bool refuses(const std::string& text)
	{
		EndingLibrary library;
		std::string error;
		return !library.load(text, &error) && !error.empty();
	}

	/** True when load() accepts the document. */
	bool loads(const std::string& text)
	{
		EndingLibrary library;
		return library.load(text, nullptr);
	}

	std::string replace_first(std::string text, const std::string& from,
		const std::string& to)
	{
		const std::size_t at = text.find(from);
		if (at != std::string::npos)
		{
			text.replace(at, from.size(), to);
		}
		return text;
	}

	// A minimal valid document: one information item, one real rule and
	// the `always` backstop, against one ending with five tiers.
	const char* kInfo = "\"items\":[\"fragment_01\"]";
	const char* kGoodRule = "{\"type\":\"info_at_least\",\"min\":1,\"ending\":\"alpha\"}";
	const char* kAlwaysRule = "{\"type\":\"always\",\"ending\":\"alpha\"}";
	const char* kNotAlwaysRule = "{\"type\":\"max_standing_at_most\",\"max\":0,\"ending\":\"alpha\"}";
	const char* kDominantRule =
		"{\"type\":\"dominant_any\",\"factions\":[\"syndicate\",\"ring\"],\"min\":1,"
		"\"min_tier\":2,\"max_tier\":3,\"ending\":\"alpha\"}";
	const char* kAlphaEnding =
		"{\"id\":\"alpha\",\"name\":{\"en\":\"A\",\"ru\":\"R\"},\"tiers\":["
		"{\"tier\":1,\"night_key\":\"ending.alpha.t1.night\",\"morning_key\":\"ending.alpha.t1.morning\"},"
		"{\"tier\":2,\"night_key\":\"ending.alpha.t2.night\",\"morning_key\":\"ending.alpha.t2.morning\"},"
		"{\"tier\":3,\"night_key\":\"ending.alpha.t3.night\",\"morning_key\":\"ending.alpha.t3.morning\"},"
		"{\"tier\":4,\"night_key\":\"ending.alpha.t4.night\",\"morning_key\":\"ending.alpha.t4.morning\"},"
		"{\"tier\":5,\"night_key\":\"ending.alpha.t5.night\",\"morning_key\":\"ending.alpha.t5.morning\"}]}";

	std::string make_doc(const std::string& selection, const std::string& endings = kAlphaEnding,
		const std::string& info = kInfo)
	{
		return std::string("{\"information\":{") + info + "},\"selection\":[" + selection
			+ "],\"endings\":[" + endings + "]}";
	}

} // namespace

int main()
{
	// --- the real document ------------------------------------------------------
	const std::string doc_text = read_repo_file("Data/Endings/endings.json");
	EndingLibrary library;
	std::string error;
	const bool loaded = library.load(doc_text, &error);
	if (!loaded)
	{
		std::printf("  load error: %s\n", error.c_str());
	}
	expect(loaded, "Data/Endings/endings.json loads");
	expect(library.endings().size() == 6, "the six endings of NARRATIVE beat 26 are there");

	// Structure: bilingual names, five tiers each, keys in the ending's namespace.
	for (const lc::EndingSpec& spec : library.endings())
	{
		expect(!spec.name_en.empty() && !spec.name_ru.empty(),
			"an ending carries a bilingual name");
		expect(spec.tiers.size() == 5, "an ending has five tier texts");
		const std::string prefix = "ending." + spec.id + ".";
		for (int tier = 1; tier <= 5; ++tier)
		{
			const lc::EndingTier* text = spec.text_for(tier);
			expect(text != nullptr, "every tier 1..5 has text");
			if (text)
			{
				expect(text->night_key.rfind(prefix, 0) == 0
					&& text->morning_key.rfind(prefix, 0) == 0,
					"text keys live in the ending's namespace");
			}
		}
		expect(spec.text_for(0) == nullptr && spec.text_for(6) == nullptr,
			"tier 0 and tier 6 have no text");
	}
	expect(library.find("archive") != nullptr && library.find("keeper") != nullptr,
		"find() by id");
	expect(library.find("no_such_ending") == nullptr, "find() of an unknown id is null");

	// --- information axis ---------------------------------------------------------
	expect(library.count_information(std::vector<std::string>{
		"fragment_01", "fragment_02", "whiskey_neat" }) == 2,
		"held fragments count, other items do not");
	expect(library.count_information(std::vector<std::string>{}) == 0,
		"empty hands hold no information");

	// --- selection on the real data ----------------------------------------------
	struct Case
	{
		const char* what;
		std::vector<std::pair<std::string, double>> standing;
		int information;
		int tier;
		const char* want;
	};

	const Case cases[] = {
		{ "nobody stands with you and the wall is unread",
			{}, 0, 3, "closed" },
		{ "all five factions at neutral reads as closed",
			{ { "civilian", 0.0 }, { "syndicate", 0.0 }, { "ring", 0.0 },
			  { "bureau", 0.0 }, { "station", 0.0 } }, 0, 1, "closed" },
		{ "nine fragments open the archive whatever the city thinks",
			{ { "syndicate", 3.0 } }, 9, 1, "archive" },
		{ "the full wall beats even perfect standing",
			{ { "ring", 2.0 }, { "syndicate", 2.0 } }, 13, 5, "archive" },
		{ "eight fragments are not the line",
			{ { "syndicate", 1.5 } }, 8, 1, "syndicate" },
		{ "nine fragments are the line",
			{ { "syndicate", 1.5 } }, 9, 1, "archive" },
		{ "the Syndicate uniquely dominant at a real standing",
			{ { "syndicate", 1.0 }, { "ring", 0.4 } }, 0, 2, "syndicate" },
		{ "a dive is bought at half a point",
			{ { "syndicate", 0.5 }, { "civilian", 0.2 } }, 0, 1, "syndicate" },
		{ "the same standing does not buy tier 2",
			{ { "syndicate", 0.5 }, { "civilian", 0.2 } }, 0, 2, "keeper" },
		{ "the Ring dominant",
			{ { "ring", 1.2 }, { "civilian", 0.3 } }, 0, 3, "ring" },
		{ "the Ring owns the dive at half a point",
			{ { "ring", 0.6 } }, 0, 1, "ring" },
		{ "the Station dominant at half on a dive",
			{ { "station", 0.7 } }, 0, 1, "precinct" },
		{ "the Bureau and the Station together are the law",
			{ { "bureau", 1.0 }, { "station", 1.0 } }, 0, 5, "precinct" },
		{ "the law tied with the Syndicate is nobody's bar",
			{ { "syndicate", 1.0 }, { "bureau", 1.0 } }, 0, 2, "keeper" },
		{ "two gangs tied keep the owner in place",
			{ { "syndicate", 2.0 }, { "ring", 2.0 } }, 0, 4, "keeper" },
		{ "the room dominant keeps the bar at any tier",
			{ { "civilian", 1.5 }, { "syndicate", 1.0 } }, 0, 5, "keeper" },
		{ "nobody above neutral closes the door",
			{ { "syndicate", -0.5 }, { "civilian", -1.0 } }, 0, 2, "closed" },
		{ "an absent faction reads as neutral and never tops the list",
			{ { "syndicate", 0.8 } }, 0, 1, "syndicate" },
		{ "partial standing with nobody positive closes the door",
			{ { "ring", -0.3 } }, 0, 5, "closed" },
	};

	for (const Case& want : cases)
	{
		EndingChoice choice = library.select(want.standing, want.information, want.tier, &error);
		if (choice.id != want.want)
		{
			std::printf("  case '%s': got '%s' (error: %s)\n",
				want.what, choice.id.c_str(), error.c_str());
		}
		expect(choice.id == want.want, want.what);
		expect(choice.tier == want.tier && !choice.night_key.empty()
			&& !choice.morning_key.empty() && !choice.name_en.empty(),
			"the choice carries its tier texts and bilingual name");
	}

	{
		EndingChoice refused = library.select({}, 0, 6, &error);
		expect(refused.id.empty() && !error.empty(),
			"a venue tier outside 1..5 refuses");
		EndingLibrary empty;
		EndingChoice before = empty.select({}, 0, 1, &error);
		expect(before.id.empty() && !error.empty(),
			"select refuses before load");
	}

	// --- cross-check: every rule faction exists in Data/Factions -------------------
	{
		JsonValue doc;
		expect(json_parse(doc_text, doc, nullptr), "the real document parses");
		lc::FactionLibrary factions;
		expect(factions.load(read_repo_file("Data/Factions/factions.json"), &error),
			"Data/Factions/factions.json loads for the cross-check");
		int checked = 0;
		int missing = 0;
		if (const JsonValue* selection = doc.find("selection"))
		{
			for (const JsonValue& rule : selection->array)
			{
				if (const JsonValue* list = rule.find("factions"))
				{
					for (const JsonValue& faction : list->array)
					{
						++checked;
						if (!factions.find(faction.string))
						{
							++missing;
							std::printf("  rule names an unknown faction '%s'\n",
								faction.string.c_str());
						}
					}
				}
			}
		}
		expect(checked > 0 && missing == 0,
			"every faction named by a selection rule exists in the faction library");
	}

	// --- localization parity: all 60 keys exist in both languages ------------------
	{
		int missing = 0;
		for (const std::string side : { "Data/Localization/en.json", "Data/Localization/ru.json" })
		{
			JsonValue table;
			if (!json_parse(read_repo_file(side), table, nullptr))
			{
				++missing;
				std::printf("  %s does not parse\n", side.c_str());
				continue;
			}
			for (const lc::EndingSpec& spec : library.endings())
			{
				for (const lc::EndingTier& text : spec.tiers)
				{
					for (const std::string& key : { text.night_key, text.morning_key })
					{
						const JsonValue* value = table.find(key);
						if (!value || value->kind != JsonValue::Kind::String
							|| value->string.empty())
						{
							++missing;
							std::printf("  %s misses %s\n", side.c_str(), key.c_str());
						}
					}
				}
			}
		}
		expect(missing == 0,
			"every ending text key exists, non-empty, in en.json and ru.json");
	}

	// --- strict loading --------------------------------------------------------------
	expect(refuses(make_doc(kGoodRule, kAlphaEnding, "\"items\":[]")),
		"an empty information list refused");
	expect(refuses(make_doc(
		"{\"type\":\"info_someday\",\"min\":1,\"ending\":\"alpha\"}," + std::string(kAlwaysRule))),
		"an unknown rule type refused");
	expect(refuses(make_doc(
		std::string(kGoodRule) + ",{\"type\":\"always\",\"ending\":\"beta\"}")),
		"a rule naming an unknown ending refused");
	expect(refuses(make_doc(std::string(kAlwaysRule) + "," + kNotAlwaysRule)),
		"a non-`always` last rule refused");
	expect(refuses(make_doc(
		std::string(kGoodRule) + ",{\"type\":\"always\",\"ending\":\"alpha\",\"note\":\"x\"}")),
		"an unknown field on a rule refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		"\"items\":[\"fragment_01\",\"fragment_01\"]")),
		"a duplicate information item refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		std::string(kAlphaEnding) + "," + kAlphaEnding)),
		"a duplicate ending id refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		replace_first(kAlphaEnding, "\"tier\":5", "\"tier\":6"))),
		"a tier outside 1..5 refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		replace_first(kAlphaEnding, "\"tier\":2", "\"tier\":1"))),
		"a duplicated tier refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		replace_first(kAlphaEnding, "ending.alpha.t2.night", "ending.alpha.t1.night"))),
		"a duplicated text key refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		replace_first(kAlphaEnding, "\"ru\":\"R\"", "\"ru\":\"\""))),
		"an empty Russian name refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule,
		replace_first(kAlphaEnding, "ending.alpha.t1.night", "Night"))),
		"a malformed localization key refused");
	expect(refuses("{\"selection\":[" + std::string(kGoodRule) + "," + kAlwaysRule
		+ "],\"endings\":[" + std::string(kAlphaEnding) + "]}"),
		"a document without `information` refused");
	expect(refuses(make_doc(std::string(kGoodRule) + "," + kAlwaysRule, kAlphaEnding,
		"\"items\":[\"fragment_01\"],\"note\":\"x\"")),
		"an unknown field under `information` refused");
	expect(loads(make_doc(std::string(kDominantRule) + "," + kAlwaysRule)),
		"a well-formed dominant rule loads");
	expect(refuses(make_doc(
		"{\"type\":\"dominant_any\",\"factions\":[\"ring\",\"ring\"],\"min\":1,"
		"\"ending\":\"alpha\"}," + std::string(kAlwaysRule))),
		"a duplicated faction in a dominant rule refused");
	expect(refuses(make_doc(
		"{\"type\":\"dominant_any\",\"factions\":[\"ring\"],\"min\":1,"
		"\"min_tier\":4,\"max_tier\":2,\"ending\":\"alpha\"}," + std::string(kAlwaysRule))),
		"an inverted tier window refused");
	expect(refuses(make_doc(
		"{\"type\":\"dominant_any\",\"factions\":[\"ring\"],\"min\":1,"
		"\"min_tier\":\"x\",\"ending\":\"alpha\"}," + std::string(kAlwaysRule))),
		"a non-integer min_tier refused");
	expect(refuses(make_doc(std::string(kGoodRule))),
		"a document without the always backstop refused");
	expect(refuses("not json"), "garbage refused");
	expect(refuses("[1,2]"), "an array document refused");

	return lc_test::report("endings");
}
