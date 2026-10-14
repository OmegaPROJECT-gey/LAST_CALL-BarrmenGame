// LAST CALL - destructibility core implementation. See header for the contract.

#include "Bar/InteractionCore.h"

#include "Core/JsonCore.h"

#include <algorithm>
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

		bool is_known_kind(const std::string& kind)
		{
			return kind == "weapon" || kind == "throwable"
				|| kind == "cover" || kind == "extinguisher";
		}

		bool is_known_phase(const std::string& phase)
		{
			// The interaction set is shared: the shift and the night only.
			return phase == "Shift" || phase == "Night";
		}

		bool parse_entry(const JsonValue& entry, InteractionSpec& out, std::string* error)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "interaction is not an object");
			}

			InteractionSpec spec;
			if (!entry.get_string("id", spec.id) || !is_lower_id(spec.id))
			{
				return refuse(error, "interaction needs a lowercase `id`");
			}
			const std::string where = "interaction `" + spec.id + "`: ";

			const JsonValue* name = entry.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", spec.name_en) || spec.name_en.empty()
				|| !name->get_string("ru", spec.name_ru) || spec.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}

			const JsonValue* kinds = entry.find("kinds");
			if (!kinds || kinds->kind != JsonValue::Kind::Array || kinds->array.empty())
			{
				return refuse(error, where + "`kinds` must be a non-empty array");
			}
			for (const JsonValue& kind : kinds->array)
			{
				if (kind.kind != JsonValue::Kind::String || !is_known_kind(kind.string))
				{
					return refuse(error, where + "kinds must be weapon, throwable, cover or extinguisher");
				}
				if (std::find(spec.kinds.begin(), spec.kinds.end(), kind.string) != spec.kinds.end())
				{
					return refuse(error, where + "duplicate kind `" + kind.string + "`");
				}
				spec.kinds.push_back(kind.string);
			}

			if (!entry.get_number("damage", spec.damage) || spec.damage < 0.0)
			{
				return refuse(error, where + "`damage` must be 0 or more");
			}
			if (!entry.get_int("sturdiness", spec.sturdiness) || spec.sturdiness < 1)
			{
				return refuse(error, where + "`sturdiness` must be a whole 1 or more");
			}

			const JsonValue* phases = entry.find("phases");
			if (!phases || phases->kind != JsonValue::Kind::Array || phases->array.empty())
			{
				return refuse(error, where + "`phases` must be a non-empty array");
			}
			for (const JsonValue& phase : phases->array)
			{
				if (phase.kind != JsonValue::Kind::String || !is_known_phase(phase.string))
				{
					return refuse(error, where + "phases must be Shift or Night");
				}
				if (std::find(spec.phases.begin(), spec.phases.end(), phase.string) != spec.phases.end())
				{
					return refuse(error, where + "duplicate phase `" + phase.string + "`");
				}
				spec.phases.push_back(phase.string);
			}

			out = std::move(spec);
			return true;
		}

		bool parse_document(const std::string& json_text, std::vector<InteractionSpec>& out,
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
				return refuse(error, "interaction document is not an object");
			}

			const JsonValue* interactions = doc.find("interactions");
			if (!interactions || interactions->kind != JsonValue::Kind::Array
				|| interactions->array.empty())
			{
				return refuse(error, "`interactions` must be a non-empty array");
			}
			for (const JsonValue& entry : interactions->array)
			{
				InteractionSpec spec;
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

	bool InteractionLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<InteractionSpec> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (InteractionSpec& spec : parsed)
		{
			if (find(spec.id))
			{
				return refuse(error, "duplicate interaction id `" + spec.id + "`");
			}
			interactions_.push_back(std::move(spec));
		}
		return true;
	}

	int InteractionLibrary::load_directory(const std::string& directory,
		std::vector<std::string>* errors)
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

			std::vector<InteractionSpec> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			for (InteractionSpec& spec : parsed)
			{
				if (find(spec.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate interaction id `"
							+ spec.id + "`");
					}
					continue;
				}
				interactions_.push_back(std::move(spec));
				++loaded;
			}
		}
		return loaded;
	}

	const InteractionSpec* InteractionLibrary::find(const std::string& id) const
	{
		for (const InteractionSpec& spec : interactions_)
		{
			if (spec.id == id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> InteractionLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(interactions_.size());
		for (const InteractionSpec& spec : interactions_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	// --- rules -------------------------------------------------------------------------

	bool usable_in(const InteractionSpec& spec, const std::string& phase)
	{
		return std::find(spec.phases.begin(), spec.phases.end(), phase) != spec.phases.end();
	}

	bool is_weapon(const InteractionSpec& spec)
	{
		const bool role = std::find(spec.kinds.begin(), spec.kinds.end(), "weapon") != spec.kinds.end()
			|| std::find(spec.kinds.begin(), spec.kinds.end(), "throwable") != spec.kinds.end();
		return role && spec.damage > 0.0;
	}

	bool is_cover(const InteractionSpec& spec)
	{
		return std::find(spec.kinds.begin(), spec.kinds.end(), "cover") != spec.kinds.end();
	}

	bool is_extinguisher(const InteractionSpec& spec)
	{
		return std::find(spec.kinds.begin(), spec.kinds.end(), "extinguisher") != spec.kinds.end();
	}

	HeldObject pick_up(const InteractionSpec& spec)
	{
		HeldObject held;
		held.id = spec.id;
		held.sturdiness_left = spec.sturdiness;
		return held;
	}

	bool use_once(HeldObject& held)
	{
		if (held.sturdiness_left <= 0)
		{
			return false;
		}
		--held.sturdiness_left;
		return held.sturdiness_left > 0;
	}

	const InteractionSpec* best_weapon(const InteractionLibrary& library,
		const std::vector<std::string>& held_ids, const std::string& phase)
	{
		const InteractionSpec* best = nullptr;
		for (const std::string& id : held_ids)
		{
			const InteractionSpec* spec = library.find(id);
			if (!spec || !usable_in(*spec, phase) || !is_weapon(*spec))
			{
				continue;
			}
			if (!best || spec->damage > best->damage)
			{
				best = spec;
			}
		}
		return best;
	}

} // namespace lc
