// LAST CALL - dynamic events core implementation. See header for the contract.

#include "Events/EventCore.h"

#include "Core/JsonCore.h"

#include <cmath>
#include <initializer_list>

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

		bool read_name(const JsonValue& body, const std::string& where,
			std::string& en, std::string& ru, std::string* error)
		{
			const JsonValue* name = body.find("name");
			if (!name || name->kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "needs a bilingual `name` (en and ru)");
			}
			if (!has_only(*name, { "en", "ru" }, where + "`name`: ", error))
			{
				return false;
			}
			if (!name->get_string("en", en) || en.empty()
				|| !name->get_string("ru", ru) || ru.empty())
			{
				return refuse(error, where + "needs a bilingual `name` (en and ru)");
			}
			return true;
		}

	} // namespace

	bool EventLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "events document is not an object");
		}
		if (error)
		{
			error->clear();
		}
		if (!has_only(doc, { "$schema", "events" }, "events document: ", error))
		{
			return false;
		}

		const JsonValue* events = doc.find("events");
		if (!events || events->kind != JsonValue::Kind::Array || events->array.empty())
		{
			return refuse(error, "`events` must be a non-empty array");
		}

		std::vector<EventSpec> loaded;
		for (const JsonValue& entry : events->array)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "event is not an object");
			}
			if (!has_only(entry, { "id", "name", "weight_base", "standing_bias",
				"per_heat", "per_day" }, "event: ", error))
			{
				return false;
			}

			EventSpec spec;
			if (!entry.get_string("id", spec.id) || !is_id(spec.id))
			{
				return refuse(error, "event without a valid `id`");
			}
			for (const EventSpec& previous : loaded)
			{
				if (previous.id == spec.id)
				{
					return refuse(error, "duplicate event id `" + spec.id + "`");
				}
			}
			const std::string where = "event `" + spec.id + "`: ";
			if (!read_name(entry, where, spec.name_en, spec.name_ru, error))
			{
				return false;
			}

			if (!entry.get_number("weight_base", spec.weight_base) || spec.weight_base < 0.0)
			{
				return refuse(error, where + "`weight_base` must be a number >= 0");
			}
			if (!entry.get_number("per_heat", spec.per_heat))
			{
				return refuse(error, where + "`per_heat` must be a number");
			}
			if (!entry.get_number("per_day", spec.per_day))
			{
				return refuse(error, where + "`per_day` must be a number");
			}

			const JsonValue* biases = entry.find("standing_bias");
			if (!biases || biases->kind != JsonValue::Kind::Array)
			{
				return refuse(error, where + "`standing_bias` must be an array");
			}
			for (const JsonValue& bias : biases->array)
			{
				if (bias.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + "a standing bias is not an object");
				}
				if (!has_only(bias, { "faction", "per_point" },
					where + "standing bias: ", error))
				{
					return false;
				}
				EventStandingBias row;
				if (!bias.get_string("faction", row.faction) || !is_id(row.faction))
				{
					return refuse(error, where + "a bias without a valid `faction`");
				}
				if (!bias.get_number("per_point", row.per_point))
				{
					return refuse(error, where + "bias `" + row.faction + "` needs a `per_point` number");
				}
				for (const EventStandingBias& previous : spec.standing_bias)
				{
					if (previous.faction == row.faction)
					{
						return refuse(error, where + "faction `" + row.faction + "` biased twice");
					}
				}
				spec.standing_bias.push_back(std::move(row));
			}

			if (spec.weight_base == 0.0 && spec.per_heat == 0.0 && spec.per_day == 0.0
				&& spec.standing_bias.empty())
			{
				return refuse(error, where + "an event with no weight anywhere can never fire");
			}

			loaded.push_back(std::move(spec));
		}

		events_ = std::move(loaded);
		return true;
	}

	const EventSpec* EventLibrary::find(const std::string& event_id) const
	{
		for (const EventSpec& spec : events_)
		{
			if (spec.id == event_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	double EventLibrary::weight(const EventSpec& spec,
		const std::vector<std::pair<std::string, double>>& standing,
		double heat, int day) const
	{
		double value = spec.weight_base;
		for (const EventStandingBias& bias : spec.standing_bias)
		{
			for (const auto& entry : standing)
			{
				if (entry.first == bias.faction)
				{
					value += bias.per_point * entry.second;
					break;
				}
			}
		}
		const double day_points = day >= 1 ? static_cast<double>(day) : 1.0;
		value += spec.per_heat * heat;
		value += spec.per_day * day_points;
		return value > 0.0 ? value : 0.0;
	}

	std::string EventLibrary::select(
		const std::vector<std::pair<std::string, double>>& standing,
		double heat, int day, double roll) const
	{
		double total = 0.0;
		for (const EventSpec& spec : events_)
		{
			total += weight(spec, standing, heat, day);
		}
		if (total <= 0.0)
		{
			return std::string(); // nothing can happen in this world
		}

		// Clamp the roll into [0, 1); a NaN roll reads as 0.
		double clamped = roll;
		if (!(clamped > 0.0))
		{
			clamped = 0.0;
		}
		if (clamped > 0.9999999)
		{
			clamped = 0.9999999;
		}
		const double threshold = clamped * total;

		double seen = 0.0;
		std::string last_positive;
		for (const EventSpec& spec : events_)
		{
			const double event_weight = weight(spec, standing, heat, day);
			if (event_weight <= 0.0)
			{
				continue; // priced out of this morning
			}
			last_positive = spec.id;
			seen += event_weight;
			if (threshold < seen)
			{
				return spec.id;
			}
		}
		return last_positive; // float tail: settle on the last live row
	}

} // namespace lc
