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

} // namespace lc
