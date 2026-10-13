// LAST CALL - dialogue runner core implementation. See header for the contract.

#include "Dialogue/DialogueRunnerCore.h"

#include "Core/JsonCore.h"

namespace lc {

	// --- world ----------------------------------------------------------------------

	bool DialogueWorld::has_flag(const std::string& flag) const
	{
		for (const auto& entry : flags)
		{
			if (entry.first == flag)
			{
				return entry.second;
			}
		}
		return false;
	}

	void DialogueWorld::set_flag(const std::string& flag, bool value)
	{
		for (auto& entry : flags)
		{
			if (entry.first == flag)
			{
				entry.second = value;
				return;
			}
		}
		flags.emplace_back(flag, value);
	}

	double DialogueWorld::standing_of(const std::string& faction) const
	{
		for (const auto& entry : standing)
		{
			if (entry.first == faction)
			{
				return entry.second;
			}
		}
		return 0.0;
	}

	void DialogueWorld::add_standing(const std::string& faction, double delta)
	{
		for (auto& entry : standing)
		{
			if (entry.first == faction)
			{
				entry.second += delta;
				return;
			}
		}
		standing.emplace_back(faction, delta);
	}

	bool DialogueWorld::owns_skill(const std::string& skill_id) const
	{
		for (const std::string& owned : skills)
		{
			if (owned == skill_id)
			{
				return true;
			}
		}
		return false;
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

		struct CondSets
		{
			std::vector<std::pair<std::string, bool>> flags;
			std::vector<std::pair<std::string, double>> standing;
			std::vector<std::pair<std::string, bool>> skills;
		};

		struct EffSets
		{
			std::vector<std::pair<std::string, bool>> flags;
			std::vector<std::pair<std::string, double>> standing;
			std::vector<AppliedEffect> routed;
		};

		bool parse_conditions(const JsonValue& owner, const std::string& where,
			CondSets& out, std::string* error)
		{
			const JsonValue* conditions = owner.find("conditions");
			if (!conditions)
			{
				return true; // optional
			}
			if (conditions->kind != JsonValue::Kind::Array)
			{
				return refuse(error, where + ": `conditions` must be an array");
			}
			for (const JsonValue& condition : conditions->array)
			{
				if (condition.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + ": condition is not an object");
				}
				std::string type;
				if (!condition.get_string("type", type))
				{
					return refuse(error, where + ": condition without `type`");
				}
				if (type == "story_flag")
				{
					std::string flag;
					bool value = true;
					if (!condition.get_string("flag", flag))
					{
						return refuse(error, where + ": story_flag condition without `flag`");
					}
					condition.get_bool("value", value); // default: must be true
					out.flags.emplace_back(flag, value);
				}
				else if (type == "faction_standing")
				{
					std::string faction;
					double minimum = 0.0;
					if (!condition.get_string("faction", faction)
						|| !condition.get_number("min", minimum))
					{
						return refuse(error, where + ": faction_standing needs `faction` and `min`");
					}
					out.standing.emplace_back(faction, minimum);
				}
				else if (type == "skill")
				{
					std::string skill_id;
					bool owned = true;
					if (!condition.get_string("id", skill_id))
					{
						return refuse(error, where + ": skill condition without `id`");
					}
					condition.get_bool("owned", owned); // default: must be owned
					out.skills.emplace_back(skill_id, owned);
				}
				else
				{
					return refuse(error, where + ": unknown condition type `" + type + "`");
				}
			}
			return true;
		}

		bool parse_effects(const JsonValue& owner, const std::string& where,
			EffSets& out, std::string* error)
		{
			const JsonValue* effects = owner.find("effects");
			if (!effects)
			{
				return true; // optional
			}
			if (effects->kind != JsonValue::Kind::Array)
			{
				return refuse(error, where + ": `effects` must be an array");
			}
			for (const JsonValue& effect : effects->array)
			{
				if (effect.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + ": effect is not an object");
				}
				std::string type;
				if (!effect.get_string("type", type))
				{
					return refuse(error, where + ": effect without `type`");
				}
				if (type == "story_flag")
				{
					std::string flag;
					bool value = true;
					if (!effect.get_string("flag", flag))
					{
						return refuse(error, where + ": story_flag effect without `flag`");
					}
					effect.get_bool("value", value);
					out.flags.emplace_back(flag, value);
				}
				else if (type == "faction_standing")
				{
					std::string faction;
					double delta = 0.0;
					if (!effect.get_string("faction", faction)
						|| !effect.get_number("delta", delta))
					{
						return refuse(error, where + ": faction_standing needs `faction` and `delta`");
					}
					out.standing.emplace_back(faction, delta);
				}
				else if (type == "mood")
				{
					AppliedEffect routed;
					routed.type = "mood";
					if (!effect.get_string("target", routed.target)
						|| !effect.get_number("delta", routed.delta))
					{
						return refuse(error, where + ": mood needs `target` and `delta`");
					}
					out.routed.push_back(std::move(routed));
				}
				else if (type == "quest")
				{
					AppliedEffect routed;
					routed.type = "quest";
					if (!effect.get_string("id", routed.target)
						|| !effect.get_string("state", routed.state))
					{
						return refuse(error, where + ": quest needs `id` and `state`");
					}
					out.routed.push_back(std::move(routed));
				}
				else
				{
					return refuse(error, where + ": unknown effect type `" + type + "`");
				}
			}
			return true;
		}

	} // namespace

	// --- runner ----------------------------------------------------------------------

	bool DialogueRunner::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "dialogue document is not an object");
		}

		std::string id;
		std::string entry;
		doc.get_string("id", id);
		if (!doc.get_string("entry", entry) || entry.empty())
		{
			return refuse(error, "missing or empty `entry`");
		}

		const JsonValue* nodes = doc.find("nodes");
		if (!nodes || nodes->kind != JsonValue::Kind::Object || nodes->object.empty())
		{
			return refuse(error, "`nodes` must be a non-empty object");
		}

		std::vector<DialogueNodeData> parsed;
		for (const auto& member : nodes->object)
		{
			const std::string& node_id = member.first;
			const JsonValue& body = member.second;
			const std::string where = "node `" + node_id + "`";
			if (body.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + ": not an object");
			}

			DialogueNodeData data;
			data.id = node_id;
			body.get_string("speaker", data.speaker);
			if (!body.get_string("text_key", data.text_key) || data.text_key.empty())
			{
				return refuse(error, where + ": missing `text_key`");
			}
			body.get_string("next", data.next);
			body.get_bool("end", data.end);

			CondSets conditions;
			if (!parse_conditions(body, where, conditions, error))
			{
				return false;
			}
			data.flag_conditions = std::move(conditions.flags);
			data.standing_conditions = std::move(conditions.standing);
			data.skill_conditions = std::move(conditions.skills);

			EffSets effects;
			if (!parse_effects(body, where, effects, error))
			{
				return false;
			}
			data.flag_effects = std::move(effects.flags);
			data.standing_effects = std::move(effects.standing);
			data.routed_effects = std::move(effects.routed);

			const JsonValue* choices = body.find("choices");
			if (choices)
			{
				if (choices->kind != JsonValue::Kind::Array)
				{
					return refuse(error, where + ": `choices` must be an array");
				}
				for (const JsonValue& choice_value : choices->array)
				{
					if (choice_value.kind != JsonValue::Kind::Object)
					{
						return refuse(error, where + ": choice is not an object");
					}
					const std::string choice_where = where + " choice";
					DialogueChoiceData choice;
					if (!choice_value.get_string("id", choice.id) || choice.id.empty())
					{
						return refuse(error, choice_where + ": missing `id`");
					}
					if (!choice_value.get_string("text_key", choice.text_key)
						|| choice.text_key.empty())
					{
						return refuse(error, choice_where + ": missing `text_key`");
					}
					if (!choice_value.get_string("next", choice.next) || choice.next.empty())
					{
						return refuse(error, choice_where + ": missing `next`");
					}

					CondSets choice_conditions;
					if (!parse_conditions(choice_value, choice_where, choice_conditions, error))
					{
						return false;
					}
					choice.flag_conditions = std::move(choice_conditions.flags);
					choice.standing_conditions = std::move(choice_conditions.standing);
					choice.skill_conditions = std::move(choice_conditions.skills);

					EffSets choice_effects;
					if (!parse_effects(choice_value, choice_where, choice_effects, error))
					{
						return false;
					}
					choice.flag_effects = std::move(choice_effects.flags);
					choice.standing_effects = std::move(choice_effects.standing);
					choice.routed_effects = std::move(choice_effects.routed);

					data.choices.push_back(std::move(choice));
				}
			}
			parsed.push_back(std::move(data));
		}

		// Graph integrity, mirrored from the CI lint (tools/lc-dialogue-lint).
		auto exists = [&](const std::string& node_id)
		{
			for (const DialogueNodeData& n : parsed)
			{
				if (n.id == node_id)
				{
					return true;
				}
			}
			return false;
		};
		if (!exists(entry))
		{
			return refuse(error, "entry node `" + entry + "` does not exist");
		}
		for (const DialogueNodeData& n : parsed)
		{
			if (!n.next.empty() && !exists(n.next))
			{
				return refuse(error, "node `" + n.id + "` -> unknown node `" + n.next + "`");
			}
			for (const DialogueChoiceData& choice : n.choices)
			{
				if (!exists(choice.next))
				{
					return refuse(error, "node `" + n.id + "` choice `" + choice.id
						+ "` -> unknown node `" + choice.next + "`");
				}
			}
		}

		id_ = std::move(id);
		entry_ = std::move(entry);
		nodes_ = std::move(parsed);
		current_.clear();
		applied_.clear();
		interrupt_ignored_.clear();
		interrupt_heard_.clear();

		const JsonValue* recording = doc.find("interrupt_recording");
		if (recording && recording->kind == JsonValue::Kind::Object)
		{
			recording->get_string("on_ignored", interrupt_ignored_);
			recording->get_string("on_heard", interrupt_heard_);
		}

		loaded_ = true;
		return true;
	}

	const DialogueNodeData* DialogueRunner::node(const std::string& id) const
	{
		for (const DialogueNodeData& n : nodes_)
		{
			if (n.id == id)
			{
				return &n;
			}
		}
		return nullptr;
	}

	void DialogueRunner::apply_effects(
		const std::vector<std::pair<std::string, bool>>& flag_effects,
		const std::vector<std::pair<std::string, double>>& standing_effects,
		const std::vector<AppliedEffect>& routed, DialogueWorld& world)
	{
		for (const auto& effect : flag_effects)
		{
			world.set_flag(effect.first, effect.second);
		}
		for (const auto& effect : standing_effects)
		{
			world.add_standing(effect.first, effect.second);
		}
		for (const AppliedEffect& effect : routed)
		{
			applied_.push_back(effect);
		}
	}

	bool DialogueRunner::conditions_pass(const DialogueWorld& world,
		const std::vector<std::pair<std::string, bool>>& flag_conditions,
		const std::vector<std::pair<std::string, double>>& standing_conditions,
		const std::vector<std::pair<std::string, bool>>& skill_conditions) const
	{
		for (const auto& condition : flag_conditions)
		{
			if (world.has_flag(condition.first) != condition.second)
			{
				return false;
			}
		}
		for (const auto& condition : standing_conditions)
		{
			if (world.standing_of(condition.first) < condition.second)
			{
				return false;
			}
		}
		for (const auto& condition : skill_conditions)
		{
			if (world.owns_skill(condition.first) != condition.second)
			{
				return false;
			}
		}
		return true;
	}

	bool DialogueRunner::start(DialogueWorld& world)
	{
		if (!loaded_)
		{
			return false;
		}
		const DialogueNodeData* entry = node(entry_);
		if (!entry)
		{
			return false;
		}
		if (!conditions_pass(world, entry->flag_conditions, entry->standing_conditions,
				entry->skill_conditions))
		{
			return false;
		}
		current_ = entry->id;
		apply_effects(entry->flag_effects, entry->standing_effects, entry->routed_effects, world);
		return true;
	}

	const std::string& DialogueRunner::current_text_key() const
	{
		static const std::string empty;
		const DialogueNodeData* n = node(current_);
		return n ? n->text_key : empty;
	}

	const std::string& DialogueRunner::current_speaker() const
	{
		static const std::string empty;
		const DialogueNodeData* n = node(current_);
		return n ? n->speaker : empty;
	}

	bool DialogueRunner::at_end() const
	{
		const DialogueNodeData* n = node(current_);
		return n && n->end;
	}

	bool DialogueRunner::can_advance() const
	{
		const DialogueNodeData* n = node(current_);
		return n && !n->end && !n->next.empty();
	}

	std::vector<int> DialogueRunner::available_choices(const DialogueWorld& world) const
	{
		std::vector<int> available;
		const DialogueNodeData* n = node(current_);
		if (!n)
		{
			return available;
		}
		for (std::size_t i = 0; i < n->choices.size(); ++i)
		{
			const DialogueChoiceData& choice = n->choices[i];
			if (conditions_pass(world, choice.flag_conditions, choice.standing_conditions,
					choice.skill_conditions))
			{
				available.push_back(static_cast<int>(i));
			}
		}
		return available;
	}

	bool DialogueRunner::advance(DialogueWorld& world)
	{
		const DialogueNodeData* n = node(current_);
		if (!n || n->end || n->next.empty())
		{
			return false;
		}
		const DialogueNodeData* target = node(n->next);
		if (!target)
		{
			return false;
		}
		if (!conditions_pass(world, target->flag_conditions, target->standing_conditions,
				target->skill_conditions))
		{
			return false; // the data leads somewhere the world does not allow
		}
		current_ = target->id;
		apply_effects(target->flag_effects, target->standing_effects, target->routed_effects, world);
		return true;
	}

	bool DialogueRunner::choose(const std::string& choice_id, DialogueWorld& world)
	{
		const DialogueNodeData* n = node(current_);
		if (!n || n->end)
		{
			return false;
		}
		for (const DialogueChoiceData& choice : n->choices)
		{
			if (choice.id != choice_id)
			{
				continue;
			}
			if (!conditions_pass(world, choice.flag_conditions, choice.standing_conditions,
					choice.skill_conditions))
			{
				return false; // filtered out for this world
			}
			const DialogueNodeData* target = node(choice.next);
			if (!target)
			{
				return false;
			}
			apply_effects(choice.flag_effects, choice.standing_effects, choice.routed_effects, world);
			// Choice effects may satisfy the target's own conditions.
			if (!conditions_pass(world, target->flag_conditions, target->standing_conditions,
					target->skill_conditions))
			{
				return false;
			}
			current_ = target->id;
			apply_effects(target->flag_effects, target->standing_effects, target->routed_effects, world);
			return true;
		}
		return false;
	}

} // namespace lc
