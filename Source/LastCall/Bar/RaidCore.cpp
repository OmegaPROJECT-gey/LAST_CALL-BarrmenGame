// LAST CALL - raid director core implementation. See header for the contract.

#include "Bar/RaidCore.h"

#include <algorithm>

namespace lc {

	RaidResult resolve_raid(const RaidState& state, double distraction_score,
		const RaidTuning& tuning)
	{
		RaidResult result;

		const int total = static_cast<int>(state.evidence.size());
		result.hidden_by_partner = std::min(state.partner_capacity, total);
		if (result.hidden_by_partner < 0)
		{
			result.hidden_by_partner = 0;
		}

		if (distraction_score >= tuning.distraction_threshold)
		{
			// They stand at the bar listening while the partner works.
			result.distraction_worked = true;
			result.found = 0;
			return result;
		}

		// They push through: everything not hidden is on the table.
		result.distraction_worked = false;
		result.found = total - result.hidden_by_partner;
		if (result.found > 0)
		{
			result.heat_delta = tuning.fail_heat
				+ tuning.per_item_heat * static_cast<double>(result.found);
			result.fine = tuning.fail_fine * static_cast<double>(result.found);
		}
		return result;
	}

} // namespace lc
