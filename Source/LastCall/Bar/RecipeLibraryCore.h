// LAST CALL - recipe library core (milestone F1/A2 groundwork).
//
// Loads the real recipe files from Data/Recipes/ with the shared JSON
// reader and hands them to the pour solver (RecipeSpec) and the order
// layer (effects). Engine-independent C++17: the same library is used by
// the UE side through ULCRecipeLibrary and by the tests without Unreal.
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

#include "Bar/PourSolverCore.h"
#include "Customers/OrderCore.h"

namespace lc {

	struct RecipeRecord
	{
		std::string id;
		std::string glass;
		std::vector<MixStep> steps;             // solver windows, in order
		std::vector<std::string> layers;        // parallel to steps: base/modifier/accent
		double continuity_break_s = 4.0;
		RecipeEffects effects;
		bool act1_hook = false;                 // story: first recipe off the uncle's wall

		// The wall-of-recipes twist (NARRATIVE.md): this recipe hides one
		// fragment of the message the uncle chalked into the recipes.
		bool has_secret = false;
		int secret_fragment = 0;                // 1-based position of the fragment
		int secret_of = 0;                      // total fragments on the wall
		std::string secret_text_key;            // localization key of the fragment
		std::string secret_reveal_flag;         // world flag that unlocks reading it ("" = always)

		// Bilingual display name: the data schema requires both languages and
		// the loader enforces it whenever the block is present (documents
		// without one stay legal for minimal packs and tests).
		std::string name_en;
		std::string name_ru;
	};

	/**
	 * Parse one recipe document. Strict: id and glass, a non-empty step
	 * list with sane flow windows (0 <= min <= max <= 1). `error` gets
	 * "<byte offset or field>: <reason>" when refused.
	 */
	bool load_recipe(const std::string& json_text, RecipeRecord& out, std::string* error = nullptr);

	// The solver's view of a recipe (combo evaluation input).
	RecipeSpec to_solver_spec(const RecipeRecord& recipe);

	class RecipeLibrary
	{
	public:
		/**
		 * Load every *.json file in a directory (Data/Recipes). Returns how
		 * many recipes were loaded; every failure - unreadable file, bad
		 * JSON, invalid recipe, duplicate id - is appended to `errors`.
		 */
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		/** Parse a single recipe text into the library (used by tests and DLC-style packs). */
		bool add(const std::string& json_text, std::vector<std::string>* errors = nullptr);

		const RecipeRecord* find(const std::string& id) const;

		/** Every id, in load order. */
		std::vector<std::string> ids() const;

		const std::vector<RecipeRecord>& recipes() const { return records_; }

	private:
		std::vector<RecipeRecord> records_;
	};

	/** One secret fragment as the story layer sees it. */
	struct SecretFragment
	{
		std::string recipe_id; // which recipe on the wall hides it
		int index = 0;         // 1-based, unique across the library
		int of = 0;            // total fragments the wall promises
		std::string text_key;
		std::string reveal_flag;
	};

	/**
	 * Collect every recipe's `secret` block, sorted by fragment index.
	 * Refused (false + error) when two recipes claim the same index or
	 * disagree on the total - the wall must add up to one message.
	 */
	bool collect_secret_fragments(const RecipeLibrary& library,
		std::vector<SecretFragment>& out, std::string* error = nullptr);

} // namespace lc
