// LAST CALL - customer reaction core implementation. See header for the contract.

#include "Customers/CustomerReactionCore.h"

#include <algorithm>

namespace lc {

	namespace {

		double clamp01(double value)
		{
			return std::max(0.0, std::min(1.0, value));
		}

	} // namespace

	void on_spill(CustomerState& customer, double spill_ml, const CustomerTuning& tuning)
	{
		if (customer.walked_out || spill_ml <= 0.0)
		{
			return;
		}

		customer.shock_s = tuning.shock_s;

		if (spill_ml >= tuning.burst_walkout_spill_ml)
		{
			// Pouring a whole glass over the bar is not an accident they
			// wait out - they are gone before the glass settles.
			customer.patience = 0.0;
			customer.walked_out = true;
			return;
		}

		customer.patience -= spill_ml * tuning.spill_patience_per_ml;
		if (customer.patience <= 0.0)
		{
			customer.patience = 0.0;
			customer.walked_out = true;
		}
	}

	double satisfaction_for(ComboQuality quality, const CustomerTuning& tuning)
	{
		switch (quality)
		{
			case ComboQuality::Critical: return tuning.bonus_critical;
			case ComboQuality::Good:     return tuning.refill_correct_order;
			case ComboQuality::Fine:     return 0.0;
			case ComboQuality::Ruined:   return -tuning.ruined_penalty;
		}
		return 0.0;
	}

	void on_serve(CustomerState& customer, ComboQuality quality, const CustomerTuning& tuning)
	{
		if (customer.walked_out)
		{
			return;
		}

		customer.patience = clamp01(customer.patience + satisfaction_for(quality, tuning));
		if (customer.patience <= 0.0)
		{
			customer.patience = 0.0;
			customer.walked_out = true;
		}
	}

	void tick(CustomerState& customer, double dt_s, bool waiting_in_queue,
		const CustomerTuning& tuning)
	{
		if (customer.walked_out || dt_s <= 0.0)
		{
			return;
		}

		const double drain = waiting_in_queue ? tuning.drain_waiting_per_s : tuning.drain_idle_per_s;
		customer.patience -= drain * dt_s;
		customer.shock_s = std::max(0.0, customer.shock_s - dt_s);

		if (customer.patience <= 0.0)
		{
			customer.patience = 0.0;
			customer.walked_out = true;
		}
	}

	Reaction reaction_for(const CustomerState& customer, const CustomerTuning& tuning)
	{
		if (customer.walked_out)
		{
			return Reaction::WalkedOut;
		}
		if (customer.shock_s > 0.0)
		{
			return Reaction::Shocked;
		}
		if (customer.patience >= tuning.band_pleased)
		{
			return Reaction::Content;
		}
		if (customer.patience >= tuning.band_annoyed)
		{
			return Reaction::Neutral;
		}
		return Reaction::Annoyed;
	}

} // namespace lc
