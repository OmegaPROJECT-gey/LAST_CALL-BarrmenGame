// LAST CALL - save state core (ARCHITECTURE.md, section 9).
//
// What a save file actually contains: the day, the phase the clock is in,
// money and heat. Written as a small versioned text block so it is
// debuggable and testable without Unreal; ULCSaveSystem (milestone F2)
// stores this payload in the engine's SaveGame slot and writes it only at
// phase boundaries - the DayClock's save_requested flag.
//
// One slot, day granularity, undo-free: consequences stick (PHILOSOPHY 3.2).
//
// License: MIT (LICENSE).

#pragma once

#include <string>

#include "Core/DayScheduleCore.h"

namespace lc {

	// Bump when the payload shape changes; older saves are refused, not
	// silently migrated (a lost run is honest, a corrupted one is not).
	constexpr int kSaveVersion = 1;

	struct SaveState
	{
		int version = kSaveVersion;
		int day = 1;              // 1-based
		Phase phase = Phase::Morning;
		int money = 2000;         // balance: starting_money; loans can push it below zero
		double heat = 0.0;        // police attention, only grows
	};

	// Deterministic text payload ("lc-save <version>" header + key=value
	// lines in fixed order). Same state -> same bytes.
	std::string serialize(const SaveState& state);

	// Strict parse: header, exact version, every field present and sane.
	// Returns false (leaving `out` untouched) for garbage, version mismatch,
	// missing or unknown fields, and out-of-range values.
	bool deserialize(const std::string& text, SaveState& out);

} // namespace lc
