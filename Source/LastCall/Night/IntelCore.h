// LAST CALL - investigations core (ROADMAP B5).
//
// Eavesdropped conversations are inventory: tokens collected during
// shifts are spent at night on intel - who meets whom, which shipment,
// which cop is bought (GAME-DESIGN 4.3). Each intel row of
// Data/Intel/intel.json costs tokens and grants a story flag that
// dialogue trees and faction arcs condition on - information opens
// doors that force cannot, which is the whole point of the night's
// third path.
//
// Engine-independent C++17; the UE side collects tokens and shows the
// dossier. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct IntelSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		int cost = 0;             // eavesdrop tokens to buy it
		std::string grants_flag;  // story flag it unlocks
		std::string note_en;      // what the player learns
		std::string note_ru;
	};

	/** Intel rows from Data/Intel/intel.json. */
	class IntelLibrary
	{
	public:
		/**
		 * Parse the document. Strict: lowercase ids, bilingual names
		 * and notes, cost 0 or more, a flag id; duplicate ids are
		 * refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		const IntelSpec* find(const std::string& id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<IntelSpec>& intel() const { return intel_; }

	private:
		std::vector<IntelSpec> intel_;
	};

	/** The player's dossier: tokens on hand and what they already know. */
	class IntelLedger
	{
	public:
		explicit IntelLedger(const IntelLibrary& library); // the library must outlive it

		int tokens() const { return tokens_; }
		void add_tokens(int count); // collected while eavesdropping

		bool knows(const std::string& intel_id) const;

		/** Can this intel be bought right now? */
		bool can_afford(const std::string& intel_id) const;

		/**
		 * Buy intel: spends its tokens and records it (once). Returns
		 * false without spending when it is unknown, unaffordable or
		 * already known. The granted flag is readable via granted_flag.
		 */
		bool learn(const std::string& intel_id);

		/** The story flag a learned intel grants, or "" if not learned. */
		std::string granted_flag(const std::string& intel_id) const;

	private:
		const IntelLibrary* library_;
		int tokens_ = 0;
		std::vector<std::string> known_;
	};

} // namespace lc
