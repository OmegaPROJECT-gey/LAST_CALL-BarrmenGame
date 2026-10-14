// LAST CALL - customer persona core implementation. See header for the contract.

#include "Customers/CustomerCore.h"

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

		bool is_known_tell(const std::string& tell)
		{
			return tell == "thirsty" || tell == "lonely" || tell == "hostile"
				|| tell == "nervous" || tell == "fake";
		}

		bool is_known_faction(const std::string& faction)
		{
			return faction == "syndicate" || faction == "ring" || faction == "bureau"
				|| faction == "station" || faction == "civilian";
		}

		bool is_known_phase(const std::string& phase)
		{
			return phase == "Morning" || phase == "Shift" || phase == "Night"
				|| phase == "Summary";
		}

		bool parse_customer(const std::string& json_text, CustomerRecord& out, std::string* error)
		{
			JsonValue doc;
			std::string parse_error;
			if (!json_parse(json_text, doc, &parse_error))
			{
				return refuse(error, "bad JSON: " + parse_error);
			}
			if (doc.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "customer document is not an object");
			}

			CustomerRecord record;
			if (!doc.get_string("id", record.id) || !is_lower_id(record.id))
			{
				return refuse(error, "customer needs a lowercase `id`");
			}
			const std::string where = "customer `" + record.id + "`: ";

			const JsonValue* name = doc.find("name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", record.name_en) || record.name_en.empty()
				|| !name->get_string("ru", record.name_ru) || record.name_ru.empty())
			{
				return refuse(error, where + "`name` needs en and ru");
			}

			doc.get_string("archetype", record.archetype); // optional
			doc.get_string("seat", record.seat);           // optional
			doc.get_string("arc", record.arc);             // optional

			if (!doc.get_string("faction", record.faction) || !is_known_faction(record.faction))
			{
				return refuse(error, where + "`faction` must be syndicate, ring, bureau, station or civilian");
			}

			// --- taste -----------------------------------------------------------------
			const JsonValue* taste = doc.find("taste");
			if (!taste || taste->kind != JsonValue::Kind::Object
				|| !taste->get_string("order", record.order) || record.order.empty()
				|| !taste->get_string("glass", record.glass) || record.glass.empty())
			{
				return refuse(error, where + "`taste` needs `order` and `glass`");
			}
			if (const JsonValue* substitutes = taste->find("acceptable_substitutes"))
			{
				if (substitutes->kind != JsonValue::Kind::Array)
				{
					return refuse(error, where + "`acceptable_substitutes` must be an array");
				}
				for (const JsonValue& substitute : substitutes->array)
				{
					if (substitute.kind != JsonValue::Kind::String || substitute.string.empty())
					{
						return refuse(error, where + "`acceptable_substitutes` entries must be ids");
					}
					record.acceptable_substitutes.push_back(substitute.string);
				}
			}

			// --- persona ---------------------------------------------------------------
			const JsonValue* persona = doc.find("persona");
			if (!persona || persona->kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "`persona` is required");
			}
			if (!persona->get_string("tell", record.persona.tell)
				|| !is_known_tell(record.persona.tell))
			{
				return refuse(error, where + "`tell` must be thirsty, lonely, hostile, nervous or fake");
			}
			if (!persona->get_number("mood_baseline", record.persona.mood_baseline)
				|| record.persona.mood_baseline < 0.0 || record.persona.mood_baseline > 1.0
				|| !persona->get_number("patience_baseline", record.persona.patience_baseline)
				|| record.persona.patience_baseline < 0.0 || record.persona.patience_baseline > 1.0)
			{
				return refuse(error, where + "persona baselines must be within 0..1");
			}
			persona->get_bool("will_tell_story", record.persona.will_tell_story); // optional

			// --- schedule (optional) ----------------------------------------------------
			if (const JsonValue* schedule = doc.find("schedule"))
			{
				if (schedule->kind != JsonValue::Kind::Object)
				{
					return refuse(error, where + "`schedule` must be an object");
				}
				record.has_schedule = true;
				if (const JsonValue* days = schedule->find("days"))
				{
					if (days->kind != JsonValue::Kind::Array)
					{
						return refuse(error, where + "`days` must be an array");
					}
					for (const JsonValue& day : days->array)
					{
						const double value = day.number;
						if (day.kind != JsonValue::Kind::Number
							|| value < 1.0 || value > 7.0 || std::floor(value) != value)
						{
							return refuse(error, where + "`days` entries must be whole days 1..7");
						}
						record.days.push_back(static_cast<int>(value));
					}
				}
				if (const JsonValue* phases = schedule->find("phases"))
				{
					if (phases->kind != JsonValue::Kind::Array)
					{
						return refuse(error, where + "`phases` must be an array");
					}
					for (const JsonValue& phase : phases->array)
					{
						if (phase.kind != JsonValue::Kind::String || !is_known_phase(phase.string))
						{
							return refuse(error, where + "`phases` entries must be known phases");
						}
						record.phases.push_back(phase.string);
					}
				}
				if (schedule->find("arrival_hour") != nullptr
					&& (!schedule->get_int("arrival_hour", record.arrival_hour)
						|| record.arrival_hour < 0 || record.arrival_hour > 23))
				{
					return refuse(error, where + "`arrival_hour` must be within 0..23");
				}
			}

			out = std::move(record);
			return true;
		}

	} // namespace

	// --- library -----------------------------------------------------------------------

	bool CustomerLibrary::load(const std::string& json_text, std::string* error)
	{
		CustomerRecord record;
		if (!parse_customer(json_text, record, error))
		{
			return false;
		}
		if (find(record.id))
		{
			return refuse(error, "duplicate customer id `" + record.id + "`");
		}
		customers_.push_back(std::move(record));
		return true;
	}

	int CustomerLibrary::load_directory(const std::string& directory, std::vector<std::string>* errors)
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

			CustomerRecord record;
			std::string error;
			if (!parse_customer(buffer.str(), record, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			if (find(record.id))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": duplicate customer id `"
						+ record.id + "`");
				}
				continue;
			}
			customers_.push_back(std::move(record));
			++loaded;
		}
		return loaded;
	}

	const CustomerRecord* CustomerLibrary::find(const std::string& customer_id) const
	{
		for (const CustomerRecord& record : customers_)
		{
			if (record.id == customer_id)
			{
				return &record;
			}
		}
		return nullptr;
	}

	std::vector<std::string> CustomerLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(customers_.size());
		for (const CustomerRecord& record : customers_)
		{
			out.push_back(record.id);
		}
		return out;
	}

	// --- reading the room ---------------------------------------------------------------

	bool tell_revealed(const CustomerPersona& persona, const TellReadiness& readiness)
	{
		if (persona.tell == "fake")
		{
			// A performed tell only shows for what it is to someone who
			// catches lies (psychology_2).
			return readiness.reveal_lies;
		}
		return readiness.reveal_tells;
	}

	// --- threat -------------------------------------------------------------------------

	double threat_score(const std::vector<PatronRead>& room, const ThreatTuning& tuning)
	{
		double score = 0.0;
		for (const PatronRead& patron : room)
		{
			if (patron.tell == "hostile")
			{
				score += tuning.hostile_tell_weight;
			}
			if (tuning.low_mood_threshold > 0.0 && patron.mood < tuning.low_mood_threshold)
			{
				score += tuning.low_mood_weight
					* (tuning.low_mood_threshold - patron.mood) / tuning.low_mood_threshold;
			}
		}
		return score > 1.0 ? 1.0 : score;
	}

	ThreatBand threat_band(double score, const ThreatTuning& tuning)
	{
		if (score >= tuning.band_danger)
		{
			return ThreatBand::Danger;
		}
		if (score >= tuning.band_elevated)
		{
			return ThreatBand::Elevated;
		}
		return ThreatBand::Calm;
	}

	// --- arrival ------------------------------------------------------------------------

	bool arrives_on(const CustomerRecord& customer, int day, const std::string& phase)
	{
		const auto day_seen = std::find(customer.days.begin(), customer.days.end(), day);
		if (day_seen == customer.days.end())
		{
			return false;
		}
		const auto phase_seen = std::find(customer.phases.begin(), customer.phases.end(), phase);
		return phase_seen != customer.phases.end();
	}

} // namespace lc
