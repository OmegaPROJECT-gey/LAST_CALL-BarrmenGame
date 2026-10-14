// LAST CALL - raid director core (ROADMAP B4).
//
// The police search: evidence items in the bar get hide-tags, the NPC
// partner drags what they can into hiding, and the player's distraction
// dialogue check decides whether the cops stop to listen or push through
// to the back room (GAME-DESIGN 5.2). Failure is heat and fines, never
// game over - the bar stays open and the city remembers.
//
// Engine-independent C++17; the UE side runs the check as dialogue and
// animates the search. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	// Tuning mirrors the "raid" section of Data/Balance/core.json.
	struct RaidTuning
	{
		double distraction_threshold = 0.5; // check score that keeps them at the door
		double fail_heat = 2.0;             // base heat when anything is found
		double per_item_heat = 1.0;         // each found item stirs the city
		double fail_fine = 25.0;            // per item the search turns up
	};

	/** What is in the bar when the knock comes. */
	struct RaidState
	{
		std::vector<std::string> evidence; // hide-tagged items
		int partner_capacity = 0;          // how many the partner can hide in time
	};

	struct RaidResult
	{
		bool distraction_worked = false;
		int hidden_by_partner = 0;
		int found = 0;
		double heat_delta = 0.0;
		double fine = 0.0;
	};

	/**
	 * Resolve the search. The partner hides up to their capacity no
	 * matter what; the distraction check decides whether the police
	 * leave with nothing or walk in on whatever is still out. Nothing
	 * found means nothing happens - a failed check over an empty bar is
	 * just an uncomfortable evening.
	 */
	RaidResult resolve_raid(const RaidState& state, double distraction_score,
		const RaidTuning& tuning);

} // namespace lc
