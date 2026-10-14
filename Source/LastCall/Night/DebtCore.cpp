// LAST CALL - debt runs core implementation. See header for the contract.

#include "Night/DebtCore.h"

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

		bool parse_entry(const JsonValue& entry, DebtorRecord& out, std::string* error)
		{
			if (entry.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "debtor is not an object");
			}

			DebtorRecord record;
			if (!entry.get_string("id", record.id) || !is_lower_id(record.id))
			{
				return refuse(error, "debtor needs a lowercase `id`");
			}
			const std::string where = "debtor `" + record.id + "`: ";

			const JsonValue* name = entry.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", record.name_en) || record.name_en.empty()
				|| !name->get_string("ru", record.name_ru) || record.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}

			if (!entry.get_string("block", record.block) || !is_lower_id(record.block))
			{
				return refuse(error, where + "`block` must be an id");
			}
			if (!entry.get_string("faction", record.faction) || !is_lower_id(record.faction))
			{
				return refuse(error, where + "`faction` must be an id");
			}
			if (!entry.get_int("owed", record.owed) || record.owed < 0)
			{
				return refuse(error, where + "`owed` must be 0 or more");
			}
			if (!entry.get_number("talk_resist", record.talk_resist)
				|| record.talk_resist < 0.0 || record.talk_resist > 1.0
				|| !entry.get_number("intimidate_resist", record.intimidate_resist)
				|| record.intimidate_resist < 0.0 || record.intimidate_resist > 1.0)
			{
				return refuse(error, where + "resistances must be within 0..1");
			}
			entry.get_string("collateral", record.collateral); // optional, "" = none

			out = std::move(record);
			return true;
		}

		bool parse_document(const std::string& json_text, std::vector<DebtorRecord>& out,
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
				return refuse(error, "debtor document is not an object");
			}

			const JsonValue* debtors = doc.find("debtors");
			if (!debtors || debtors->kind != JsonValue::Kind::Array || debtors->array.empty())
			{
				return refuse(error, "`debtors` must be a non-empty array");
			}
			for (const JsonValue& entry : debtors->array)
			{
				DebtorRecord record;
				if (!parse_entry(entry, record, error))
				{
					return false;
				}
				out.push_back(std::move(record));
			}
			return true;
		}

	} // namespace

	// --- library -----------------------------------------------------------------------

	bool DebtorLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<DebtorRecord> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (DebtorRecord& record : parsed)
		{
			if (find(record.id))
			{
				return refuse(error, "duplicate debtor id `" + record.id + "`");
			}
			debtors_.push_back(std::move(record));
		}
		return true;
	}

	int DebtorLibrary::load_directory(const std::string& directory,
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

			std::vector<DebtorRecord> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			for (DebtorRecord& record : parsed)
			{
				if (find(record.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate debtor id `"
							+ record.id + "`");
					}
					continue;
				}
				debtors_.push_back(std::move(record));
				++loaded;
			}
		}
		return loaded;
	}

	const DebtorRecord* DebtorLibrary::find(const std::string& id) const
	{
		for (const DebtorRecord& record : debtors_)
		{
			if (record.id == id)
			{
				return &record;
			}
		}
		return nullptr;
	}

	std::vector<std::string> DebtorLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(debtors_.size());
		for (const DebtorRecord& record : debtors_)
		{
			out.push_back(record.id);
		}
		return out;
	}

	// --- encounters ----------------------------------------------------------------------

	DebtOutcome resolve_debt(const DebtorRecord& debtor, const DebtAttempt& attempt,
		const DebtTuning& tuning)
	{
		DebtOutcome outcome;

		const auto collected_of = [&debtor](const double ratio)
		{
			return static_cast<int>(std::floor(debtor.owed * ratio));
		};

		if (attempt.approach == "talk")
		{
			if (attempt.score >= debtor.talk_resist)
			{
				outcome.resolution = DebtResolution::Paid;
				outcome.collected = debtor.owed;
			}
			else if (attempt.score >= debtor.talk_resist * 0.5)
			{
				outcome.resolution = DebtResolution::Partial;
				outcome.collected = collected_of(tuning.partial_ratio);
			}
			else
			{
				outcome.resolution = DebtResolution::Escaped;
			}
		}
		else if (attempt.approach == "intimidate")
		{
			if (attempt.score >= debtor.intimidate_resist)
			{
				outcome.resolution = DebtResolution::Paid;
				outcome.collected = debtor.owed;
			}
			else if (attempt.score >= debtor.intimidate_resist * 0.5)
			{
				outcome.resolution = DebtResolution::Partial;
				outcome.collected = collected_of(tuning.partial_ratio);
			}
			else
			{
				// They stood up to the siphon. You take what is loose
				// and the whole block hears about it.
				outcome.resolution = DebtResolution::Fight;
				outcome.collected = collected_of(tuning.fight_loot_ratio);
				outcome.heat_delta += tuning.fight_heat;
			}
		}
		else if (attempt.approach == "take")
		{
			// Seizing something is always partial - and it gets reported.
			outcome.resolution = DebtResolution::Partial;
			outcome.collected = collected_of(tuning.partial_ratio);
			outcome.collateral = debtor.collateral;
			outcome.heat_delta += tuning.take_heat;
		}
		else
		{
			// Unknown approach: you stood there and they left.
			outcome.resolution = DebtResolution::Escaped;
		}

		if (attempt.violent)
		{
			outcome.heat_delta += tuning.violence_heat;
			outcome.standing_delta += tuning.violence_standing;
		}
		return outcome;
	}

} // namespace lc
