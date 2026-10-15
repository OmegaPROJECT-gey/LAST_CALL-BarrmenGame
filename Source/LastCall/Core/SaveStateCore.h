// LAST CALL - save state core (ARCHITECTURE.md, section 9).
//
// What a save file actually contains: the day, the phase the clock is in,
// money, heat, the faction standing the city remembers (A5), the
// hospital stay the story sits out (B1), the story flags the
// dialogues recorded (C2 - consequences stick, PHILOSOPHY 3.2) and
// the bar upgrades the morning bought (A3). Written as a small
// versioned text block so it is debuggable and testable
// without Unreal; ULCSaveSystem (milestone F2) stores this payload in
// the engine's SaveGame slot and writes it only at phase boundaries -
// the DayClock's save_requested flag.
//
// One slot, day granularity, undo-free: consequences stick (PHILOSOPHY 3.2).
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Core/DayScheduleCore.h"

namespace lc {

	// Bump when the payload shape changes; older saves are refused, not
	// silently migrated (a lost run is honest, a corrupted one is not).
	// v2 adds venue_tier (D1: the growth ladder position).
	// v3 adds standing (A5: the faction ledger the city reacts with).
	// v4 adds hospital_days (B1: the stay the story sits out survives).
	// v5 adds flags (C2: the dialogue consequences survive a save).
	// v6 adds upgrades (A3: the bought bar upgrades survive a save).
	constexpr int kSaveVersion = 6;

	// The growth ladder of Data/Venue/venue.json spans these bounds; the
	// save test cross-checks kMaxVenueTier against the real file.
	constexpr int kMinVenueTier = 1;
	constexpr int kMaxVenueTier = 5;

	struct SaveState
	{
		int version = kSaveVersion;
		int day = 1;              // 1-based
		Phase phase = Phase::Morning;
		int money = 2000;         // balance: starting_money; loans can push it below zero
		double heat = 0.0;        // police attention, only grows
		int venue_tier = kMinVenueTier; // growth ladder position (Data/Venue)
		// Hospital days still to sit out (B1); spent in one jump at the
		// Summary -> Morning boundary, so a save taken while the story
		// is in the hospital keeps the stay.
		int hospital_days = 0;
		// Faction id -> standing, the ledger's rows; sorted by id on
		// write, refused with duplicates or malformed ids on read.
		std::vector<std::pair<std::string, double>> standing;
		// Story flag -> value, what the dialogue choices recorded (C2);
		// sorted by id on write, refused with duplicates or malformed
		// ids on read - the conditions read this after a load.
		std::vector<std::pair<std::string, bool>> flags;
		// Bar upgrade ids the morning bought (A3); sorted by id on
		// write, refused with duplicates or malformed ids on read -
		// buy_upgrade() refuses these, so the shop stays one-time.
		std::vector<std::string> upgrades;
	};

	// Deterministic text payload ("lc-save <version>" header + key=value
	// lines in fixed order). Same state -> same bytes.
	std::string serialize(const SaveState& state);

	// Strict parse: header, exact version, every field present and sane.
	// Returns false (leaving `out` untouched) for garbage, version mismatch,
	// missing or unknown fields, and out-of-range values.
	bool deserialize(const std::string& text, SaveState& out);

} // namespace lc
