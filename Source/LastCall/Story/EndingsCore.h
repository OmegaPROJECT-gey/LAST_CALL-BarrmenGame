// LAST CALL - endings core (ROADMAP C4).
//
// The six endings are selected from what the last day accumulated:
// faction standing (Social/SocialCore's ledger), information collected
// (the uncle's wall fragments held in the inventory) and the venue tier
// (GAME-DESIGN growth ladder: dive -> renovated -> VIP -> second floor ->
// underground casino = 1..5). Data/Endings/endings.json declares which
// held items count as information, an ordered list of selection rules
// (first match wins, the last one must be `always` so every state
// resolves) and the six endings - each with five last nights and five
// mornings after, one pair per venue tier (NARRATIVE beat 26: "each
// ending has its own last night and its own morning after"). No good/
// evil axis: a rule reads standing and knowledge, never virtue
// (PHILOSOPHY section 6).
//
// Plain C++17, no Unreal. License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lc {

	/** One venue tier's prose: the localization keys of this ending. */
	struct EndingTier
	{
		int tier = 1;                 // 1..5, load-ordered
		std::string night_key;        // ending.<id>.t<n>.night
		std::string morning_key;      // ending.<id>.t<n>.morning
	};

	/** One of the six endings: bilingual name plus the five tier texts. */
	struct EndingSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::vector<EndingTier> tiers; // exactly five (tier 1..5)

		/** This ending's prose for a venue tier, or nullptr. */
		const EndingTier* text_for(int tier) const;
	};

	/** What the ending screen shows: the chosen ending and its texts. */
	struct EndingChoice
	{
		std::string id; // selected ending id ("" when select() refused)
		std::string name_en;
		std::string name_ru;
		std::string night_key;
		std::string morning_key;
		int tier = 1;
	};

	/** The six endings and the rules that select between them. */
	class EndingLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known fields only, known rule types
		 * only, bilingual names, exactly five tiers per ending, every rule
		 * naming an ending that exists, every text key unique, `min_tier`
		 * <= `max_tier`, and the last rule `always`. Duplicate ending ids
		 * are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		/**
		 * How many pieces of information these held items are: one per
		 * held instance of an item the document lists (the 13 fragments).
		 */
		int count_information(const std::vector<std::string>& held_items) const;

		/**
		 * Walk the selection rules in order; the first match wins.
		 * `standing` is faction id -> value and should carry every faction
		 * the ledger knows (an absent faction reads as neutral and never
		 * tops the list); `information` comes from count_information();
		 * `venue_tier` is 1..5. Refuses (empty choice + `error`) before
		 * load or with a tier outside 1..5.
		 */
		EndingChoice select(const std::vector<std::pair<std::string, double>>& standing,
			int information, int venue_tier, std::string* error = nullptr) const;

		const std::vector<EndingSpec>& endings() const { return endings_; }
		const EndingSpec* find(const std::string& ending_id) const;

	private:
		enum class RuleKind { InfoAtLeast, MaxStandingAtMost, DominantAny, Always };

		struct Rule
		{
			RuleKind kind = RuleKind::Always;
			int min_info = 0;                    // InfoAtLeast
			double max_standing = 0.0;           // MaxStandingAtMost
			std::vector<std::string> factions;   // DominantAny: the dominant set
			double min_standing = 0.0;           // DominantAny
			int min_tier = 1;                    // DominantAny window
			int max_tier = 5;
			std::string ending;                  // ending this rule selects
		};

		std::vector<std::string> info_items_;
		std::vector<Rule> rules_;
		std::vector<EndingSpec> endings_;
	};

} // namespace lc
