// LAST CALL - order core (milestone F3: "one customer, one order").
//
// What was ordered, what arrived, and whether that counts. Engine-
// independent C++17 like the other cores; the UE side (ULCCustomerComponent)
// feeds it recipe and glass ids straight from the data files in Data/.
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	// Tuning mirrors the "order" section of Data/Balance/core.json.
	struct OrderTuning
	{
		double substitute_tip_penalty = 0.7; // tip multiplier for an acceptable substitute
	};

	// One drink as the customer wants it (Data/Customers/*.json "taste").
	struct OrderSpec
	{
		std::string customer_id;
		std::string recipe_id;  // recipe they asked for
		std::string glass;      // glass it must arrive in
		std::vector<std::string> acceptable_substitutes;
		bool story_order = false; // carries a story beat (NARRATIVE.md)
	};

	// The two recipe effects the order outcome may still pay out
	// (Data/Recipes/*.json "effects"); the combo quality itself is
	// evaluated by the pour solver, this is the order layer on top.
	struct RecipeEffects
	{
		double mood_bonus = 0.0;
		double tip_multiplier = 1.0;
	};

	struct ServeOutcome
	{
		bool correct = false;    // exactly the order, right glass
		bool substitute = false; // acceptable substitute, reduced tip
		bool wrong = false;      // something else entirely
		bool wrong_glass = false; // right drink, wrong glass - still wrong
		double tip_multiplier = 0.0;
		double mood_bonus = 0.0;
	};

	// The order makes sense only if its recipe exists in the library.
	bool order_is_valid(const OrderSpec& order, const std::vector<std::string>& known_recipe_ids);

	// What arriving in front of the customer counts as. Wrong drinks pay
	// nothing: the pour solver already punished the pour, the customer
	// layer only decides whether this is what they ordered.
	ServeOutcome evaluate_serve(const OrderSpec& order, const std::string& poured_recipe_id,
		const std::string& poured_glass, const RecipeEffects& poured_effects,
		const OrderTuning& tuning);

	// --- arrival (Data/Customers "schedule") ---------------------------------

	struct CustomerSchedule
	{
		std::vector<int> days;   // 1-based days of the week they come
		std::string phase;       // "Morning" | "Shift" | "Night"
		int arrival_hour = 18;
	};

	// Do they show up on this day of this phase? The hour is a hint for the
	// spawning system, not a gate (the phase clock has no hours yet).
	bool arrives_on(const CustomerSchedule& schedule, int day, const std::string& phase);

} // namespace lc
