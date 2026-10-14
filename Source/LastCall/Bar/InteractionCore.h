// LAST CALL - destructibility core (ROADMAP A8).
//
// The same interaction table serves the shift and the night: what in the
// bar can be grabbed, swung, hidden behind, thrown or sprayed
// (GAME-DESIGN 5.3 and the night rules: bottles as weapons, flipped
// tables as cover, the siphon as an extinguisher). Each row of Data/Interactions/
// interactions.json says which roles an object plays, how hard it hits
// and how many uses it survives; the rules here answer the questions the
// night layer asks: is this usable now, is it a weapon, what is the best
// thing I am holding, and did that swing break it.
//
// Engine-independent C++17; the UE side does the physics and animation.
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct InteractionSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::vector<std::string> kinds; // weapon | throwable | cover | extinguisher
		double damage = 0.0;
		int sturdiness = 1;              // uses before it breaks
		std::vector<std::string> phases; // Shift | Night - when it is available
	};

	/** Objects the room lets you manhandle, from Data/Interactions. */
	class InteractionLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known kinds and phases, damage
		 * 0 or more, sturdiness 1 or more, bilingual names; duplicate
		 * ids are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		const InteractionSpec* find(const std::string& id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<InteractionSpec>& interactions() const { return interactions_; }

	private:
		std::vector<InteractionSpec> interactions_;
	};

	/** One object in the player's hands, with uses left. */
	struct HeldObject
	{
		std::string id;
		int sturdiness_left = 0;
	};

	// --- rules the night layer asks ------------------------------------------------------

	/** Is this object listed for this phase ("Shift" or "Night")? */
	bool usable_in(const InteractionSpec& spec, const std::string& phase);

	/** Swings: has the weapon or throwable role and something to hit with. */
	bool is_weapon(const InteractionSpec& spec);

	bool is_cover(const InteractionSpec& spec);

	bool is_extinguisher(const InteractionSpec& spec);

	/** Pick an object up at full sturdiness. */
	HeldObject pick_up(const InteractionSpec& spec);

	/**
	 * One use of a held object. Returns false when this swing broke it
	 * (sturdiness spent) - the caller drops it either way.
	 */
	bool use_once(HeldObject& held);

	/**
	 * The hardest weapon among the ids actually held and usable in this
	 * phase, or nullptr when the player is holding nothing that swings.
	 */
	const InteractionSpec* best_weapon(const InteractionLibrary& library,
		const std::vector<std::string>& held_ids, const std::string& phase);

} // namespace lc
