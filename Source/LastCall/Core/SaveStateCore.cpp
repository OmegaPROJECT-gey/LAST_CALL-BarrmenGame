// LAST CALL - save state core implementation. See header for the contract.

#include "Core/SaveStateCore.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace lc {

	namespace {

		constexpr const char* kHeaderPrefix = "lc-save ";

		struct ParsedFields
		{
			bool has_day = false;
			bool has_phase = false;
			bool has_money = false;
			bool has_heat = false;
			bool has_venue_tier = false;
			bool has_standing = false;
			int day = 1;
			int phase = 0;
			int money = 0;
			double heat = 0.0;
			int venue_tier = kMinVenueTier;
			std::vector<std::pair<std::string, double>> standing;
		};

		// strtol/strtod instead of std::stoi/std::stod: the Game target
		// compiles with exceptions disabled (-fno-exceptions), and a save
		// file must refuse garbage without throwing.
		bool parse_int(const std::string& text, int& out)
		{
			if (text.empty())
			{
				return false;
			}
			errno = 0;
			char* end = nullptr;
			const long value = std::strtol(text.c_str(), &end, 10);
			if (end != text.c_str() + text.size() || errno == ERANGE
				|| value < std::numeric_limits<int>::min()
				|| value > std::numeric_limits<int>::max())
			{
				return false;
			}
			out = static_cast<int>(value);
			return true;
		}

		bool parse_double(const std::string& text, double& out)
		{
			if (text.empty())
			{
				return false;
			}
			errno = 0;
			char* end = nullptr;
			const double value = std::strtod(text.c_str(), &end);
			if (end != text.c_str() + text.size() || errno == ERANGE
				|| !std::isfinite(value))
			{
				return false;
			}
			out = value;
			return true;
		}

		// Faction ids in the ledger keep the same shape every data file
		// uses: lowercase letters, digits, underscores.
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

		// `standing=` -> pairs; "" is an empty ledger. Refuses a
		// malformed id, a non-numeric value and a duplicate faction.
		bool parse_standing(const std::string& text,
			std::vector<std::pair<std::string, double>>& out)
		{
			if (text.empty())
			{
				return true;
			}
			std::vector<std::pair<std::string, double>> rows;
			std::size_t at = 0;
			while (at <= text.size())
			{
				const std::size_t comma = text.find(',', at);
				const std::string token =
					text.substr(at, comma == std::string::npos ? std::string::npos : comma - at);
				const std::size_t colon = token.find(':');
				if (colon == std::string::npos || colon == 0
					|| colon + 1 >= token.size())
				{
					return false;
				}
				std::pair<std::string, double> row;
				row.first = token.substr(0, colon);
				if (!is_id(row.first) || !parse_double(token.substr(colon + 1), row.second))
				{
					return false;
				}
				for (const std::pair<std::string, double>& previous : rows)
				{
					if (previous.first == row.first)
					{
						return false;
					}
				}
				rows.push_back(std::move(row));
				if (comma == std::string::npos)
				{
					break;
				}
				at = comma + 1;
			}
			out = std::move(rows);
			return true;
		}

	} // namespace

	std::string serialize(const SaveState& state)
	{
		// Sorted by faction id: same ledger -> same bytes.
		std::vector<std::pair<std::string, double>> standing = state.standing;
		std::sort(standing.begin(), standing.end(),
			[](const std::pair<std::string, double>& a,
				const std::pair<std::string, double>& b) { return a.first < b.first; });

		std::ostringstream out;
		out << kHeaderPrefix << kSaveVersion << '\n';
		out << "day=" << state.day << '\n';
		out << "phase=" << static_cast<int>(state.phase) << '\n';
		out << "money=" << state.money << '\n';
		out << "heat=" << state.heat << '\n';
		out << "venue_tier=" << state.venue_tier << '\n';
		out << "standing=";
		for (std::size_t index = 0; index < standing.size(); ++index)
		{
			if (index > 0)
			{
				out << ',';
			}
			out << standing[index].first << ':' << standing[index].second;
		}
		return out.str();
	}

	bool deserialize(const std::string& text, SaveState& out)
	{
		std::istringstream in(text);
		std::string line;

		// Header: exact marker plus the version number.
		if (!std::getline(in, line))
		{
			return false;
		}
		if (line.rfind(kHeaderPrefix, 0) != 0)
		{
			return false;
		}
		int version = 0;
		if (!parse_int(line.substr(std::string(kHeaderPrefix).size()), version) || version != kSaveVersion)
		{
			return false; // unknown or corrupt version: refuse, never migrate silently
		}

		ParsedFields fields;
		while (std::getline(in, line))
		{
			if (line.empty())
			{
				continue; // tolerate a trailing newline
			}
			const std::size_t eq = line.find('=');
			if (eq == std::string::npos)
			{
				return false;
			}
			const std::string key = line.substr(0, eq);
			const std::string value = line.substr(eq + 1);

			if (key == "day")
			{
				if (fields.has_day || !parse_int(value, fields.day)) { return false; }
				fields.has_day = true;
			}
			else if (key == "phase")
			{
				if (fields.has_phase || !parse_int(value, fields.phase)) { return false; }
				fields.has_phase = true;
			}
			else if (key == "money")
			{
				if (fields.has_money || !parse_int(value, fields.money)) { return false; }
				fields.has_money = true;
			}
			else if (key == "heat")
			{
				if (fields.has_heat || !parse_double(value, fields.heat)) { return false; }
				fields.has_heat = true;
			}
			else if (key == "venue_tier")
			{
				if (fields.has_venue_tier || !parse_int(value, fields.venue_tier)) { return false; }
				fields.has_venue_tier = true;
			}
			else if (key == "standing")
			{
				if (fields.has_standing
					|| !parse_standing(value, fields.standing)) { return false; }
				fields.has_standing = true;
			}
			else
			{
				return false; // unknown field: same version must mean the same shape
			}
		}

		if (!fields.has_day || !fields.has_phase || !fields.has_money
			|| !fields.has_heat || !fields.has_venue_tier || !fields.has_standing)
		{
			return false; // missing fields
		}
		if (fields.day < 1 || fields.phase < 0 || fields.phase > 3 || fields.heat < 0.0
			|| fields.venue_tier < kMinVenueTier || fields.venue_tier > kMaxVenueTier)
		{
			return false; // out of range
		}

		SaveState result;
		result.version = version;
		result.day = fields.day;
		result.phase = static_cast<Phase>(fields.phase);
		result.money = fields.money;
		result.heat = fields.heat;
		result.venue_tier = fields.venue_tier;
		result.standing = std::move(fields.standing);
		out = result;
		return true;
	}

} // namespace lc
