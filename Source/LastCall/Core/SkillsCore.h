// LAST CALL - skill tree core (ARCHITECTURE.md, section 3; GAME-DESIGN, skills).
//
// The engine-independent half of skill progression: loads Data/Skills/
// skills.json with the shared JSON reader, answers what a player can buy
// with a set of owned nodes and a money amount, aggregates the owned
// nodes' effects (multipliers compose, unlocks union, reveals flip), and
// applies them to the pour/serve simulation the way GAME-DESIGN 5.1
// names them - longer combo windows, higher tips (A6).
// Engine-independent C++17: the UE side owns the UI and the wallet; the
// rules live here, tested against the real data file.
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

#include "Bar/PourSolverCore.h"
#include "Customers/OrderCore.h"

namespace lc {

	struct SkillEffect
	{
		std::string type; // service_speed | tip_multiplier | pour_shake |
		                  // combo_window | reveal_tells | reveal_lies |
		                  // unlock_ingredients
		double mult = 1.0;                        // for the *_mult types
		std::vector<std::string> unlock_ids;      // for unlock_ingredients
	};

	struct SkillNode
	{
		std::string id;
		std::string branch;
		double cost = 0.0;
		std::vector<std::string> requirements;  // node ids that must be owned first
		bool has_effect = false;
		SkillEffect effect;
	};

	/** What a set of owned nodes does to the simulation. */
	struct SkillSummary
	{
		double service_speed_mult = 1.0; // < 1: faster service (flair)
		double tip_multiplier = 1.0;     // > 1: better tips (flair)
		double pour_shake_mult = 1.0;    // < 1: less shake noise (steady hands)
		double combo_window_mult = 1.0;  // > 1: wider combo windows (steady hands)
		bool reveal_tells = false;       // psychology
		bool reveal_lies = false;
		std::vector<std::string> unlocked_ingredients; // sorted, deduplicated
	};

	class SkillTree
	{
	public:
		/**
		 * Parse a skills document. Strict: unique node ids across branches,
		 * every `requires` entry names an existing node, costs are not
		 * negative, effects carry a known type with sane parameters.
		 * `error` explains the refusal.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		const SkillNode* find(const std::string& node_id) const;
		std::vector<std::string> ids() const; // load order

		/** Nodes buyable right now: requirements owned, not owned yet. */
		std::vector<std::string> available(const std::vector<std::string>& owned) const;

		/** Can `node_id` be bought by this player? Fills `cost_out` when given. */
		bool can_buy(const std::string& node_id, const std::vector<std::string>& owned,
			double money, double* cost_out = nullptr) const;

		/** Buy: deducts the cost and appends the node. False when not buyable. */
		bool buy(const std::string& node_id, std::vector<std::string>& owned,
			double& money) const;

		/** Compose the effects of every owned node. */
		SkillSummary summarize(const std::vector<std::string>& owned) const;

	private:
		std::vector<SkillNode> nodes_;
	};

	// --- A6: the summary as the pour/serve simulation feels it -----------------

	/**
	 * Steady hands' longer combo windows (GAME-DESIGN 5.1): every flow
	 * window grows away from its own center by combo_window_mult (>1
	 * wider, <1 tighter, clamped to the solver's 0..1), so a player who
	 * bought the nodes hits the speed windows the purchase promised.
	 * A mult of 1.0 returns the spec unchanged; input is never touched.
	 */
	RecipeSpec apply_combo_windows(const SkillSummary& skills, const RecipeSpec& spec);

	/**
	 * Flair's higher tips (GAME-DESIGN 5.1): the skill multiplier lands
	 * on whatever the recipe and the substitute already did to the
	 * payout - a wrong serve still pays nothing (0 x anything = 0).
	 * Input is never touched.
	 */
	ServeOutcome apply_skill_tips(const SkillSummary& skills, const ServeOutcome& outcome);

} // namespace lc
