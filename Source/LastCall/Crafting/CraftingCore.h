// LAST CALL - crafting core (ROADMAP A7: infusions and syrups).
//
// Morning prep: turn cheaper shelf liquids into new ones. Each row of
// Data/Crafting/crafting.json says what goes in, how long it should sit
// and how strong it should end - the player picks their own time and
// strength, and the distance from the row decides the outcome: a clean
// infusion, a sloppy-but-usable one, or swill you dump or sell cheap
// (GAME-DESIGN 4: failures are usable or dumpable, never a dead end).
// Crafted outputs are ordinary liquids (Data/Liquids) so recipes can
// use them as ingredients.
//
// Engine-independent C++17; the UE side runs the morning screen and the
// timers. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct CraftInput
	{
		std::string item; // liquid id (Data/Liquids) or another craft's output
		double amount = 1.0;
	};

	struct CraftSpec
	{
		std::string id;
		std::string kind; // "infusion" | "syrup"
		std::string name_en;
		std::string name_ru;
		std::vector<CraftInput> inputs;
		double time_minutes = 0.0; // target steep/cook time
		double strength = 0.5;     // target strength, 0..1
		std::string output_item;   // liquid id that lands on the shelf
		int output_amount = 1;
		std::string failure_item;  // what a bad attempt still yields (dumpable)
	};

	/** Infusion and syrup rows from Data/Crafting/crafting.json. */
	class CraftLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known kinds, positive inputs and
		 * time, strength within 0..1, bilingual names; duplicate ids
		 * are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		const CraftSpec* find(const std::string& id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<CraftSpec>& specs() const { return specs_; }

	private:
		std::vector<CraftSpec> specs_;
	};

	// Tuning mirrors the "crafting" section of Data/Balance/core.json.
	struct CraftTuning
	{
		double perfect_time_minutes = 2.0; // |dt| within: clean
		double perfect_strength = 0.15;    // |ds| within: clean
		double usable_time_minutes = 6.0;  // |dt| within: sloppy but usable
		double usable_strength = 0.35;     // |ds| within: sloppy but usable
		double sloppy_quality = 0.5;       // quality of a sloppy result
	};

	enum class CraftOutcome
	{
		Perfect,
		Usable,
		Failed
	};

	struct CraftResult
	{
		CraftOutcome outcome = CraftOutcome::Failed;
		std::string item; // output on success, failure_item on failure
		int amount = 0;
		double quality = 0.0; // 1.0 clean, sloppy_quality usable, 0 failed
	};

	/**
	 * One attempt: `minutes` steeped and `strength` aimed, against the
	 * spec's targets. Both must land within the perfect tolerances for a
	 * clean result; either drifting into the usable band still gives the
	 * right liquid, just weak or overdone; beyond that you get swill.
	 */
	CraftResult attempt_craft(const CraftSpec& spec, double minutes, double strength,
		const CraftTuning& tuning);

} // namespace lc
