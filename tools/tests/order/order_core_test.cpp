// LAST CALL - F3 cores unit tests: order layer + consequence log.
//
// Runs without Unreal: scripts/test-order.sh compiles this against
// Source/LastCall/Customers/OrderCore.cpp and
// Source/LastCall/Core/ConsequenceLogCore.cpp with g++.

#include "Core/ConsequenceLogCore.h"
#include "Customers/OrderCore.h"

#include "test_support.h"

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::ConsequenceLog;
	using lc::OrderSpec;
	using lc::OrderTuning;
	using lc::RecipeEffects;

	// What Data/Customers/harris.json asks for, against signature_noir.
	OrderSpec harris_order()
	{
		OrderSpec order;
		order.customer_id = "harris";
		order.recipe_id = "whiskey_neat";
		order.glass = "rocks";
		order.acceptable_substitutes = { "whiskey_on_ice" };
		return order;
	}

} // namespace

int main()
{
	const OrderTuning tuning;
	const std::vector<std::string> known_recipes = { "signature_noir", "whiskey_neat", "whiskey_on_ice" };

	// --- order validity ----------------------------------------------------------
	expect(lc::order_is_valid(harris_order(), known_recipes),
		"an order with a known recipe is valid");
	{
		OrderSpec order = harris_order();
		order.recipe_id = "martini_my_way";
		expect(!lc::order_is_valid(order, known_recipes),
			"an order for a missing recipe is refused");
	}
	{
		OrderSpec order = harris_order();
		order.glass.clear();
		expect(!lc::order_is_valid(order, known_recipes),
			"an order without a glass is incomplete");
	}

	// --- serve outcomes -----------------------------------------------------------
	{
		const lc::ServeOutcome outcome = lc::evaluate_serve(
			harris_order(), "whiskey_neat", "rocks", { 0.2, 1.4 }, tuning);
		expect(outcome.correct && !outcome.substitute && !outcome.wrong,
			"the exact order in the right glass is correct");
		expect_near(outcome.tip_multiplier, 1.4, 1e-9, "correct order pays the recipe tip");
		expect_near(outcome.mood_bonus, 0.2, 1e-9, "correct order pays the recipe mood bonus");
	}
	{
		const lc::ServeOutcome outcome = lc::evaluate_serve(
			harris_order(), "whiskey_on_ice", "rocks", { 0.2, 1.4 }, tuning);
		expect(!outcome.correct && outcome.substitute && !outcome.wrong,
			"an acceptable substitute counts, but differently");
		expect_near(outcome.tip_multiplier, 1.4 * 0.7, 1e-9,
			"substitute tip is penalized by substitute_tip_penalty");
		expect_near(outcome.mood_bonus, 0.2, 1e-9, "substitute keeps the mood bonus");
	}
	{
		const lc::ServeOutcome outcome = lc::evaluate_serve(
			harris_order(), "vodka_neat", "rocks", { 0.2, 1.4 }, tuning);
		expect(outcome.wrong && !outcome.correct && !outcome.substitute,
			"a different drink is wrong");
		expect_near(outcome.tip_multiplier, 0.0, 1e-9, "wrong drink pays no tip");
		expect_near(outcome.mood_bonus, 0.0, 1e-9, "wrong drink pays no mood");
	}
	{
		const lc::ServeOutcome outcome = lc::evaluate_serve(
			harris_order(), "whiskey_neat", "highball", { 0.2, 1.4 }, tuning);
		expect(outcome.wrong && outcome.wrong_glass,
			"right drink, wrong glass is still wrong");
		expect_near(outcome.tip_multiplier, 0.0, 1e-9, "wrong glass pays no tip");
	}

	// --- arrival schedule -----------------------------------------------------------
	{
		lc::CustomerSchedule schedule;
		schedule.days = { 1, 2, 3, 4, 5, 6, 7 };
		schedule.phase = "Shift";
		expect(lc::arrives_on(schedule, 3, "Shift"), "comes on a listed day in the right phase");
		expect(!lc::arrives_on(schedule, 3, "Night"), "does not come in another phase");
		schedule.days = { 2, 4 };
		expect(!lc::arrives_on(schedule, 3, "Shift"), "does not come on an unlisted day");
		schedule.days.clear();
		expect(!lc::arrives_on(schedule, 2, "Shift"), "an empty schedule never comes");
	}

	// --- consequence log --------------------------------------------------------------
	{
		ConsequenceLog log;
		expect(!log.has("eavesdrop_interrupted"), "an empty log has nothing");
		expect(log.record("eavesdrop_interrupted", 2, "Night"), "first record is new");
		expect(log.has("eavesdrop_interrupted"), "recorded flag is visible");
		expect(!log.record("eavesdrop_interrupted", 2, "Night"),
			"the same flag on the same day collapses");
		expect(log.count("eavesdrop_interrupted") == 1, "one distinct day so far");
		expect(log.record("eavesdrop_interrupted", 5, "Night"),
			"the same flag on a new day is a new consequence");
		expect(log.count("eavesdrop_interrupted") == 2, "two distinct days now");
		expect(log.record("harris_told_story", 5, "Shift"), "flags are independent");
		expect(log.entries().size() == 3, "three entries in record order");
		expect(log.entries()[0].day == 2 && log.entries()[2].flag == "harris_told_story",
			"entries keep their order and payload");
		expect(!log.record("", 3, "Shift"), "a nameless consequence is refused");
		expect(!log.record("bad_day", 0, "Shift"), "a consequence needs a real day");
	}

	// --- balance sync (Data/Balance/core.json "order") ------------------------------
	expect_near(lc_test::balance_number("substitute_tip_penalty"),
		tuning.substitute_tip_penalty, 1e-9, "substitute penalty in sync with core.json");

	return lc_test::report("order + consequences");
}
