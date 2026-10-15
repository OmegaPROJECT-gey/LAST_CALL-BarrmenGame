// LAST CALL - day schedule core unit tests.
//
// Runs without Unreal: scripts/test-day-schedule.sh compiles this against
// Source/LastCall/Core/DayScheduleCore.cpp with g++.

#include "Core/DayScheduleCore.h"
#include "Night/NightCore.h"

#include "test_support.h"

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::DayClock;
	using lc::DayScheduleSpec;
	using lc::Phase;
	using lc::PhaseTransition;

} // namespace

int main()
{
	const DayScheduleSpec spec;

	// --- initial state --------------------------------------------------------
	{
		DayClock clock;
		expect(clock.phase == Phase::Morning && clock.day == 1,
			"the day starts at Morning of day 1");
		expect(!clock.save_requested, "nothing to save before the first boundary");
	}

	// --- legal and illegal transitions ----------------------------------------
	{
		DayClock clock;
		const PhaseTransition jump = lc::request_phase(clock, Phase::Night);
		expect(jump.changed && jump.from == Phase::Morning && jump.to == Phase::Night,
			"skipping forward within the day is allowed");
		expect(clock.phase == Phase::Night && clock.day == 1, "clock moved to Night");
		expect(clock.save_requested, "every boundary requests a save");

		clock.save_requested = false;
		const PhaseTransition back = lc::request_phase(clock, Phase::Morning);
		expect(!back.changed && clock.phase == Phase::Night,
			"backward jumps are refused - the day only moves forward");
		expect(!clock.save_requested, "a refused jump is not a boundary");

		const PhaseTransition same = lc::request_phase(clock, Phase::Night);
		expect(!same.changed, "requesting the current phase is a no-op");
	}
	{
		DayClock clock;
		clock.phase = Phase::Summary;
		const PhaseTransition wrap = lc::request_phase(clock, Phase::Morning);
		expect(!wrap.changed && clock.day == 1,
			"Summary -> Morning only happens through advance (a new day)");
	}

	// --- timed phases auto-advance ---------------------------------------------
	{
		DayClock clock;
		expect(lc::tick(clock, 100.0, spec).empty(), "morning is not over after 100 s");
		const std::vector<PhaseTransition> flow = lc::tick(clock, 250.0, spec);
		expect(flow.size() == 1 && flow[0].to == Phase::Shift,
			"morning auto-ends at its deadline");
		expect(clock.phase == Phase::Shift, "clock moved on to the shift");
		expect(!flow[0].new_day, "the shift is still the same day");
		expect_near(clock.phase_time_s, 0.0, 1e-9, "phase timer resets at a boundary");
		expect_near(clock.day_time_s, 350.0, 1e-9, "day timer keeps running across phases");
		expect(clock.save_requested, "the boundary requested a save");
	}
	{
		DayClock clock;
		lc::request_phase(clock, Phase::Shift);
		clock.save_requested = false;
		const std::vector<PhaseTransition> flow = lc::tick(clock, 10000.0, spec);
		expect(flow.empty() && clock.phase == Phase::Shift,
			"the shift is untimed - it ends by event, not by clock");
		expect_near(clock.phase_time_s, 10000.0, 1e-9, "untimed phases still accumulate time");
	}

	// --- the full loop ----------------------------------------------------------
	{
		DayClock clock;
		lc::advance_phase(clock); // Morning -> Shift
		lc::advance_phase(clock); // Shift -> Night
		lc::advance_phase(clock); // Night -> Summary
		expect(clock.phase == Phase::Summary && clock.day == 1,
			"four advances land on Summary of the same day");
		clock.save_requested = false;
		const PhaseTransition wrap = lc::advance_phase(clock);
		expect(wrap.changed && wrap.new_day, "Summary wraps into a new day");
		expect(clock.phase == Phase::Morning && clock.day == 2,
			"day counter advanced");
		expect_near(clock.day_time_s, 0.0, 1e-9, "day timer resets with the new day");
		expect(clock.save_requested, "the day boundary requested a save");
	}

	// --- hospital days (B1: death is consequences, you sit them out) -------------
	{
		// The night rule as data: die on day 5, sit the stay out, and
		// the clock lands on the resume day the night core promises.
		const int hospital =
			static_cast<int>(lc_test::balance_number("death_hospital_days"));
		expect(hospital > 0, "the balance carries a hospital stay");

		DayClock clock;
		clock.phase = Phase::Summary;
		clock.day = 5;
		lc::admit_to_hospital(clock, hospital);
		expect(clock.hospital_days_left == hospital, "the clock holds the stay");

		const PhaseTransition wrap = lc::advance_phase(clock);
		expect(wrap.changed && wrap.new_day, "the day boundary still fires");
		expect(clock.phase == Phase::Morning, "and lands in the morning");
		expect(clock.day == 5 + hospital,
			"the stay is sat out in one jump, not day by day");
		expect(clock.hospital_days_left == 0, "the stay is spent");

		lc::NightTuning tuning;
		tuning.death_hospital_days = hospital;
		expect(lc::resume_day(5, tuning) == clock.day,
			"exactly the resume day NightCore promises for day 5");
	}
	{
		// Nothing to sit out: the day advances by one, as ever.
		DayClock clock;
		clock.phase = Phase::Summary;
		clock.day = 5;
		lc::admit_to_hospital(clock, 0);
		lc::advance_phase(clock);
		expect(clock.day == 6, "a stay of zero advances one day");
	}
	{
		// A negative count is a refusal, not a time machine.
		DayClock clock;
		lc::admit_to_hospital(clock, -3);
		expect(clock.hospital_days_left == 0, "negative days are clamped to none");
	}
	{
		// Only the wrap into a new day spends the stay.
		DayClock clock;
		lc::admit_to_hospital(clock, 2);
		lc::request_phase(clock, Phase::Shift);
		lc::request_phase(clock, Phase::Night);
		expect(clock.hospital_days_left == 2,
			"skipping phases does not spend the stay");
		lc::advance_phase(clock); // Night -> Summary
		expect(clock.hospital_days_left == 2, "Summary does not spend it either");
		lc::advance_phase(clock); // Summary -> Morning of the next day
		expect(clock.day == 3 && clock.hospital_days_left == 0,
			"only the wrap into a new day spends it");
	}
	{
		// A timed Summary auto-advancing through tick carries the stay.
		DayClock clock;
		clock.phase = Phase::Summary;
		clock.day = 5;
		lc::admit_to_hospital(clock, 2);
		const std::vector<PhaseTransition> flow = lc::tick(clock, 30.0, spec);
		expect(flow.size() == 1 && flow[0].new_day && clock.day == 7,
			"the timed wrap sits the stay out the same way");
		expect(clock.phase == Phase::Morning, "and lands in the morning");
	}

	// --- progress ----------------------------------------------------------------
	{
		DayClock clock;
		lc::tick(clock, 150.0, spec);
		expect_near(lc::phase_progress(clock, spec), 0.5, 1e-9,
			"half the morning is half the progress");
		lc::request_phase(clock, Phase::Shift);
		expect_near(lc::phase_progress(clock, spec), 0.0, 1e-9,
			"untimed phases report no progress");
	}

	// --- balance sync (Data/Balance/core.json "phases") ---------------------------
	expect_near(lc_test::balance_number("morning_seconds"),
		spec.morning_seconds, 1e-9, "morning duration in sync with core.json");
	expect_near(lc_test::balance_number("shift_seconds"),
		spec.shift_seconds, 1e-9, "shift duration in sync with core.json");
	expect_near(lc_test::balance_number("night_seconds"),
		spec.night_seconds, 1e-9, "night duration in sync with core.json");
	expect_near(lc_test::balance_number("summary_seconds"),
		spec.summary_seconds, 1e-9, "summary duration in sync with core.json");

	return lc_test::report("day schedule");
}
