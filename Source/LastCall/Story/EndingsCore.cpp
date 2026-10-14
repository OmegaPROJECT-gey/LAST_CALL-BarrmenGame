// LAST CALL - endings core implementation. See header for the contract.

#include "Story/EndingsCore.h"

#include "Core/JsonCore.h"

#include <initializer_list>
#include <set>

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

		bool is_key(const std::string& text)
		{
			if (text.empty())
			{
				return false;
			}
			for (const char c : text)
			{
				if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.'))
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

		/** An optional scalar integer member of an object (tier windows). */
		bool read_optional_int(const JsonValue& body, const std::string& key,
			int& out, const std::string& where, std::string* error)
		{
			const JsonValue* value = body.find(key);
			if (!value)
			{
				return true; // absent: keep the default
			}
			if (value->kind != JsonValue::Kind::Number
				|| value->number != static_cast<int>(value->number))
			{
				return refuse(error, where + "`" + key + "` must be an integer");
			}
			out = static_cast<int>(value->number);
			return true;
		}

		bool contains(const std::vector<std::string>& list, const std::string& value)
		{
			for (const std::string& item : list)
			{
				if (item == value)
				{
					return true;
				}
			}
			return false;
		}

	} // namespace

	// --- spec --------------------------------------------------------------------------

	const EndingTier* EndingSpec::text_for(int tier) const
	{
		for (const EndingTier& entry : tiers)
		{
			if (entry.tier == tier)
			{
				return &entry;
			}
		}
		return nullptr;
	}

	const EndingSpec* EndingLibrary::find(const std::string& ending_id) const
	{
		for (const EndingSpec& spec : endings_)
		{
			if (spec.id == ending_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	// --- library -----------------------------------------------------------------------

	bool EndingLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "endings document is not an object");
		}
		if (error)
		{
			error->clear();
		}
		if (!has_only(doc, { "$schema", "information", "selection", "endings" },
			"endings document: ", error))
		{
			return false;
		}

		// --- information: which held items are pieces of the archive
		const JsonValue* information = doc.find("information");
		if (!information || information->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "`information` must be an object");
		}
		if (!has_only(*information, { "items" }, "`information`: ", error))
		{
			return false;
		}
		const JsonValue* items = information->find("items");
		if (!items || items->kind != JsonValue::Kind::Array || items->array.empty())
		{
			return refuse(error, "`information.items` must be a non-empty array");
		}
		info_items_.clear();
		for (const JsonValue& item : items->array)
		{
			if (item.kind != JsonValue::Kind::String || !is_id(item.string))
			{
				return refuse(error, "`information.items` entries must be id strings");
			}
			if (contains(info_items_, item.string))
			{
				return refuse(error, "duplicate information item `" + item.string + "`");
			}
			info_items_.push_back(item.string);
		}

		// --- endings: six rows, five tiers each, unique text keys
		const JsonValue* endings = doc.find("endings");
		if (!endings || endings->kind != JsonValue::Kind::Array || endings->array.empty())
		{
			return refuse(error, "`endings` must be a non-empty array");
		}
		std::set<std::string> text_keys;
		endings_.clear();
		for (const JsonValue& entry : endings->array)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "ending is not an object");
			}
			if (!has_only(entry, { "id", "name", "tiers" }, "ending: ", error))
			{
				return false;
			}

			EndingSpec spec;
			if (!entry.get_string("id", spec.id) || !is_id(spec.id))
			{
				return refuse(error, "ending without a valid `id`");
			}
			const std::string where = "ending `" + spec.id + "`: ";
			if (find(spec.id))
			{
				return refuse(error, "duplicate ending id `" + spec.id + "`");
			}
			if (!read_name(entry, where, spec.name_en, spec.name_ru, error))
			{
				return false;
			}

			const JsonValue* tiers = entry.find("tiers");
			if (!tiers || tiers->kind != JsonValue::Kind::Array || tiers->size() != 5)
			{
				return refuse(error, where + "`tiers` must hold exactly five entries");
			}
			bool seen[6] = { false, false, false, false, false, false }; // index by tier
			for (const JsonValue& tier_entry : tiers->array)
			{
				if (tier_entry.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + "tier entry is not an object");
				}
				if (!has_only(tier_entry, { "tier", "night_key", "morning_key" },
					where + "tier: ", error))
				{
					return false;
				}
				EndingTier tier;
				if (!tier_entry.get_int("tier", tier.tier) || tier.tier < 1 || tier.tier > 5)
				{
					return refuse(error, where +"`tier` must be an integer 1..5");
				}
				if (seen[tier.tier])
				{
					return refuse(error, where + "duplicate tier " + std::to_string(tier.tier));
				}
				seen[tier.tier] = true;
				if (!tier_entry.get_string("night_key", tier.night_key)
					|| !is_key(tier.night_key)
					|| !tier_entry.get_string("morning_key", tier.morning_key)
					|| !is_key(tier.morning_key))
				{
					return refuse(error, where + "tier keys must be localization key strings");
				}
				if (!text_keys.insert(tier.night_key).second)
				{
					return refuse(error, where + "duplicate text key `" + tier.night_key + "`");
				}
				if (!text_keys.insert(tier.morning_key).second)
				{
					return refuse(error, where + "duplicate text key `" + tier.morning_key + "`");
				}
				spec.tiers.push_back(std::move(tier));
			}
			endings_.push_back(std::move(spec));
		}

		// --- selection: ordered rules, first match wins, `always` last
		const JsonValue* selection = doc.find("selection");
		if (!selection || selection->kind != JsonValue::Kind::Array || selection->array.empty())
		{
			return refuse(error, "`selection` must be a non-empty array");
		}
		rules_.clear();
		for (const JsonValue& entry : selection->array)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "selection rule is not an object");
			}
			std::string type;
			if (!entry.get_string("type", type))
			{
				return refuse(error, "selection rule without `type`");
			}

			Rule rule;
			if (type == "info_at_least")
			{
				if (!has_only(entry, { "type", "min", "ending" }, "rule `info_at_least`: ", error))
				{
					return false;
				}
				rule.kind = RuleKind::InfoAtLeast;
				if (!entry.get_int("min", rule.min_info) || rule.min_info < 0)
				{
					return refuse(error, "rule `info_at_least`: `min` must be an integer >= 0");
				}
			}
			else if (type == "max_standing_at_most")
			{
				if (!has_only(entry, { "type", "max", "ending" }, "rule `max_standing_at_most`: ", error))
				{
					return false;
				}
				rule.kind = RuleKind::MaxStandingAtMost;
				if (!entry.get_number("max", rule.max_standing))
				{
					return refuse(error, "rule `max_standing_at_most`: `max` must be a number");
				}
			}
			else if (type == "dominant_any")
			{
				if (!has_only(entry, { "type", "factions", "min", "min_tier", "max_tier", "ending" },
					"rule `dominant_any`: ", error))
				{
					return false;
				}
				rule.kind = RuleKind::DominantAny;
				const JsonValue* factions = entry.find("factions");
				if (!factions || factions->kind != JsonValue::Kind::Array || factions->array.empty())
				{
					return refuse(error, "rule `dominant_any`: `factions` must be a non-empty array");
				}
				for (const JsonValue& faction : factions->array)
				{
					if (faction.kind != JsonValue::Kind::String || !is_id(faction.string))
					{
						return refuse(error, "rule `dominant_any`: `factions` entries must be id strings");
					}
					if (contains(rule.factions, faction.string))
					{
						return refuse(error, "rule `dominant_any`: duplicate faction `" + faction.string + "`");
					}
					rule.factions.push_back(faction.string);
				}
				if (!entry.get_number("min", rule.min_standing))
				{
					return refuse(error, "rule `dominant_any`: `min` must be a number");
				}
				if (!read_optional_int(entry, "min_tier", rule.min_tier,
					"rule `dominant_any`: ", error))
				{
					return false;
				}
				if (!read_optional_int(entry, "max_tier", rule.max_tier,
					"rule `dominant_any`: ", error))
				{
					return false;
				}
				if (rule.min_tier < 1 || rule.min_tier > 5 || rule.max_tier < 1 || rule.max_tier > 5
					|| rule.min_tier > rule.max_tier)
				{
					return refuse(error, "rule `dominant_any`: tier window must be within 1..5");
				}
			}
			else if (type == "always")
			{
				if (!has_only(entry, { "type", "ending" }, "rule `always`: ", error))
				{
					return false;
				}
				rule.kind = RuleKind::Always;
			}
			else
			{
				return refuse(error, "selection rule of unknown type `" + type + "`");
			}

			const std::string where = "rule `" + type + "`: ";
			if (!entry.get_string("ending", rule.ending) || !is_id(rule.ending))
			{
				return refuse(error, where + "without a valid `ending`");
			}
			if (!find(rule.ending))
			{
				return refuse(error, where + "unknown ending `" + rule.ending + "`");
			}
			rules_.push_back(std::move(rule));
		}
		if (rules_.back().kind != RuleKind::Always)
		{
			return refuse(error, "the last selection rule must be `always` so every state resolves");
		}

		return true;
	}

	int EndingLibrary::count_information(const std::vector<std::string>& held_items) const
	{
		int count = 0;
		for (const std::string& held : held_items)
		{
			if (contains(info_items_, held))
			{
				++count;
			}
		}
		return count;
	}

	EndingChoice EndingLibrary::select(const std::vector<std::pair<std::string, double>>& standing,
		int information, int venue_tier, std::string* error) const
	{
		EndingChoice choice;
		if (endings_.empty() || rules_.empty())
		{
			refuse(error, "the ending library is not loaded");
			return choice;
		}
		if (venue_tier < 1 || venue_tier > 5)
		{
			refuse(error, "venue tier out of range (1..5)");
			return choice;
		}
		if (error)
		{
			error->clear();
		}

		for (const Rule& rule : rules_)
		{
			bool matched = false;
			switch (rule.kind)
			{
			case RuleKind::InfoAtLeast:
				matched = information >= rule.min_info;
				break;

			case RuleKind::MaxStandingAtMost:
			{
				// Absent factions read as neutral (0.0), so the ceiling
				// never starts below neutral.
				double ceiling = 0.0;
				for (const auto& entry : standing)
				{
					ceiling = entry.second > ceiling ? entry.second : ceiling;
				}
				matched = ceiling <= rule.max_standing;
				break;
			}

			case RuleKind::DominantAny:
			{
				if (venue_tier < rule.min_tier || venue_tier > rule.max_tier || standing.empty())
				{
					break;
				}
				double top = standing.front().second;
				for (const auto& entry : standing)
				{
					top = entry.second > top ? entry.second : top;
				}
				if (top < rule.min_standing)
				{
					break;
				}
				// Collective dominance: every faction sharing the top
				// standing must be listed - a tie with an unlisted faction
				// blocks the match.
				matched = true;
				for (const auto& entry : standing)
				{
					if (entry.second == top && !contains(rule.factions, entry.first))
					{
						matched = false;
						break;
					}
				}
				break;
			}

			case RuleKind::Always:
				matched = true;
				break;
			}

			if (!matched)
			{
				continue;
			}

			const EndingSpec* spec = find(rule.ending);
			if (!spec)
			{
				// Unreachable through load(); kept so a corrupt library
				// refuses instead of returning a half-built choice.
				refuse(error, "selection rule names an ending that is not loaded");
				return EndingChoice();
			}
			const EndingTier* text = spec->text_for(venue_tier);
			if (!text)
			{
				refuse(error, "ending `" + spec->id + "` has no text for this tier");
				return EndingChoice();
			}
			choice.id = spec->id;
			choice.name_en = spec->name_en;
			choice.name_ru = spec->name_ru;
			choice.night_key = text->night_key;
			choice.morning_key = text->morning_key;
			choice.tier = venue_tier;
			return choice;
		}

		refuse(error, "no selection rule matched - the last rule must be `always`");
		return EndingChoice();
	}

} // namespace lc
