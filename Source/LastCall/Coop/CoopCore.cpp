// LAST CALL - co-op roles core implementation. See header for the contract.

#include "Coop/CoopCore.h"

#include "Core/JsonCore.h"

#include <algorithm>
#include <initializer_list>

namespace lc {

	// The job vocabulary of GAME-DESIGN 5.5; Data/schemas/coop.schema.json
	// enums the same set and the unit tests pin the two together.
	const std::vector<std::string>& action_vocabulary()
	{
		static const std::vector<std::string> kActions = {
			"pour", "combo", "dialogue",
			"carry", "seat", "clear", "relay",
			"door", "threat", "crowd", "night_security",
			"music", "mood", "cue"
		};
		return kActions;
	}

	namespace {

		bool refuse(std::string* error, const std::string& reason)
		{
			if (error)
			{
				*error = reason;
			}
			return false;
		}

		bool is_id(const std::string& text)
		{
			if (text.empty())
			{
				return false;
			}
			for (const char c : text)
			{
				if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
				{
					return false;
				}
			}
			return true;
		}

		bool has_only(const JsonValue& body, std::initializer_list<const char*> allowed,
			const std::string& where, std::string* error)
		{
			for (const auto& member : body.object)
			{
				bool known = false;
				for (const char* field : allowed)
				{
					if (member.first == field)
					{
						known = true;
						break;
					}
				}
				if (!known)
				{
					return refuse(error, where + "unknown field `" + member.first + "`");
				}
			}
			return true;
		}

		bool read_localized(const JsonValue& body, const char* key,
			const std::string& where, std::string& en, std::string& ru, std::string* error)
		{
			const JsonValue* text = body.find(key);
			if (!text || text->kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "needs a bilingual `" + std::string(key) + "`");
			}
			if (!has_only(*text, { "en", "ru" },
				where + "`" + std::string(key) + "`: ", error))
			{
				return false;
			}
			if (!text->get_string("en", en) || en.empty()
				|| !text->get_string("ru", ru) || ru.empty())
			{
				return refuse(error, where + "needs a bilingual `" + std::string(key) + "`");
			}
			return true;
		}

	} // namespace

	bool RoleLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "co-op document is not an object");
		}
		if (error)
		{
			error->clear();
		}
		if (!has_only(doc, { "$schema", "players", "roles" }, "co-op document: ", error))
		{
			return false;
		}

		const JsonValue* players = doc.find("players");
		if (!players || players->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "`players` must be an object with min and max");
		}
		if (!has_only(*players, { "min", "max" }, "`players`: ", error))
		{
			return false;
		}
		int min_players = 0;
		int max_players = 0;
		if (!players->get_int("min", min_players) || min_players < 2)
		{
			return refuse(error, "`players.min` must be an integer >= 2 (co-op starts at two)");
		}
		if (!players->get_int("max", max_players) || max_players < min_players)
		{
			return refuse(error, "`players.max` must be an integer >= `players.min`");
		}

		const JsonValue* roles = doc.find("roles");
		if (!roles || roles->kind != JsonValue::Kind::Array || roles->array.empty())
		{
			return refuse(error, "`roles` must be a non-empty array");
		}

		std::vector<RoleSpec> loaded;
		for (const JsonValue& entry : roles->array)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "role is not an object");
			}
			if (!has_only(entry, { "id", "name", "job", "actions", "crowd" }, "role: ", error))
			{
				return false;
			}

			RoleSpec role;
			if (!entry.get_string("id", role.id) || !is_id(role.id))
			{
				return refuse(error, "role without a valid `id`");
			}
			for (const RoleSpec& previous : loaded)
			{
				if (previous.id == role.id)
				{
					return refuse(error, "duplicate role id `" + role.id + "`");
				}
			}
			const std::string where = "role `" + role.id + "`: ";
			if (!read_localized(entry, "name", where, role.name_en, role.name_ru, error)
				|| !read_localized(entry, "job", where, role.job_en, role.job_ru, error))
			{
				return false;
			}

			const JsonValue* actions = entry.find("actions");
			if (!actions || actions->kind != JsonValue::Kind::Array || actions->array.empty())
			{
				return refuse(error, where + "`actions` must be a non-empty array");
			}
			for (const JsonValue& action : actions->array)
			{
				if (action.kind != JsonValue::Kind::String)
				{
					return refuse(error, where + "`actions` must hold strings");
				}
				const std::string& action_id = action.string;
				if (std::find(action_vocabulary().begin(), action_vocabulary().end(),
					action_id) == action_vocabulary().end())
				{
					return refuse(error, where + "`" + action_id
						+ "` is not in the action vocabulary");
				}
				if (std::find(role.actions.begin(), role.actions.end(), action_id)
					!= role.actions.end())
				{
					return refuse(error, where + "action `" + action_id + "` listed twice");
				}
				role.actions.push_back(action_id);
			}

			if (!entry.get_int("crowd", role.crowd) || role.crowd < 1)
			{
				return refuse(error, where + "`crowd` must be an integer >= 1");
			}

			loaded.push_back(std::move(role));
		}

		roles_ = std::move(loaded);
		min_players_ = min_players;
		max_players_ = max_players;
		return true;
	}

	const RoleSpec* RoleLibrary::find(const std::string& role_id) const
	{
		for (const RoleSpec& role : roles_)
		{
			if (role.id == role_id)
			{
				return &role;
			}
		}
		return nullptr;
	}

	bool RoleLibrary::can(const std::string& role_id, const std::string& action) const
	{
		const RoleSpec* role = find(role_id);
		if (!role)
		{
			return false;
		}
		return std::find(role->actions.begin(), role->actions.end(), action)
			!= role->actions.end();
	}

	std::string RoleLibrary::assign_blocker(int player_count,
		const std::vector<std::string>& role_ids) const
	{
		if (player_count < min_players_)
		{
			return "co-op runs from " + std::to_string(min_players_) + " players";
		}
		if (player_count > max_players_)
		{
			return "co-op stops at " + std::to_string(max_players_) + " players";
		}
		if (static_cast<int>(role_ids.size()) != player_count)
		{
			return "one role per player";
		}

		bool any_can_pour = false;
		for (const RoleSpec& role : roles_)
		{
			if (can(role.id, "pour"))
			{
				any_can_pour = true;
				break;
			}
		}

		bool crew_can_pour = false;
		std::vector<std::string> taken;
		for (const std::string& role_id : role_ids)
		{
			const RoleSpec* role = find(role_id);
			if (!role)
			{
				return "unknown role `" + role_id + "`";
			}
			if (std::find(taken.begin(), taken.end(), role_id) != taken.end())
			{
				return "role `" + role_id + "` twice in one crew";
			}
			taken.push_back(role_id);
			if (can(role->id, "pour"))
			{
				crew_can_pour = true;
			}
		}

		if (any_can_pour && !crew_can_pour)
		{
			return "someone must pour: no assigned role works `pour`";
		}
		return std::string();
	}

	int RoleLibrary::crowd_capacity(const std::vector<std::string>& role_ids) const
	{
		int capacity = 0;
		for (const std::string& role_id : role_ids)
		{
			const RoleSpec* role = find(role_id);
			if (!role)
			{
				return 0; // an illegal crew handles nobody
			}
			capacity += role->crowd;
		}
		return capacity;
	}

	double RoleLibrary::crowd_load(int crowd,
		const std::vector<std::string>& role_ids) const
	{
		const int capacity = crowd_capacity(role_ids);
		if (capacity < 1 || crowd < 1)
		{
			return 0.0;
		}
		return static_cast<double>(crowd) / static_cast<double>(capacity);
	}

} // namespace lc
