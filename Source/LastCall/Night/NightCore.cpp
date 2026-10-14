// LAST CALL - night mode core implementation. See header for the contract.

#include "Night/NightCore.h"

#include "Core/ConsequenceLogCore.h"
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

		bool parse_entry(const JsonValue& entry, NightActivity& out, std::string* error)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "activity is not an object");
			}

			NightActivity activity;
			if (!entry.get_string("id", activity.id) || !is_lower_id(activity.id))
			{
				return refuse(error, "activity needs a lowercase `id`");
			}
			const std::string where = "activity `" + activity.id + "`: ";

			const JsonValue* name = entry.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", activity.name_en) || activity.name_en.empty()
				|| !name->get_string("ru", activity.name_ru) || activity.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}

			if (!entry.get_string("required_flag", activity.required_flag)
				|| (!activity.required_flag.empty() && !is_lower_id(activity.required_flag)))
			{
				return refuse(error, where + "`required_flag` must be a flag id or \"\"");
			}

			out = std::move(activity);
			return true;
		}

		bool parse_document(const std::string& json_text, std::vector<NightActivity>& out,
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
				return refuse(error, "night document is not an object");
			}

			const JsonValue* activities = doc.find("activities");
			if (!activities || activities->kind != JsonValue::Kind::Array
				|| activities->array.empty())
			{
				return refuse(error, "`activities` must be a non-empty array");
			}
			for (const JsonValue& entry : activities->array)
			{
				NightActivity activity;
				if (!parse_entry(entry, activity, error))
				{
					return false;
				}
				out.push_back(std::move(activity));
			}
			return true;
		}

	} // namespace

	// --- library -----------------------------------------------------------------------

	bool NightLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<NightActivity> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (NightActivity& activity : parsed)
		{
			if (find(activity.id))
			{
				return refuse(error, "duplicate activity id `" + activity.id + "`");
			}
			activities_.push_back(std::move(activity));
		}
		return true;
	}

	int NightLibrary::load_directory(const std::string& directory,
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

			std::vector<NightActivity> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			for (NightActivity& activity : parsed)
			{
				if (find(activity.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate activity id `"
							+ activity.id + "`");
					}
					continue;
				}
				activities_.push_back(std::move(activity));
				++loaded;
			}
		}
		return loaded;
	}

	const NightActivity* NightLibrary::find(const std::string& id) const
	{
		for (const NightActivity& activity : activities_)
		{
			if (activity.id == id)
			{
				return &activity;
			}
		}
		return nullptr;
	}

	std::vector<std::string> NightLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(activities_.size());
		for (const NightActivity& activity : activities_)
		{
			out.push_back(activity.id);
		}
		return out;
	}

	// --- gating ------------------------------------------------------------------------

	bool available(const NightActivity& activity, const ConsequenceLog& log)
	{
		return activity.required_flag.empty() || log.has(activity.required_flag);
	}

	std::vector<std::string> available_activities(const NightLibrary& library,
		const ConsequenceLog& log)
	{
		std::vector<std::string> out;
		for (const NightActivity& activity : library.activities())
		{
			if (available(activity, log))
			{
				out.push_back(activity.id);
			}
		}
		return out;
	}

	// --- death is consequences -----------------------------------------------------------

	DeathConsequences death_consequences(const NightTuning& tuning)
	{
		DeathConsequences consequences;
		consequences.heat = tuning.death_heat;
		consequences.standing = tuning.death_standing;
		consequences.hospital_days = tuning.death_hospital_days;
		return consequences;
	}

	int resume_day(int died_on_day, const NightTuning& tuning)
	{
		const int died = died_on_day < 1 ? 1 : died_on_day;
		return died + (tuning.death_hospital_days < 0 ? 0 : tuning.death_hospital_days);
	}

} // namespace lc
