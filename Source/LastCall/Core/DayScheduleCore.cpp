// LAST CALL - day schedule core implementation. See header for the contract.

#include "Core/DayScheduleCore.h"

namespace lc {

	namespace {

		// The single place the clock mutates on a boundary: this is where the
		// save file gets written in milestone F2 (ARCHITECTURE 9).
		PhaseTransition apply(DayClock& clock, Phase target, bool new_day)
		{
			PhaseTransition transition;
			transition.changed = true;
			transition.new_day = new_day;
			transition.from = clock.phase;
			transition.to = target;

			clock.phase = target;
			clock.phase_time_s = 0.0;
			clock.save_requested = true;
			if (new_day)
			{
				++clock.day;
				clock.day_time_s = 0.0;
			}
			return transition;
		}

	} // namespace

	double duration_for(const DayScheduleSpec& spec, Phase phase)
	{
		switch (phase)
		{
			case Phase::Morning: return spec.morning_seconds;
			case Phase::Shift:   return spec.shift_seconds;
			case Phase::Night:   return spec.night_seconds;
			case Phase::Summary: return spec.summary_seconds;
		}
		return 0.0;
	}

	PhaseTransition request_phase(DayClock& clock, Phase target)
	{
		PhaseTransition refused;
		if (target == clock.phase)
		{
			return refused; // no-op
		}
		if (static_cast<int>(target) <= static_cast<int>(clock.phase))
		{
			return refused; // backward within the day, or Summary -> Morning
		}
		return apply(clock, target, false);
	}

	PhaseTransition advance_phase(DayClock& clock)
	{
		if (clock.phase == Phase::Summary)
		{
			return apply(clock, Phase::Morning, true);
		}
		const Phase next = static_cast<Phase>(static_cast<int>(clock.phase) + 1);
		return apply(clock, next, false);
	}

	std::vector<PhaseTransition> tick(DayClock& clock, double dt_s, const DayScheduleSpec& spec)
	{
		std::vector<PhaseTransition> transitions;
		if (dt_s <= 0.0)
		{
			return transitions;
		}

		clock.phase_time_s += dt_s;
		clock.day_time_s += dt_s;

		const double duration = duration_for(spec, clock.phase);
		if (duration > 0.0 && clock.phase_time_s >= duration)
		{
			transitions.push_back(advance_phase(clock));
		}
		return transitions;
	}

	double phase_progress(const DayClock& clock, const DayScheduleSpec& spec)
	{
		const double duration = duration_for(spec, clock.phase);
		if (duration <= 0.0)
		{
			return 0.0;
		}
		return clock.phase_time_s / duration;
	}

} // namespace lc
