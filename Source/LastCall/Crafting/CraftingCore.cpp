// LAST CALL - crafting core implementation. See header for the contract.

#include "Crafting/CraftingCore.h"

#include "Core/JsonCore.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

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

		bool is_lower_id(const std::string& text)
		{
			for (const char c : text)
			{
				const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
				if (!ok)
				{
					return false;
				}
			}
			return !text.empty();
		}

		bool parse_entry(const JsonValue& entry, CraftSpec& out, std::string* error)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "craft is not an object");
			}

			CraftSpec spec;
			if (!entry.get_string("id", spec.id) || !is_lower_id(spec.id))
			{
				return refuse(error, "craft needs a lowercase `id`");
			}
			const std::string where = "craft `" + spec.id + "`: ";

			if (!entry.get_string("kind", spec.kind)
				|| (spec.kind != "infusion" && spec.kind != "syrup"))
			{
				return refuse(error, where + "`kind` must be infusion or syrup");
			}

			const JsonValue* name = entry.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", spec.name_en) || spec.name_en.empty()
				|| !name->get_string("ru", spec.name_ru) || spec.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}

			const JsonValue* inputs = entry.find("inputs");
			if (!inputs || inputs->kind != JsonValue::Kind::Array || inputs->array.empty())
			{
				return refuse(error, where + "`inputs` must be a non-empty array");
			}
			for (const JsonValue& input : inputs->array)
			{
				if (input.kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + "input is not an object");
				}
				CraftInput item;
				if (!input.get_string("item", item.item) || !is_lower_id(item.item)
					|| !input.get_number("amount", item.amount) || item.amount <= 0.0)
				{
					return refuse(error, where + "inputs need an `item` id and a positive `amount`");
				}
				spec.inputs.push_back(std::move(item));
			}

			if (!entry.get_number("time_minutes", spec.time_minutes) || spec.time_minutes <= 0.0)
			{
				return refuse(error, where + "`time_minutes` must be positive");
			}
			if (!entry.get_number("strength", spec.strength)
				|| spec.strength < 0.0 || spec.strength > 1.0)
			{
				return refuse(error, where + "`strength` must be within 0..1");
			}
			if (!entry.get_string("output_item", spec.output_item) || !is_lower_id(spec.output_item))
			{
				return refuse(error, where + "`output_item` must be an id");
			}
			if (entry.find("output_amount") != nullptr
				&& (!entry.get_int("output_amount", spec.output_amount) || spec.output_amount < 1))
			{
				return refuse(error, where + "`output_amount` must be a whole 1 or more");
			}
			if (!entry.get_string("failure_item", spec.failure_item) || !is_lower_id(spec.failure_item))
			{
				return refuse(error, where + "`failure_item` must be an id");
			}

			out = std::move(spec);
			return true;
		}

		bool parse_document(const std::string& json_text, std::vector<CraftSpec>& out,
			std::string* error)
		{
			JsonValue doc;
			std::string parse_error;
			if (!json_parse(json_text, doc, &parse_error))
			{
				return refuse(error, "bad JSON: " + parse_error);
			}
			if (doc.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "crafting document is not an object");
			}

			const JsonValue* crafts = doc.find("crafts");
			if (!crafts || crafts->kind != JsonValue::Kind::Array || crafts->array.empty())
			{
				return refuse(error, "`crafts` must be a non-empty array");
			}
			for (const JsonValue& entry : crafts->array)
			{
				CraftSpec spec;
				if (!parse_entry(entry, spec, error))
				{
					return false;
				}
				out.push_back(std::move(spec));
			}
			return true;
		}

	} // namespace

	// --- library -----------------------------------------------------------------------

	bool CraftLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<CraftSpec> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (CraftSpec& spec : parsed)
		{
			if (find(spec.id))
			{
				return refuse(error, "duplicate craft id `" + spec.id + "`");
			}
			specs_.push_back(std::move(spec));
		}
		return true;
	}

	int CraftLibrary::load_directory(const std::string& directory, std::vector<std::string>* errors)
	{
		namespace fs = std::filesystem;

		int loaded = 0;
		std::error_code ec;
		fs::directory_iterator it(directory, ec);
		if (ec)
		{
			if (errors)
			{
				errors->push_back(directory + ": cannot read directory: " + ec.message());
			}
			return 0;
		}

		std::vector<fs::path> files;
		for (const auto& entry : it)
		{
			std::error_code entry_ec;
			if (entry.is_regular_file(entry_ec) && entry.path().extension() == ".json")
			{
				files.push_back(entry.path());
			}
		}
		std::sort(files.begin(), files.end());

		for (const fs::path& file : files)
		{
			std::ifstream input(file);
			if (!input)
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": unreadable");
				}
				continue;
			}
			std::stringstream buffer;
			buffer << input.rdbuf();

			std::vector<CraftSpec> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			for (CraftSpec& spec : parsed)
			{
				if (find(spec.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate craft id `"
							+ spec.id + "`");
					}
					continue;
				}
				specs_.push_back(std::move(spec));
				++loaded;
			}
		}
		return loaded;
	}

	const CraftSpec* CraftLibrary::find(const std::string& id) const
	{
		for (const CraftSpec& spec : specs_)
		{
			if (spec.id == id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> CraftLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(specs_.size());
		for (const CraftSpec& spec : specs_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	// --- attempt -----------------------------------------------------------------------

	CraftResult attempt_craft(const CraftSpec& spec, double minutes, double strength,
		const CraftTuning& tuning)
	{
		const double time_off = std::fabs(minutes - spec.time_minutes);
		const double strength_off = std::fabs(strength - spec.strength);

		CraftResult result;
		if (time_off <= tuning.perfect_time_minutes
			&& strength_off <= tuning.perfect_strength)
		{
			result.outcome = CraftOutcome::Perfect;
			result.item = spec.output_item;
			result.amount = spec.output_amount;
			result.quality = 1.0;
			return result;
		}
		if (time_off <= tuning.usable_time_minutes
			&& strength_off <= tuning.usable_strength)
		{
			// Weak or overdone, still the right liquid on the shelf.
			result.outcome = CraftOutcome::Usable;
			result.item = spec.output_item;
			result.amount = spec.output_amount;
			result.quality = tuning.sloppy_quality;
			return result;
		}
		result.outcome = CraftOutcome::Failed;
		result.item = spec.failure_item;
		result.amount = 1;
		result.quality = 0.0;
		return result;
	}

} // namespace lc
