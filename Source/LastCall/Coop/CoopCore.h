// LAST CALL - co-op roles core (ROADMAP D2, GAME-DESIGN 5.5).
//
// The crew as data: Data/Coop/roles.json declares the co-op bounds
// (2-4 players) and the four roles of GDD section 5.5 - bartender,
// waiter, bouncer, DJ - each with the job vocabulary it works. The core
// answers the two questions the engine asks without Unreal:
//
//   can(role, action)         - the job split: the bartender can't seat
//                               people, the bouncer can't pour; the input
//                               bindings gate on it.
//   assign_blocker(count, []) - is this crew legal for a session: bounds,
//                               one role per player, no duplicates, known
//                               roles, and someone must pour (the
//                               bartender IS the single-player loop).
//
// Listen server, reconnect-as-pause and crowd load are engine work -
// they are not this file.
//
// Plain C++17, no Unreal. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	/** The known job actions (GDD 5.5); the schema enums the same set. */
	const std::vector<std::string>& action_vocabulary();

	/** One role of the crew. */
	struct RoleSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::string job_en;
		std::string job_ru;
		std::vector<std::string> actions; // vocabulary: pour, seat, door, ...
		int crowd = 0;                   // bodies this role keeps up with
	};

	/** The roles of Data/Coop/roles.json. */
	class RoleLibrary
	{
	public:
		/**
		 * Parse the document. Strict: known fields only, bilingual
		 * names and jobs, unique role ids, at least one action per
		 * role drawn from the known vocabulary, players bounded at
		 * min >= 2 and max >= min. A refusal leaves the previous
		 * library intact.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		const std::vector<RoleSpec>& roles() const { return roles_; }
		const RoleSpec* find(const std::string& role_id) const;

		int min_players() const { return min_players_; }
		int max_players() const { return max_players_; }

		/**
		 * Does this role work this action? Unknown role or unknown
		 * action reads false - the engine treats false as "not bound
		 * for this player".
		 */
		bool can(const std::string& role_id, const std::string& action) const;

		/**
		 * "" when a session with `player_count` players taking exactly
		 * `role_ids` is legal, else the refusal: out of bounds, a
		 * player without a role, a repeated role, an unknown role, or
		 * a crew with nobody to pour.
		 */
		std::string assign_blocker(int player_count,
			const std::vector<std::string>& role_ids) const;

		/**
		 * Bodies the assigned crew keeps up with - the sum of its
		 * roles' crowd (GDD 5.5: under load the roles split). An
		 * unknown role reads 0: an illegal crew handles nobody. The
		 * venue ladder is the yardstick - a duo runs a dive, the full
		 * crew runs the casino.
		 */
		int crowd_capacity(const std::vector<std::string>& role_ids) const;

		/**
		 * crowd / capacity - how hard that crew works. 0.0 is an empty
		 * room, 1.0 the crew at its limit, above 1.0 overrun (the
		 * engine scales mood decay and the room's music on it).
		 */
		double crowd_load(int crowd, const std::vector<std::string>& role_ids) const;

	private:
		std::vector<RoleSpec> roles_;
		int min_players_ = 2;
		int max_players_ = 4;
	};

} // namespace lc
