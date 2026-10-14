// LAST CALL - orders to go core unit tests.
//
// Runs without Unreal: scripts/test-togo.sh compiles this against the
// to-go core and the harness. Pure rules: no data files beyond the
// balance tuning sync.

#include "Night/ToGoCore.h"

#include "test_support.h"

#include <cmath>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::ToGoOrder;
	using lc::ToGoSession;
	using lc::ToGoTuning;

	ToGoOrder order(const std::string& recipe, double window)
	{
		ToGoOrder out;
		out.recipe_id = recipe;
		out.glass = "rocks";
		out.window_seconds = window;
		return out;
	}

} // namespace

int main()
{
	const ToGoTuning tuning;

	// Code and Data/Balance/core.json must agree on the tuning.
	expect_near(tuning.base_window_seconds,
		lc_test::balance_number("base_window_seconds"), 1e-9,
		"base_window_seconds synced with core.json");
	expect_near(tuning.tip_bonus_fast,
		lc_test::balance_number("tip_bonus_fast"), 1e-9,
		"tip_bonus_fast synced with core.json");
	expect_near(tuning.tip_penalty_late,
		lc_test::balance_number("tip_penalty_late"), 1e-9,
		"tip_penalty_late synced with core.json");
	expect_near(tuning.pressure_per_late,
		lc_test::balance_number("pressure_per_late"), 1e-9,
		"pressure_per_late synced with core.json");
	expect_near(tuning.walkout_pressure,
		lc_test::balance_number("walkout_pressure"), 1e-9,
		"walkout_pressure synced with core.json");
	expect_near(tuning.pressure_limit,
		lc_test::balance_number("pressure_limit"), 1e-9,
		"pressure_limit synced with core.json");

	// --- the rush ------------------------------------------------------------------------
	{
		ToGoSession session(tuning);
		expect(session.waiting() == 0 && !session.over(), "an empty shutter is calm");
		expect_near(session.pressure(), 0.0, 1e-9, "no pressure yet");

		expect(session.enqueue(order("signature_noir", 25.0)), "first order queued");
		expect(session.enqueue(order("whiskey_neat", 25.0)), "second order queued");
		expect(session.enqueue(order("stout", 15.0)), "third, impatient, queued");
		expect(session.waiting() == 3, "three waiting outside");

		// Dead on the first: bonus.
		const double fast = session.serve(5.0);
		expect_near(fast, 1.0 + tuning.tip_bonus_fast, 1e-9, "fast hand earns the bonus");
		expect(session.waiting() == 2, "one served");
		expect_near(session.pressure(), 0.0, 1e-9, "speed keeps the street calm");

		// Inside the window, not fast: par.
		const double par = session.serve(20.0);
		expect_near(par, 1.0, 1e-9, "inside the window is par");

		// Late on the short window: penalized and pressing.
		const double late = session.serve(30.0); // 15s window, 30s spent
		expect_near(late, 1.0 - tuning.tip_penalty_late, 1e-9, "late pays the penalty");
		expect_near(session.pressure(), tuning.pressure_per_late, 1e-9,
			"a late serve angers the queue");
		expect(session.waiting() == 0, "the queue is empty");
	}

	// --- walkouts and the window slamming shut ----------------------------------------------
	{
		ToGoSession session(tuning);
		for (int i = 0; i < 4; ++i)
		{
			expect(session.enqueue(order("stout", 25.0)), "queue a customer");
			session.miss();
		}
		expect_near(session.pressure(), 4.0 * tuning.walkout_pressure, 1e-9,
			"four walkouts stack pressure");
		expect(session.over(), "and that shuts the window");

		// A miss with an empty queue still presses: the street outside heard.
		session.miss();
		expect_near(session.pressure(), 5.0 * tuning.walkout_pressure, 1e-9,
			"walkouts press even with no one queued");
	}

	// --- guards ------------------------------------------------------------------------------
	{
		ToGoSession session(tuning);
		expect_near(session.serve(5.0), 1.0, 1e-9, "serving an empty queue is par");
		expect(session.waiting() == 0, "and stays empty");

		expect(!session.enqueue(ToGoOrder()), "an empty order is refused");
		ToGoOrder no_glass = order("signature_noir", 25.0);
		no_glass.glass.clear();
		expect(!session.enqueue(no_glass), "an order without a glass is refused");
		ToGoOrder no_window = order("signature_noir", 0.0);
		expect(!session.enqueue(no_window), "an order with no window is refused");
		expect(session.waiting() == 0, "nothing malformed got queued");
	}

	return lc_test::report("orders to go");
}
