// LAST CALL - day schedule core (ARCHITECTURE.md, sections 2 and 4).
//
// The state machine behind ULCLPhaseManager: which phase the day is in,
// how long it has been there, and where the save boundary fires. Like the
// pour solver and the customer reaction core this is plain C++17 with no
// Unreal types, so milestone F2 logic is testable before the engine exists.
//
// License: MIT (LICENSE).

#pragma once

#include <vector>

namespace lc {

	/** The four phases of one in-game day (GAME-DESIGN.md, section 2). */
	enum class Phase : int
	{
		Morning = 0, // Prep: purchasing, upgrades, crafting
		Shift = 1,   // Day/Evening: pouring, customers, social
		Night = 2,   // After closing: action/stealth
		Summary = 3  // Tips, XP, standing, heat -> next day
	};

	// Tuning mirrors the "phases" section of Data/Balance/core.json;
	// 0 seconds means the phase is untimed and ends by event (the shift
	// ends when the bar closes, not when a clock runs out).
	struct DayScheduleSpec
	{
		double morning_seconds = 300.0;
		double shift_seconds = 0.0;
		double night_seconds = 0.0;
		double summary_seconds = 20.0;
	};

	// Duration of a phase in seconds; 0 = untimed.
	double duration_for(const DayScheduleSpec& spec, Phase phase);

	struct DayClock
	{
		Phase phase = Phase::Morning;
		int day = 1;              // 1-based
		double phase_time_s = 0.0; // time spent in the current phase
		double day_time_s = 0.0;   // time since the day started (Morning)
		bool save_requested = false; // set at every boundary; the save system
		                             // (milestone F2) consumes and clears it
	};

	struct PhaseTransition
	{
		bool changed = false;
		bool new_day = false;
		Phase from = Phase::Morning;
		Phase to = Phase::Morning;
	};

	/**
	 * Jump forward within the day (skipping phases is allowed - GAME-DESIGN 2:
	 * phases are skippable). Backward jumps and no-op requests are refused:
	 * the day only moves forward, consequences stick (PHILOSOPHY 3.2).
	 * Summary -> Morning is only reachable through advance_phase, which
	 * starts the next day.
	 */
	PhaseTransition request_phase(DayClock& clock, Phase target);

	/** Advance one phase; Summary wraps into Morning of the next day. */
	PhaseTransition advance_phase(DayClock& clock);

	/**
	 * Accumulate time. A timed phase auto-advances when its duration is
	 * reached (returns the transition); untimed phases just accumulate.
	 * The vector holds at most one transition - overflow time is not
	 * carried across a boundary.
	 */
	std::vector<PhaseTransition> tick(DayClock& clock, double dt_s, const DayScheduleSpec& spec);

	/**
	 * Share of the current phase's duration spent (>= 1.0 means done);
	 * untimed phases report 0.
	 */
	double phase_progress(const DayClock& clock, const DayScheduleSpec& spec);

} // namespace lc
