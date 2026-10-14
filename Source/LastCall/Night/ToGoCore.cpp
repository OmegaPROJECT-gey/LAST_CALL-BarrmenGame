// LAST CALL - orders to go core implementation. See header for the contract.

#include "Night/ToGoCore.h"

namespace lc {

	ToGoSession::ToGoSession(const ToGoTuning& tuning)
		: tuning_(tuning)
	{
	}

	bool ToGoSession::enqueue(const ToGoOrder& order)
	{
		if (order.recipe_id.empty() || order.glass.empty() || order.window_seconds <= 0.0)
		{
			return false;
		}
		queue_.push_back(order);
		return true;
	}

	double ToGoSession::serve(double seconds_spent)
	{
		if (queue_.empty())
		{
			return 1.0; // nobody waiting: par, no pressure change
		}

		ToGoOrder order = queue_.front();
		queue_.erase(queue_.begin());

		if (order.window_seconds <= 0.0)
		{
			order.window_seconds = tuning_.base_window_seconds;
		}

		if (seconds_spent <= order.window_seconds * 0.5)
		{
			return 1.0 + tuning_.tip_bonus_fast;
		}
		if (seconds_spent <= order.window_seconds)
		{
			return 1.0;
		}

		// Late: the customer pays, and the queue saw it.
		pressure_ += tuning_.pressure_per_late;
		return 1.0 - tuning_.tip_penalty_late;
	}

	void ToGoSession::miss()
	{
		if (!queue_.empty())
		{
			queue_.erase(queue_.begin());
		}
		pressure_ += tuning_.walkout_pressure;
	}

} // namespace lc
