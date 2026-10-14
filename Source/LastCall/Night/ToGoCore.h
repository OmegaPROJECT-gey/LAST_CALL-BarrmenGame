// LAST CALL - orders to go core (ROADMAP B2).
//
// The armored window: the city queues at the shutter, orders must be
// assembled fast while pressure builds outside (GAME-DESIGN 4.1). This
// is the session rules without the engine: a queue of orders, each with
// its own window; speed is money (a fast hand earns a tip bonus), lateness
// is cheap (and angers the queue), a customer who walks raises the
// pressure, and when pressure tops out the window slams shut - the night
// of to-go orders is over, win or lose.
//
// Engine-independent C++17; the UE side runs the assembly and feeds
// seconds spent. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct ToGoOrder
	{
		std::string recipe_id;      // what they ordered
		std::string glass;          // how it must leave
		double window_seconds = 25.0; // how long this customer waits
	};

	// Tuning mirrors the "to_go" section of Data/Balance/core.json.
	struct ToGoTuning
	{
		double base_window_seconds = 25.0; // window for orders without one
		double tip_bonus_fast = 0.3;       // served inside half the window
		double tip_penalty_late = 0.5;     // multiplier drops by this when late
		double pressure_per_late = 1.0;    // a late serve angers the queue
		double walkout_pressure = 2.5;     // a customer who leaves
		double pressure_limit = 10.0;      // at the limit the window closes
	};

	/** One rush at the armored window. */
	class ToGoSession
	{
	public:
		explicit ToGoSession(const ToGoTuning& tuning);

		/** Queue an order up. Refuses malformed orders (nothing enqueued). */
		bool enqueue(const ToGoOrder& order);

		int waiting() const { return static_cast<int>(queue_.size()); }

		/**
		 * An order is handed out after `seconds_spent` assembling.
		 * Returns the tip multiplier for it: a bonus inside half the
		 * window, par inside it, penalized and pressing the queue when
		 * late. An empty queue is a no-op at par.
		 */
		double serve(double seconds_spent);

		/** A customer gave up and walked into the night. */
		void miss();

		double pressure() const { return pressure_; }

		/** Has the pressure topped out and shut the window? */
		bool over() const { return pressure_ >= tuning_.pressure_limit; }

		const ToGoTuning& tuning() const { return tuning_; }

	private:
		ToGoTuning tuning_;
		std::vector<ToGoOrder> queue_;
		double pressure_ = 0.0;
	};

} // namespace lc
