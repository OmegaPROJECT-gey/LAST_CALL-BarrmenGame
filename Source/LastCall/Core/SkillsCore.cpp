// LAST CALL - skill tree core implementation. See header for the contract.

#include "Core/SkillsCore.h"

#include "Core/JsonCore.h"

#include <algorithm>

namespace lc {

	namespace {

		bool refuse(std::string* error, const std::string& reason)
		{
			if (error)
			{
				*error = reason;
			}
			return false;
		}

		bool is_mult_type(const std::string& type)
		{
			return type == "service_speed" || type == "tip_multiplier"
				|| type == "pour_shake" || type == "combo_window";
		}

		bool owns(const std::vector<std::string>& owned, const std::string& node_id)
		{
			for (const std::string& id : owned)
			{
				if (id == node_id)
				{
					return true;
				}
			}
			return false;
		}

	} // namespace

	bool SkillTree::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "skills document is not an object");
		}

		const JsonValue* branches = doc.find("branches");
		if (!branches || branches->kind != JsonValue::Kind::Object || branches->object.empty())
		{
			return refuse(error, "`branches` must be a non-empty object");
		}

		std::vector<SkillNode> parsed;
		for (const auto& branch_member : branches->object)
		{
			const std::string& branch_id = branch_member.first;
			const JsonValue& branch = branch_member.second;
			if (branch.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "branch `" + branch_id + "`: not an object");
			}
			const JsonValue* nodes = branch.find("nodes");
			if (!nodes || nodes->kind != JsonValue::Kind::Array || nodes->array.empty())
			{
				return refuse(error, "branch `" + branch_id + "`: `nodes` must be a non-empty array");
			}

			for (const JsonValue& node_value : nodes->array)
			{
				const std::string where = "branch `" + branch_id + "`";
				if (node_value.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + ": node is not an object");
				}

				SkillNode node;
				node.branch = branch_id;
				if (!node_value.get_string("id", node.id) || node.id.empty())
				{
					return refuse(error, where + ": node without `id`");
				}
				const std::string node_where = where + " node `" + node.id + "`";
				if (!node_value.get_number("cost", node.cost) || node.cost < 0.0)
				{
					return refuse(error, node_where + ": `cost` must be a non-negative number");
				}

				const JsonValue* requires_list = node_value.find("requires");
				if (requires_list)
				{
					if (requires_list->kind != JsonValue::Kind::Array)
					{
						return refuse(error, node_where + ": `requires` must be an array");
					}
					for (const JsonValue& requirement : requires_list->array)
					{
						if (requirement.kind != JsonValue::Kind::String
							|| requirement.string.empty())
						{
							return refuse(error, node_where + ": `requires` entries must be node ids");
						}
						node.requirements.push_back(requirement.string);
					}
				}

				const JsonValue* effect_value = node_value.find("effect");
				if (effect_value)
				{
					if (effect_value->kind != JsonValue::Kind::Object)
					{
						return refuse(error, node_where + ": `effect` must be an object");
					}
					node.has_effect = true;
					if (!effect_value->get_string("type", node.effect.type)
						|| node.effect.type.empty())
					{
						return refuse(error, node_where + ": effect without `type`");
					}
					if (is_mult_type(node.effect.type))
					{
						if (!effect_value->get_number("mult", node.effect.mult)
							|| node.effect.mult <= 0.0)
						{
							return refuse(error, node_where + ": effect `mult` must be positive");
						}
					}
					else if (node.effect.type == "unlock_ingredients")
					{
						const JsonValue* ids = effect_value->find("ids");
						if (!ids || ids->kind != JsonValue::Kind::Array || ids->array.empty())
						{
							return refuse(error, node_where + ": unlock_ingredients needs `ids`");
						}
						for (const JsonValue& id : ids->array)
						{
							if (id.kind != JsonValue::Kind::String || id.string.empty())
							{
								return refuse(error, node_where + ": `ids` entries must be non-empty");
							}
							node.effect.unlock_ids.push_back(id.string);
						}
					}
					else if (node.effect.type != "reveal_tells" && node.effect.type != "reveal_lies")
					{
						return refuse(error, node_where + ": unknown effect type `"
							+ node.effect.type + "`");
					}
				}

				for (const SkillNode& existing : parsed)
				{
					if (existing.id == node.id)
					{
						return refuse(error, "duplicate node id `" + node.id + "`");
					}
				}
				parsed.push_back(std::move(node));
			}
		}

		// Requirements may point forward (validated once every id is known).
		for (const SkillNode& node : parsed)
		{
			for (const std::string& requirement : node.requirements)
			{
				bool found = false;
				for (const SkillNode& other : parsed)
				{
					if (other.id == requirement)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					return refuse(error, "node `" + node.id + "` requires unknown node `"
						+ requirement + "`");
				}
			}
		}

		nodes_ = std::move(parsed);
		return true;
	}

	const SkillNode* SkillTree::find(const std::string& node_id) const
	{
		for (const SkillNode& node : nodes_)
		{
			if (node.id == node_id)
			{
				return &node;
			}
		}
		return nullptr;
	}

	std::vector<std::string> SkillTree::ids() const
	{
		std::vector<std::string> out;
		out.reserve(nodes_.size());
		for (const SkillNode& node : nodes_)
		{
			out.push_back(node.id);
		}
		return out;
	}

	std::vector<std::string> SkillTree::available(const std::vector<std::string>& owned) const
	{
		std::vector<std::string> out;
		for (const SkillNode& node : nodes_)
		{
			if (owns(owned, node.id))
			{
				continue;
			}
			bool requirements_met = true;
			for (const std::string& requirement : node.requirements)
			{
				if (!owns(owned, requirement))
				{
					requirements_met = false;
					break;
				}
			}
			if (requirements_met)
			{
				out.push_back(node.id);
			}
		}
		return out;
	}

	bool SkillTree::can_buy(const std::string& node_id, const std::vector<std::string>& owned,
		double money, double* cost_out) const
	{
		const SkillNode* node = find(node_id);
		if (!node || owns(owned, node_id))
		{
			return false;
		}
		for (const std::string& requirement : node->requirements)
		{
			if (!owns(owned, requirement))
			{
				return false;
			}
		}
		if (cost_out)
		{
			*cost_out = node->cost;
		}
		return money >= node->cost;
	}

	bool SkillTree::buy(const std::string& node_id, std::vector<std::string>& owned,
		double& money) const
	{
		if (!can_buy(node_id, owned, money))
		{
			return false;
		}
		money -= find(node_id)->cost;
		owned.push_back(node_id);
		return true;
	}

	SkillSummary SkillTree::summarize(const std::vector<std::string>& owned) const
	{
		SkillSummary summary;
		for (const SkillNode& node : nodes_)
		{
			if (!owns(owned, node.id) || !node.has_effect)
			{
				continue;
			}
			const SkillEffect& effect = node.effect;
			if (effect.type == "service_speed")
			{
				summary.service_speed_mult *= effect.mult;
			}
			else if (effect.type == "tip_multiplier")
			{
				summary.tip_multiplier *= effect.mult;
			}
			else if (effect.type == "pour_shake")
			{
				summary.pour_shake_mult *= effect.mult;
			}
			else if (effect.type == "combo_window")
			{
				summary.combo_window_mult *= effect.mult;
			}
			else if (effect.type == "reveal_tells")
			{
				summary.reveal_tells = true;
			}
			else if (effect.type == "reveal_lies")
			{
				summary.reveal_lies = true;
			}
			else if (effect.type == "unlock_ingredients")
			{
				for (const std::string& id : effect.unlock_ids)
				{
					if (!owns(summary.unlocked_ingredients, id))
					{
						summary.unlocked_ingredients.push_back(id);
					}
				}
			}
		}
		std::sort(summary.unlocked_ingredients.begin(), summary.unlocked_ingredients.end());
		return summary;
	}

} // namespace lc
