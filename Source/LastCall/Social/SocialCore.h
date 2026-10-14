// LAST CALL - social consequences core (ROADMAP A5).
//
// Faction-visible memory without a good/evil meter: Data/Factions/
// factions.json registers the five factions with bilingual names and
// initial standing, and maps consequence flags (ConsequenceLogCore) to
// standing deltas - reaction tables, one reaction per flag. The Standing
// Ledger applies each reaction exactly once, no matter how often the flag
// is recorded, and is fed by the consequence log.
//
// Direct standing effects in dialogue (DialogueRunnerCore) are the people
// in the room reacting to your face; these tables are the city finding
// out afterwards. Plain C++17, no Unreal. License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lc {

	class ConsequenceLog; // Core/ConsequenceLogCore.h

	struct FactionSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		double initial_standing = 0.0;
	};

	struct Reaction
	{
		std::string flag;                                   // consequence flag id
		std::vector<std::pair<std::string, double>> standing; // faction id -> delta
		std::string note_en;                                // why the city reacted
		std::string note_ru;
	};

	/** Factions and reaction tables from Data/Factions/factions.json. */
	class FactionLibrary
	{
	public:
		/**
		 * Parse the document. Strict: every faction bilingual with an
		 * initial standing; every reaction names a known faction, a
		 * unique flag and a bilingual note. Duplicate faction or flag
		 * ids are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		const FactionSpec* find(const std::string& faction_id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<FactionSpec>& factions() const { return factions_; }
		const std::vector<Reaction>& reactions() const { return reactions_; }

		/** The reaction table row for a flag, or nullptr. */
		const Reaction* reaction_for(const std::string& flag) const;

	private:
		std::vector<FactionSpec> factions_;
		std::vector<Reaction> reactions_;
	};

	/**
	 * What the city remembers: standing per faction, starting from the
	 * library's initial values. Each reaction fires once per flag -
	 * repeating a consequence changes nothing (ConsequenceLogCore already
	 * collapsed repeats per day; this collapses across days).
	 * The library must outlive the ledger.
	 */
	class StandingLedger
	{
	public:
		explicit StandingLedger(const FactionLibrary& library);

		double standing(const std::string& faction_id) const; // unknown faction: 0
		void set_standing(const std::string& faction_id, double value); // saves/restores

		/**
		 * Fire the reaction for `flag` if it has not fired yet.
		 * Returns how many factions moved (0 = no table row or already
		 * applied).
		 */
		int apply_flag(const std::string& flag);

		/** Apply every flag recorded in the log, once each; returns factions moved. */
		int apply_log(const ConsequenceLog& log);

		bool applied(const std::string& flag) const;

	private:
		const FactionLibrary* library_;
		std::vector<std::pair<std::string, double>> standing_; // faction id -> value
		std::vector<std::string> applied_;                     // flags already fired
	};

} // namespace lc
