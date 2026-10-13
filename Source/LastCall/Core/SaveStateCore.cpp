// LAST CALL - save state core implementation. See header for the contract.

#include "Core/SaveStateCore.h"

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
			int day = 1;
			int phase = 0;
			int money = 0;
			double heat = 0.0;
		};

		bool parse_int(const std::string& text, int& out)
		{
			try
			{
				std::size_t consumed = 0;
				out = std::stoi(text, &consumed);
				return consumed == text.size();
			}
			catch (...)
			{
				return false;
			}
		}

		bool parse_double(const std::string& text, double& out)
		{
			try
			{
				std::size_t consumed = 0;
				out = std::stod(text, &consumed);
				return consumed == text.size();
			}
			catch (...)
			{
				return false;
			}
		}

	} // namespace

	std::string serialize(const SaveState& state)
	{
		std::ostringstream out;
		out << kHeaderPrefix << kSaveVersion << '\n';
		out << "day=" << state.day << '\n';
		out << "phase=" << static_cast<int>(state.phase) << '\n';
		out << "money=" << state.money << '\n';
		out << "heat=" << state.heat;
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
			else
			{
				return false; // unknown field: same version must mean the same shape
			}
		}

		if (!fields.has_day || !fields.has_phase || !fields.has_money || !fields.has_heat)
		{
			return false; // missing fields
		}
		if (fields.day < 1 || fields.phase < 0 || fields.phase > 3 || fields.heat < 0.0)
		{
			return false; // out of range
		}

		SaveState result;
		result.version = version;
		result.day = fields.day;
		result.phase = static_cast<Phase>(fields.phase);
		result.money = fields.money;
		result.heat = fields.heat;
		out = result;
		return true;
	}

} // namespace lc
