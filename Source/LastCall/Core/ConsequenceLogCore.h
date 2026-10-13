// LAST CALL - consequence log core (milestone F3: "interrupting has a
// recorded consequence").
//
// Choices are not vibes: every consequence the narrative cares about is a
// recorded flag with the day and phase it happened in. Dialogue trees set
// them, faction reactions (A5) and endings (C4) read them. Plain C++17,
// testable without Unreal; the UE side persists the log inside the save
// payload (SaveStateCore grows a section for it when F3 lands).
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct ConsequenceEntry
	{
		std::string flag; // stable id, e.g. "eavesdrop_interrupted"
		int day = 1;      // 1-based, when it happened
		std::string phase; // "Morning" | "Shift" | "Night" | "Summary"
	};

	class ConsequenceLog
	{
	public:
		/**
		 * Record a consequence. Repeats of the same flag on the same day
		 * collapse into one entry - what matters for the story is that it
		 * happened, not how many times the player pressed the button.
		 * Returns true when the entry was new.
		 */
		bool record(const std::string& flag, int day, const std::string& phase);

		/** Has this consequence ever been recorded? */
		bool has(const std::string& flag) const;

		/** On how many distinct days it was recorded. */
		int count(const std::string& flag) const;

		/** Everything, in the order it happened. */
		const std::vector<ConsequenceEntry>& entries() const { return log_; }

	private:
		std::vector<ConsequenceEntry> log_;
	};

} // namespace lc
