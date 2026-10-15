// LAST CALL - venue progression core (ROADMAP D1).
//
// The growth ladder as data: Data/Venue/venue.json lists the five tiers
// of GAME-DESIGN section 5.4 (dive -> renovated bar -> VIP floor ->
// second floor -> underground casino), each with the price to reach it,
// the layout id of the room (layout data, not hardcoded rooms), how many
// bodies the crowd fits, the risk multiplier the tier carries and its
// music set - "each tier: new room, new crowd, new risk, new music set",
// and the same 1..5 span feeds the ending texts (NARRATIVE beat 26).
//
// The core answers two questions engine-free: can the venue at tier N
// buy its way to N+1 right now, and at what price (plan_advance) - and,
// once yes, what the purchase itself carries: the tier it lands on and
// the sum to leave in the treasury (buy_advance). The ladder is money
// only (GDD 5.4: upgrades are a money sink) - no story gates; position
// in the document defines the tier.
//
// Plain C++17, no Unreal. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	/** One rung of the growth ladder. */
	struct VenueTier
	{
		int index = 1;             // 1..N, position in the ladder
		std::string id;
		std::string name_en;
		std::string name_ru;
		int upgrade_cost = 0;      // price to reach this tier (0 for tier 1)
		std::string layout;        // layout data id (UE content)
		int capacity = 0;          // bodies the room seats
		double risk = 1.0;         // multiplier on the tier's dangers
		std::string music_set;     // music set id (UE content)
	};

	/** The plan to climb one rung: what it costs and whether it can be paid. */
	struct VenueAdvance
	{
		bool possible = false;
		int to_index = -1;         // target tier (1..N)
		int cost = 0;              // money to pay on success
		std::string blocker;       // "" when possible, else why not
	};

	/** The climb paid for: which tier was bought and for how much. */
	struct VenuePurchase
	{
		bool ok = false;
		int from_index = -1;       // tier the venue stood on
		int to_index = -1;         // tier it owns once the money is paid
		int cost = 0;              // sum to leave in the treasury
		std::string blocker;       // "" when ok, else why it was refused
	};

	/** The tiers of Data/Venue/venue.json. */
	class VenueLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known fields only, bilingual names,
		 * the ladder contiguous from 1 (array position = tier index),
		 * tier 1 free to stand on, every later tier a positive price,
		 * prices and capacities growing with the ladder, risk never
		 * falling as the room grows, capacity >= 1, risk > 0. Duplicate
		 * tier ids are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		int count() const { return static_cast<int>(tiers_.size()); }

		/** The tier at this position (1..count), or nullptr. */
		const VenueTier* tier(int index) const;

		/** The tier by id ("dive", "casino", ...), or nullptr. */
		const VenueTier* find_id(const std::string& tier_id) const;

		/**
		 * Can the venue at `current_index` buy its way to the next tier
		 * with `money` in the treasury? Refuses (possible = false, a
		 * blocker saying why) on an unknown current tier, at the top of
		 * the ladder, or when the treasury is short - the blocker names
		 * the missing sum in whole units. Loans may hold `money` below
		 * zero; the arithmetic still holds.
		 */
		VenueAdvance plan_advance(int current_index, int money) const;

		/**
		 * Buy the climb from `current_index` with `money` in the
		 * treasury: the same refusals as plan_advance (unknown tier,
		 * top of the ladder, a blocker naming the shortfall), and on
		 * success the purchase itself - the tier to stand on and the
		 * sum to deduct; a refusal leaves every field at its default
		 * except the blocker. The ladder stays sequential: one rung
		 * per purchase, never a skip. This core only decides - the
		 * caller moves the wallet and the save's tier (interim:
		 * ULCSaveSystem).
		 */
		VenuePurchase buy_advance(int current_index, int money) const;

	private:
		std::vector<VenueTier> tiers_;
	};

} // namespace lc
