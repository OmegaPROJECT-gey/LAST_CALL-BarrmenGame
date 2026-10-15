// LAST CALL - dynamic events core (ROADMAP D3, GAME-DESIGN 5.2).
//
// The morning's weighted pool as data: Data/Events/events.json lists
// the outcomes GDD 5.2 names - a police raid, a rival bar opening, a
// gang meetup, a celebrity visit, a blackout - plus the quiet day that
// keeps the pool honest. Each row carries a base weight, an additive
// bias per standing point of a faction, a per-heat and a per-day slope.
// The weight of the morning is
//
//   weight = base + sum(per_point x faction standing)
//                + per_heat x heat + per_day x day
//
// clamped at zero (an event the world has priced out simply does not
// happen). `select` walks the pool with a roll the engine owns - same
// world state, same roll, same event. Faction ids are named, not
// owned: the unit test cross-checks them against Data/Factions.
//
// Plain C++17, no Unreal. License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lc {

	/** One faction's additive weight per standing point. */
	struct EventStandingBias
	{
		std::string faction;
		double per_point = 0.0;
	};

	/** One row of the morning pool. */
	struct EventSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		double weight_base = 0.0;
		std::vector<EventStandingBias> standing_bias;
		double per_heat = 0.0;
		double per_day = 0.0;
	};

	/** The pool of Data/Events/events.json. */
	class EventLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known fields only, bilingual
		 * names, unique event ids, base weight >= 0, at most one bias
		 * per faction per event, and a row that can actually fire
		 * (base or a slope or a bias). A refusal leaves the previous
		 * pool intact.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		const std::vector<EventSpec>& events() const { return events_; }
		const EventSpec* find(const std::string& event_id) const;

		/**
		 * This event's weight in this world: base + biases x standing
		 * + per_heat x heat + per_day x day (day is 1-based; anything
		 * below 1 counts as 1), clamped at zero. Standing entries the
		 * event does not mention do nothing; an unlisted faction
		 * simply never appears.
		 */
		double weight(const EventSpec& spec,
			const std::vector<std::pair<std::string, double>>& standing,
			double heat, int day) const;

		/**
		 * The morning's pick: `roll` in [0,1) walks the pool in file
		 * order over the total weight (the engine owns the RNG).
		 * Returns "" when the pool weighs nothing in this world, the
		 * id of the chosen event otherwise - never an event whose
		 * weight is zero.
		 */
		std::string select(const std::vector<std::pair<std::string, double>>& standing,
			double heat, int day, double roll) const;

	private:
		std::vector<EventSpec> events_;
	};

} // namespace lc
