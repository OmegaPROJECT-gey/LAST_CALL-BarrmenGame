// LAST CALL - investigations core implementation. See header for the contract.

#include "Night/IntelCore.h"

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

		bool read_localized(const JsonValue& body, const char* key,
			std::string& en, std::string& ru, const std::string& where, std::string* error)
		{
			const JsonValue* name = body.find(key);
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", en) || en.empty()
				|| !name->get_string("ru", ru) || ru.empty())
			{
				return refuse(error, where + std::string("`") + key + "` needs en and ru");
			}
			return true;
		}

		bool parse_entry(const JsonValue& entry, IntelSpec& out, std::string* error)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "intel is not an object");
			}

			IntelSpec spec;
			if (!entry.get_string("id", spec.id) || !is_lower_id(spec.id))
			{
				return refuse(error, "intel needs a lowercase `id`");
			}
			const std::string where = "intel `" + spec.id + "`: ";

			if (!read_localized(entry, "display_name", spec.name_en, spec.name_ru, where, error)
				|| !read_localized(entry, "note", spec.note_en, spec.note_ru, where, error))
			{
				return false;
			}
			if (!entry.get_int("cost", spec.cost) || spec.cost < 0)
			{
				return refuse(error, where + "`cost` must be 0 or more");
			}
			if (!entry.get_string("grants_flag", spec.grants_flag)
				|| !is_lower_id(spec.grants_flag))
			{
				return refuse(error, where + "`grants_flag` must be a flag id");
			}

			out = std::move(spec);
			return true;
		}

		bool parse_document(const std::string& json_text, std::vector<IntelSpec>& out,
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
				return refuse(error, "intel document is not an object");
			}

			const JsonValue* intel = doc.find("intel");
			if (!intel || intel->kind != JsonValue::Kind::Array || intel->array.empty())
			{
				return refuse(error, "`intel` must be a non-empty array");
			}
			for (const JsonValue& entry : intel->array)
			{
				IntelSpec spec;
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

	bool IntelLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<IntelSpec> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (IntelSpec& spec : parsed)
		{
			if (find(spec.id))
			{
				return refuse(error, "duplicate intel id `" + spec.id + "`");
			}
			intel_.push_back(std::move(spec));
		}
		return true;
	}

	int IntelLibrary::load_directory(const std::string& directory,
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

			std::vector<IntelSpec> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			for (IntelSpec& spec : parsed)
			{
				if (find(spec.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate intel id `"
							+ spec.id + "`");
					}
					continue;
				}
				intel_.push_back(std::move(spec));
				++loaded;
			}
		}
		return loaded;
	}

	const IntelSpec* IntelLibrary::find(const std::string& id) const
	{
		for (const IntelSpec& spec : intel_)
		{
			if (spec.id == id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> IntelLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(intel_.size());
		for (const IntelSpec& spec : intel_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	// --- ledger ------------------------------------------------------------------------

	IntelLedger::IntelLedger(const IntelLibrary& library)
		: library_(&library)
	{
	}

	void IntelLedger::add_tokens(int count)
	{
		if (count > 0)
		{
			tokens_ += count;
		}
	}

	bool IntelLedger::knows(const std::string& intel_id) const
	{
		return std::find(known_.begin(), known_.end(), intel_id) != known_.end();
	}

	bool IntelLedger::can_afford(const std::string& intel_id) const
	{
		const IntelSpec* spec = library_->find(intel_id);
		return spec != nullptr && !knows(intel_id) && tokens_ >= spec->cost;
	}

	bool IntelLedger::learn(const std::string& intel_id)
	{
		if (!can_afford(intel_id))
		{
			return false;
		}
		const IntelSpec* spec = library_->find(intel_id);
		tokens_ -= spec->cost;
		known_.push_back(intel_id);
		return true;
	}

	std::string IntelLedger::granted_flag(const std::string& intel_id) const
	{
		if (!knows(intel_id))
		{
			return "";
		}
		const IntelSpec* spec = library_->find(intel_id);
		return spec ? spec->grants_flag : "";
	}

} // namespace lc
