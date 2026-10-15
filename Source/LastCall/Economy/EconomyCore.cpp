// LAST CALL - economy core implementation. See header for the contract.

#include "Economy/EconomyCore.h"

#include "Core/JsonCore.h"

#include <cstdint>
#include <cstdio>

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

		std::string money_string(double value)
		{
			// Prices are whole-ish numbers; two decimals is all the UI shows.
			char buffer[32];
			std::snprintf(buffer, sizeof(buffer), "%.2f", value);
			return buffer;
		}

	} // namespace

	// --- suppliers --------------------------------------------------------------------

	bool SupplierSpec::sells_liquid(const std::string& liquid_id) const
	{
		for (const std::string& liquid : sells)
		{
			if (liquid == liquid_id)
			{
				return true;
			}
		}
		return false;
	}

	bool SupplierLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "supplier document is not an object");
		}

		const JsonValue* suppliers = doc.find("suppliers");
		if (!suppliers || suppliers->kind != JsonValue::Kind::Object
			|| suppliers->object.empty())
		{
			return refuse(error, "`suppliers` must be a non-empty object");
		}

		for (const auto& member : suppliers->object)
		{
			SupplierSpec spec;
			spec.id = member.first;
			spec.index = static_cast<int>(suppliers_.size());
			const std::string where = "supplier `" + spec.id + "`: ";
			const JsonValue& body = member.second;
			if (body.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "not an object");
			}

			const JsonValue* name = body.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", spec.name_en) || spec.name_en.empty()
				|| !name->get_string("ru", spec.name_ru) || spec.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}
			if (!body.get_string("tier", spec.tier)
				|| (spec.tier != "cheap" && spec.tier != "standard" && spec.tier != "premium"))
			{
				return refuse(error, where + "`tier` must be cheap, standard or premium");
			}
			if (!body.get_number("price_mult", spec.price_mult) || spec.price_mult <= 0.0)
			{
				return refuse(error, where + "`price_mult` must be positive");
			}
			if (!body.get_int("stock_limit", spec.stock_limit) || spec.stock_limit < 0)
			{
				return refuse(error, where + "`stock_limit` must be a non-negative whole number");
			}
			if (!body.get_number("night_risk", spec.night_risk)
				|| spec.night_risk < 0.0 || spec.night_risk > 1.0)
			{
				return refuse(error, where + "`night_risk` must be within 0..1");
			}

			const JsonValue* wares = body.find("sells");
			if (!wares || wares->kind != JsonValue::Kind::Array || wares->array.empty())
			{
				return refuse(error, where + "`sells` must be a non-empty array");
			}
			for (const JsonValue& liquid : wares->array)
			{
				if (liquid.kind != JsonValue::Kind::String || liquid.string.empty())
				{
					return refuse(error, where + "`sells` entries must be liquid ids");
				}
				for (const std::string& existing : spec.sells)
				{
					if (existing == liquid.string)
					{
						return refuse(error, where + "duplicate ware `" + liquid.string + "`");
					}
				}
				spec.sells.push_back(liquid.string);
			}

			for (const SupplierSpec& existing : suppliers_)
			{
				if (existing.id == spec.id)
				{
					return refuse(error, "duplicate supplier id `" + spec.id + "`");
				}
			}
			suppliers_.push_back(std::move(spec));
		}
		return true;
	}

	const SupplierSpec* SupplierLibrary::find(const std::string& supplier_id) const
	{
		for (const SupplierSpec& spec : suppliers_)
		{
			if (spec.id == supplier_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> SupplierLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(suppliers_.size());
		for (const SupplierSpec& spec : suppliers_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	// --- prices -----------------------------------------------------------------------

	double price_drift(int day, int supplier_index, const PriceTuning& tuning)
	{
		if (tuning.drift_amplitude <= 0.0)
		{
			return 1.0;
		}
		// FNV-1a over (day, supplier), then avalanche: cheap, stable, no <random>.
		std::uint32_t hash = 2166136261u;
		const auto mix = [&hash](std::uint32_t value)
		{
			for (int byte = 0; byte < 4; ++byte)
			{
				hash ^= (value >> (byte * 8)) & 0xffu;
				hash *= 16777619u;
			}
		};
		mix(static_cast<std::uint32_t>(day));
		mix(static_cast<std::uint32_t>(supplier_index));
		hash ^= hash >> 13;
		hash *= 0x5bd1e995u;
		hash ^= hash >> 15;

		const double unit = (hash % 1000u) / 999.0; // 0..1 inclusive
		return 1.0 + tuning.drift_amplitude * (2.0 * unit - 1.0);
	}

	double unit_price(double base_price, const SupplierSpec& supplier, int day,
		const PriceTuning& tuning)
	{
		return base_price * supplier.price_mult * price_drift(day, supplier.index, tuning);
	}

	// --- ledger -----------------------------------------------------------------------

	MorningLedger::MorningLedger(double money, const PriceTuning& tuning)
		: money_(money)
		, tuning_(tuning)
	{
	}

	void MorningLedger::open_day(const DayCostTuning& costs)
	{
		// One charge per morning, before the shopping decisions; no
		// refusal here by design - the sinks bite (GDD 5.4).
		money_ -= costs.stand_cost + costs.wages;
	}

	// --- upgrades --------------------------------------------------------------

	bool UpgradeLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "upgrade document is not an object");
		}

		const JsonValue* upgrades = doc.find("upgrades");
		if (!upgrades || upgrades->kind != JsonValue::Kind::Object
			|| upgrades->object.empty())
		{
			return refuse(error, "`upgrades` must be a non-empty object");
		}

		for (const auto& member : upgrades->object)
		{
			UpgradeSpec spec;
			spec.id = member.first;
			const std::string where = "upgrade `" + spec.id + "`: ";
			const JsonValue& body = member.second;
			if (body.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "not an object");
			}

			const JsonValue* name = body.find("name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", spec.name_en) || spec.name_en.empty()
				|| !name->get_string("ru", spec.name_ru) || spec.name_ru.empty())
			{
				return refuse(error, where + "`name` needs en and ru");
			}
			if (!body.get_int("cost", spec.cost) || spec.cost < 1)
			{
				return refuse(error, where + "`cost` must be a positive whole number");
			}

			upgrades_.push_back(std::move(spec));
		}
		return true;
	}

	const UpgradeSpec* UpgradeLibrary::find(const std::string& upgrade_id) const
	{
		for (const UpgradeSpec& spec : upgrades_)
		{
			if (spec.id == upgrade_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	bool MorningLedger::buy_upgrade(const UpgradeSpec& spec,
		const std::vector<std::string>& owned, std::string* error)
	{
		for (const std::string& id : owned)
		{
			if (id == spec.id)
			{
				return refuse(error, "upgrade `" + spec.id + "` is already bought");
			}
		}
		if (spec.cost > money_)
		{
			return refuse(error, "need " + money_string(static_cast<double>(spec.cost))
				+ ", wallet has " + money_string(money_));
		}
		money_ -= static_cast<double>(spec.cost);
		spent_ += static_cast<double>(spec.cost);
		return true;
	}

	int MorningLedger::stock_used(const std::string& supplier_id) const
	{
		for (const auto& entry : stock_)
		{
			if (entry.first == supplier_id)
			{
				return entry.second;
			}
		}
		return 0;
	}

	bool MorningLedger::purchase(const SupplierSpec& supplier, const std::string& liquid_id,
		double base_price, int units, int day, std::string* error)
	{
		if (units <= 0)
		{
			return refuse(error, "units must be positive");
		}
		if (!supplier.sells_liquid(liquid_id))
		{
			return refuse(error, "supplier `" + supplier.id + "` does not sell `" + liquid_id + "`");
		}
		const int used = stock_used(supplier.id);
		if (used + units > supplier.stock_limit)
		{
			return refuse(error, "stock limit of `" + supplier.id + "` is "
				+ std::to_string(supplier.stock_limit) + " units per morning ("
				+ std::to_string(used) + " already bought)");
		}

		const double total = unit_price(base_price, supplier, day, tuning_) * units;
		if (total > money_)
		{
			return refuse(error, "need " + money_string(total) + ", wallet has "
				+ money_string(money_));
		}

		money_ -= total;
		spent_ += total;
		for (auto& entry : stock_)
		{
			if (entry.first == supplier.id)
			{
				entry.second += units;
				return true;
			}
		}
		stock_.emplace_back(supplier.id, units);
		return true;
	}

} // namespace lc
