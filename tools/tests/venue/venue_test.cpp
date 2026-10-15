// LAST CALL - venue progression core unit tests.
//
// Runs without Unreal: scripts/test-venue.sh compiles this against the
// venue core, the JSON reader, the endings core (tier-span cross-check)
// and the harness. Drives the real file Data/Venue/venue.json.

#include "Venue/VenueCore.h"

#include "Story/EndingsCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>
#include <string>

namespace {

	using lc_test::expect;

	using lc::EndingLibrary;
	using lc::VenueAdvance;
	using lc::VenueLibrary;

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
		VenueLibrary library;
		std::string error;
		return !library.load(text, &error) && !error.empty();
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

} // namespace

int main()
{
	// --- the real document --------------------------------------------------------
	const std::string doc_text = read_repo_file("Data/Venue/venue.json");
	VenueLibrary library;
	std::string error;
	const bool loaded = library.load(doc_text, &error);
	if (!loaded)
	{
		std::printf("  load error: %s\n", error.c_str());
	}
	expect(loaded, "Data/Venue/venue.json loads");
	expect(library.count() == 5, "the growth ladder has its five rungs");

	// Structure: position = tier, bilingual names, content references,
	// prices and capacities growing, risk never falling.
	for (int index = 1; index <= library.count(); ++index)
	{
		const lc::VenueTier* tier = library.tier(index);
		expect(tier != nullptr, "every tier 1..5 is addressable");
		if (!tier)
		{
			continue;
		}
		expect(tier->index == index, "the position is the tier");
		expect(!tier->name_en.empty() && !tier->name_ru.empty(),
			"the tier has a bilingual name");
		expect(!tier->layout.empty() && !tier->music_set.empty(),
			"the tier names its layout and its music set");
		if (index > 1)
		{
			const lc::VenueTier* previous = library.tier(index - 1);
			expect(tier->upgrade_cost > previous->upgrade_cost,
				"the next rung costs more than the last");
			expect(tier->capacity > previous->capacity,
				"the next room seats more bodies");
			expect(tier->risk >= previous->risk,
				"a bigger room is at least as big a target");
		}
	}
	expect(library.tier(1)->upgrade_cost == 0, "the dive is where the game starts - free");
	expect(library.tier(0) == nullptr && library.tier(6) == nullptr,
		"tier 0 and tier 6 do not exist");
	expect(library.find_id("dive") != nullptr && library.find_id("casino") != nullptr,
		"find() by id");
	expect(library.find_id("no_such_tier") == nullptr, "find() of an unknown id is null");

	// --- the plan to climb ----------------------------------------------------------
	{
		const lc::VenueTier* second = library.tier(2);
		const lc::VenueTier* fifth = library.tier(5);

		VenueAdvance poor = library.plan_advance(1, 0);
		expect(!poor.possible && !poor.blocker.empty(),
			"an empty treasury cannot buy the renovated bar");
		VenueAdvance almost = library.plan_advance(1, second->upgrade_cost - 1);
		expect(!almost.possible && !almost.blocker.empty(),
			"one unit short is still short");
		VenueAdvance exact = library.plan_advance(1, second->upgrade_cost);
		expect(exact.possible && exact.to_index == 2
			&& exact.cost == second->upgrade_cost,
			"exact change buys the renovated bar");
		VenueAdvance rich = library.plan_advance(3, 1000000);
		expect(rich.possible && rich.to_index == 4 && rich.cost == library.tier(4)->upgrade_cost,
			"money buys the second floor from the VIP floor");
		VenueAdvance top = library.plan_advance(4, 1000000);
		expect(top.possible && top.to_index == 5, "money buys the casino");
		VenueAdvance done = library.plan_advance(5, 1000000);
		expect(!done.possible && !done.blocker.empty(),
			"the top of the ladder is the top");
		VenueAdvance debt = library.plan_advance(1, -100);
		expect(!debt.possible && !debt.blocker.empty(),
			"loans below zero still cannot buy an upgrade");
		VenueAdvance unknown_low = library.plan_advance(0, 1000000);
		expect(!unknown_low.possible && !unknown_low.blocker.empty(),
			"tier 0 refuses");
		VenueAdvance unknown_high = library.plan_advance(6, 1000000);
		expect(!unknown_high.possible && !unknown_high.blocker.empty(),
			"tier 6 refuses");
		expect(fifth != nullptr, "the casino exists to end on");
	}

	// --- the purchase of the next rung ------------------------------------------------
	{
		using lc::VenuePurchase;
		const lc::VenueTier* second = library.tier(2);

		VenuePurchase paid = library.buy_advance(1, second->upgrade_cost);
		expect(paid.ok && paid.from_index == 1 && paid.to_index == 2
			&& paid.cost == second->upgrade_cost,
			"exact change buys the renovated bar - the purchase names tier and price");

		VenuePurchase skip = library.buy_advance(1, 1000000);
		expect(skip.ok && skip.to_index == 2,
			"the ladder moves one rung at a time - no skipping to the casino");

		VenuePurchase mid = library.buy_advance(3, 1000000);
		expect(mid.ok && mid.from_index == 3 && mid.to_index == 4
			&& mid.cost == library.tier(4)->upgrade_cost,
			"the VIP floor buys the second floor for exactly the ladder's price");

		VenuePurchase last = library.buy_advance(4, 1000000);
		expect(last.ok && last.to_index == 5,
			"the casino is buyable - the ladder has an end");

		VenuePurchase too_poor = library.buy_advance(1, second->upgrade_cost - 1);
		expect(!too_poor.ok && too_poor.from_index == -1 && too_poor.to_index == -1
			&& too_poor.cost == 0 && !too_poor.blocker.empty(),
			"a treasury one unit short buys nothing and says why");

		VenuePurchase top = library.buy_advance(5, 1000000);
		expect(!top.ok && !top.blocker.empty(),
			"the top of the ladder cannot be bought past");

		VenuePurchase unknown = library.buy_advance(0, 1000000);
		expect(!unknown.ok && !unknown.blocker.empty(),
			"an unknown tier buys nothing");

		VenuePurchase red = library.buy_advance(1, -100);
		expect(!red.ok && !red.blocker.empty(),
			"a wallet below zero cannot buy a rung either");
	}

	// --- cross-check: the ladder and the ending texts span the same tiers -----------
	{
		EndingLibrary endings;
		expect(endings.load(read_repo_file("Data/Endings/endings.json"), &error),
			"Data/Endings/endings.json loads for the tier-span cross-check");
		bool span_agrees = !endings.endings().empty();
		for (const lc::EndingSpec& spec : endings.endings())
		{
			// The venue's top tier has an ending text, and one tier
			// above the ladder has none - the two data files agree.
			if (!spec.text_for(library.count()) || spec.text_for(library.count() + 1))
			{
				span_agrees = false;
				std::printf("  ending '%s' does not span the venue ladder\n", spec.id.c_str());
			}
		}
		expect(span_agrees,
			"the venue ladder and the six endings span the same five tiers");
	}

	// --- strict loading ---------------------------------------------------------------
	expect(refuses(replace_first(doc_text, "\"index\": 3", "\"index\": 9")),
		"a gap in the ladder refused");
	expect(refuses(replace_first(doc_text, "\"index\": 4", "\"index\": 42")),
		"an out-of-order ladder refused");
	expect(refuses(replace_first(doc_text, "\"upgrade_cost\": 0", "\"upgrade_cost\": 100")),
		"the first tier must be free");
	expect(refuses(replace_first(doc_text, "\"upgrade_cost\": 5000", "\"upgrade_cost\": 1000")),
		"prices must grow with the ladder");
	expect(refuses(replace_first(doc_text, "\"upgrade_cost\": 25000", "\"upgrade_cost\": -1")),
		"a negative price refused");
	expect(refuses(replace_first(doc_text, "\"capacity\": 24", "\"capacity\": 10")),
		"capacity must grow with the ladder");
	expect(refuses(replace_first(doc_text, "\"capacity\": 12", "\"capacity\": 0")),
		"a zero capacity refused");
	expect(refuses(replace_first(doc_text, "\"risk\": 1.35", "\"risk\": 0.9")),
		"risk must not fall as the room grows");
	expect(refuses(replace_first(doc_text, "\"risk\": 1.0", "\"risk\": -0.5")),
		"a negative risk refused");
	expect(refuses(replace_first(doc_text, "\"id\": \"vip_floor\"", "\"id\": \"dive\"")),
		"a duplicate tier id refused");
	expect(refuses(replace_first(doc_text, "\"ru\": \"VIP-этаж\"", "\"ru\": \"\"")),
		"a half-translated name refused");
	expect(refuses(replace_first(doc_text, "\"layout\": \"layout_vip\"", "\"layout\": \"Layout Vip!\"")),
		"a malformed layout id refused");
	expect(refuses(replace_first(doc_text,
		",\n\t\t\t\"layout\"", ",\n\t\t\t\"secret\": 1,\n\t\t\t\"layout\"")),
		"an unknown field refused");
	expect(refuses(replace_first(doc_text, "\"tiers\": [", "\"rungs\": [")),
		"a document without `tiers` refused");
	expect(refuses("not json"), "garbage refused");
	expect(refuses("[1,2]"), "an array document refused");

	return lc_test::report("venue progression");
}
