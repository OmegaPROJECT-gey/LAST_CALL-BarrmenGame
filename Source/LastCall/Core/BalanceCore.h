// LAST CALL - balance file core (engine-side tuning; F1/F2 groundwork).
//
// Data/Balance/core.json is the single source of truth for tuning: every
// core already asserts its defaults against it in the unit tests. This
// loader is the piece the ENGINE reads at boot - the phase durations
// behind ULCPhaseManager, the pour tuning behind FLCPourSolver, the
// starting money behind the save. Plain C++17 like every core, so the
// parse itself is unit-tested without Unreal.
//
// Sections the engine does not consume yet (customer, tips, ...) stay
// ignored here - their cores own them, and the schema plus
// lc-data-lint own the whole file's shape.
//
// License: MIT (LICENSE).

#pragma once

#include <string>

#include "Bar/PourSolverCore.h"
#include "Bar/RaidCore.h"
#include "Core/DayScheduleCore.h"

namespace lc {

	struct BalanceData
	{
		DayScheduleSpec phases;
		PourTuning pour;
		int starting_money = 2000; // economy.starting_money == SaveState default
		// economy.*: the day's standing charges (GAME-DESIGN 5.4 money
		// sinks, A3) - paid once when a new day opens; the wallet may go
		// red, a run in the red is the loan the design allows.
		int daily_stand_cost = 200; // the pitch, per day
		int bar_staff_wages = 350;  // the staff, per day
		RaidTuning raid;            // raid.*: the search's heat, fines and check
	};

	/**
	 * Strict parse: `phases`, `pour`, the three `economy.*` numbers and
	 * `raid` must be present and sane - a balance file missing a
	 * section is a broken install, not a license to run on silently
	 * wrong numbers. Unknown sections are ignored (schema +
	 * lc-data-lint own the file).
	 */
	bool load_balance(const std::string& json_text, BalanceData& out, std::string* error);

	/** Same, straight from a file path (what the engine calls at boot). */
	bool load_balance_file(const std::string& path, BalanceData& out, std::string* error);

} // namespace lc
