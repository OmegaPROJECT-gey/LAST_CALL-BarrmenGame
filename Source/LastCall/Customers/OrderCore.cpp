// LAST CALL - order core implementation. See header for the contract.

#include "Customers/OrderCore.h"

#include <algorithm>

namespace lc {

	bool order_is_valid(const OrderSpec& order, const std::vector<std::string>& known_recipe_ids)
	{
		if (order.recipe_id.empty() || order.glass.empty())
		{
			return false;
		}
		return std::find(known_recipe_ids.begin(), known_recipe_ids.end(), order.recipe_id)
			!= known_recipe_ids.end();
	}

	ServeOutcome evaluate_serve(const OrderSpec& order, const std::string& poured_recipe_id,
		const std::string& poured_glass, const RecipeEffects& poured_effects,
		const OrderTuning& tuning)
	{
		ServeOutcome outcome;

		const bool right_recipe = poured_recipe_id == order.recipe_id;
		const bool acceptable = std::find(order.acceptable_substitutes.begin(),
			order.acceptable_substitutes.end(), poured_recipe_id) != order.acceptable_substitutes.end();

		if (!right_recipe && !acceptable)
		{
			outcome.wrong = true;
			return outcome;
		}

		if (!poured_glass.empty() && poured_glass != order.glass)
		{
			// The right drink in the wrong glass is not what was ordered -
			// the regulars notice (GAME-DESIGN 3.1).
			outcome.wrong = true;
			outcome.wrong_glass = true;
			return outcome;
		}

		outcome.mood_bonus = poured_effects.mood_bonus;
		if (right_recipe)
		{
			outcome.correct = true;
			outcome.tip_multiplier = poured_effects.tip_multiplier;
		}
		else
		{
			outcome.substitute = true;
			outcome.tip_multiplier = poured_effects.tip_multiplier * tuning.substitute_tip_penalty;
		}
		return outcome;
	}

	bool arrives_on(const CustomerSchedule& schedule, int day, const std::string& phase)
	{
		if (schedule.days.empty() || phase.empty())
		{
			return false;
		}
		const bool day_matches = std::find(schedule.days.begin(), schedule.days.end(), day)
			!= schedule.days.end();
		return day_matches && schedule.phase == phase;
	}

} // namespace lc
