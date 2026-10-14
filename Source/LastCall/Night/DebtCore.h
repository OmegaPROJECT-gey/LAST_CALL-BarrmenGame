// LAST CALL - debt runs core (ROADMAP B3).
//
// After closing you walk the city blocks to collect what the bar is
// owed. Each debtor of Data/Debts/debtors.json resists talk and
// intimidation differently and may hold collateral; the encounter
// resolves by the approach you pick and the check score you bring
// (GAME-DESIGN 4.2: talk / intimidate / take something). Night combat
// rules apply - lethal is avoidable, and the intended wins are the
// non-lethal ones: violence is worth money today and costs heat and
// standing tomorrow.
//
// Engine-independent C++17; the UE side runs the blocks and the checks.
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct DebtorRecord
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::string block;   // which city block they hide in
		std::string faction; // who they answer to (Data/Factions ids)
		int owed = 0;
		double talk_resist = 0.5;       // check score that talks them dry
		double intimidate_resist = 0.5; // check score that empties them by fear
		std::string collateral;         // item seized on "take", "" = nothing worth taking
	};

	/** Debtors from Data/Debts/debtors.json. */
	class DebtorLibrary
	{
	public:
		/**
		 * Parse the document. Strict: lowercase ids, bilingual names,
		 * owed 0 or more, resistances within 0..1; duplicate ids are
		 * refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		const DebtorRecord* find(const std::string& id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<DebtorRecord>& debtors() const { return debtors_; }

	private:
		std::vector<DebtorRecord> debtors_;
	};

	struct DebtAttempt
	{
		std::string approach; // "talk" | "intimidate" | "take"
		double score = 0.0;   // how the check went, 0..1
		bool violent = false; // you did it with weapons drawn
	};

	enum class DebtResolution
	{
		Paid,     // full debt collected, nobody hurt
		Partial,  // half (or a quarter after a fight), something collected
		Escaped,  // they got away; nothing collected
		Fight     // they stood up to you; you loot what you can and the block hears
	};

	struct DebtOutcome
	{
		DebtResolution resolution = DebtResolution::Escaped;
		int collected = 0;
		double heat_delta = 0.0;
		double standing_delta = 0.0;
		std::string collateral; // seized item, "" if none
	};

	// Tuning mirrors the "debt" section of Data/Balance/core.json.
	struct DebtTuning
	{
		double partial_ratio = 0.5;     // share of the debt collected short of full
		double fight_loot_ratio = 0.25; // share looted after a fight
		double take_heat = 1.0;         // seizing collateral gets reported
		double fight_heat = 2.5;        // a fight on a block does not stay quiet
		double violence_heat = 2.0;     // extra heat for doing it armed
		double violence_standing = -1.0; // and the faction remembers
	};

	/**
	 * Resolve one encounter. Talk and intimidate pay in full on a
	 * passing score, half on a near miss (talk), and end in escape
	 * (talk) or a fight (intimidate); taking something is always
	 * partial plus heat; any violence piles on heat and cools standing.
	 * Unknown approaches read as an escape.
	 */
	DebtOutcome resolve_debt(const DebtorRecord& debtor, const DebtAttempt& attempt,
		const DebtTuning& tuning);

} // namespace lc
