// LAST CALL - balance file core. See header for the contract.

#include "Core/BalanceCore.h"

#include <fstream>
#include <sstream>

#include "Core/JsonCore.h"

namespace
{
	bool refuse(std::string* error, const std::string& reason)
	{
		if (error)
		{
			*error = reason;
		}
		return false;
	}
} // namespace

namespace lc {

	bool load_balance(const std::string& json_text, BalanceData& out, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "balance document is not an object");
		}

		// --- phases: the durations behind the day clock ---
		const JsonValue* phases = doc.find("phases");
		if (!phases || phases->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "missing `phases` section");
		}
		DayScheduleSpec Spec;
		if (!phases->get_number("morning_seconds", Spec.morning_seconds)
			|| !phases->get_number("shift_seconds", Spec.shift_seconds)
			|| !phases->get_number("night_seconds", Spec.night_seconds)
			|| !phases->get_number("summary_seconds", Spec.summary_seconds))
		{
			return refuse(error,
				"`phases` needs morning/shift/night/summary_seconds as numbers");
		}
		if (Spec.morning_seconds < 0.0 || Spec.shift_seconds < 0.0
			|| Spec.night_seconds < 0.0 || Spec.summary_seconds < 0.0)
		{
			return refuse(error, "`phases` durations must be >= 0 (0 = untimed)");
		}

		// --- pour: the tilt -> flow curve behind the solver ---
		const JsonValue* pour = doc.find("pour");
		if (!pour || pour->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "missing `pour` section");
		}
		const JsonValue* curve = pour->find("flow_curve");
		if (!curve || curve->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "missing `pour.flow_curve` section");
		}
		PourTuning Tuning;
		if (!curve->get_number("min_angle_deg", Tuning.min_angle_deg)
			|| !curve->get_number("max_angle_deg", Tuning.max_angle_deg)
			|| !curve->get_number("viscosity_default", Tuning.viscosity_default)
			|| !pour->get_number("flow_ml_per_s", Tuning.base_flow_ml_per_s)
			|| !pour->get_number("overflow_spill_threshold",
				Tuning.overflow_spill_threshold))
		{
			return refuse(error,
				"`pour` needs flow_ml_per_s, overflow_spill_threshold and "
				"flow_curve {min_angle_deg, max_angle_deg, viscosity_default}");
		}
		if (Tuning.min_angle_deg >= Tuning.max_angle_deg)
		{
			return refuse(error, "`pour.flow_curve` min_angle_deg must be < max_angle_deg");
		}
		if (Tuning.viscosity_default <= 0.0 || Tuning.base_flow_ml_per_s <= 0.0
			|| Tuning.overflow_spill_threshold <= 0.0)
		{
			return refuse(error, "`pour` viscosity, flow and spill threshold must be > 0");
		}

		// --- economy: where a run starts ---
		const JsonValue* economy = doc.find("economy");
		if (!economy || economy->kind != JsonValue::Kind::Object)
		{
			return refuse(error, "missing `economy` section");
		}
		int starting_money = 0;
		if (!economy->get_int("starting_money", starting_money))
		{
			return refuse(error, "`economy.starting_money` must be an integer");
		}
		if (starting_money < 0)
		{
			return refuse(error, "`economy.starting_money` must be >= 0");
		}

		out.phases = Spec;
		out.pour = Tuning;
		out.starting_money = starting_money;
		return true;
	}

	bool load_balance_file(const std::string& path, BalanceData& out, std::string* error)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			return refuse(error, "cannot open " + path);
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return load_balance(buffer.str(), out, error);
	}

} // namespace lc
