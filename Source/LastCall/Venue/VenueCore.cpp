// LAST CALL - venue progression core implementation. See header for the contract.

#include "Venue/VenueCore.h"

#include "Core/JsonCore.h"

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

	bool VenueLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "venue document is not an object");
		}
		if (error)
		{
			error->clear();
		}
		if (!has_only(doc, { "$schema", "tiers" }, "venue document: ", error))
		{
			return false;
		}

		const JsonValue* tiers = doc.find("tiers");
		if (!tiers || tiers->kind != JsonValue::Kind::Array || tiers->array.empty())
		{
			return refuse(error, "`tiers` must be a non-empty array");
		}

		// Built off to the side: a refusal leaves the previous ladder intact.
		std::vector<VenueTier> loaded;
		for (std::size_t position = 0; position < tiers->array.size(); ++position)
		{
			const JsonValue& entry = tiers->array[position];
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "tier is not an object");
			}
			if (!has_only(entry, { "index", "id", "name", "upgrade_cost", "layout",
				"capacity", "risk", "music_set" }, "tier: ", error))
			{
				return false;
			}

			VenueTier tier;
			const int want_index = static_cast<int>(position) + 1;
			if (!entry.get_int("index", tier.index))
			{
				return refuse(error, "tier without an integer `index`");
			}
			if (tier.index != want_index)
			{
				return refuse(error, "tier at position " + std::to_string(want_index)
					+ " declares index " + std::to_string(tier.index)
					+ " - the ladder must be contiguous from 1");
			}
			const std::string where = "tier " + std::to_string(tier.index) + ": ";

			if (!entry.get_string("id", tier.id) || !is_id(tier.id))
			{
				return refuse(error, where + "without a valid `id`");
			}
			for (const VenueTier& previous : loaded)
			{
				if (previous.id == tier.id)
				{
					return refuse(error, "duplicate tier id `" + tier.id + "`");
				}
			}

			if (!read_name(entry, where, tier.name_en, tier.name_ru, error))
			{
				return false;
			}

			if (!entry.get_int("upgrade_cost", tier.upgrade_cost) || tier.upgrade_cost < 0)
			{
				return refuse(error, where + "`upgrade_cost` must be an integer >= 0");
			}
			if (position == 0)
			{
				if (tier.upgrade_cost != 0)
				{
					return refuse(error, "the first tier is where the game starts - `upgrade_cost` must be 0");
				}
			}
			else if (tier.upgrade_cost <= loaded.back().upgrade_cost)
			{
				return refuse(error, where + "prices must grow with the ladder");
			}

			if (!entry.get_string("layout", tier.layout) || !is_id(tier.layout))
			{
				return refuse(error, where + "needs a valid `layout` id");
			}
			if (!entry.get_string("music_set", tier.music_set) || !is_id(tier.music_set))
			{
				return refuse(error, where + "needs a valid `music_set` id");
			}

			if (!entry.get_int("capacity", tier.capacity) || tier.capacity < 1)
			{
				return refuse(error, where + "`capacity` must be an integer >= 1");
			}
			if (position > 0 && tier.capacity <= loaded.back().capacity)
			{
				return refuse(error, where + "capacity must grow with the ladder");
			}

			if (!entry.get_number("risk", tier.risk) || tier.risk <= 0.0)
			{
				return refuse(error, where + "`risk` must be a number > 0");
			}
			if (position > 0 && tier.risk < loaded.back().risk)
			{
				return refuse(error, where + "risk must not fall as the room grows");
			}

			loaded.push_back(std::move(tier));
		}

		tiers_ = std::move(loaded);
		return true;
	}

	const VenueTier* VenueLibrary::tier(int index) const
	{
		if (index < 1 || index > count())
		{
			return nullptr;
		}
		return &tiers_[static_cast<std::size_t>(index - 1)];
	}

	const VenueTier* VenueLibrary::find_id(const std::string& tier_id) const
	{
		for (const VenueTier& tier_spec : tiers_)
		{
			if (tier_spec.id == tier_id)
			{
				return &tier_spec;
			}
		}
		return nullptr;
	}

	VenueAdvance VenueLibrary::plan_advance(int current_index, int money) const
	{
		VenueAdvance plan;
		const VenueTier* current = tier(current_index);
		if (!current)
		{
			plan.blocker = "unknown venue tier";
			return plan;
		}
		if (current_index >= count())
		{
			plan.blocker = "the venue stands at the top of its ladder";
			return plan;
		}

		const VenueTier& next = tiers_[static_cast<std::size_t>(current_index)];
		if (money < next.upgrade_cost)
		{
			plan.blocker = "the treasury is short by "
				+ std::to_string(next.upgrade_cost - money);
			return plan;
		}

		plan.possible = true;
		plan.to_index = next.index;
		plan.cost = next.upgrade_cost;
		return plan;
	}

} // namespace lc
