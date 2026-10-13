// LAST CALL - consequence log core implementation. See header for the contract.

#include "Core/ConsequenceLogCore.h"

#include <algorithm>

namespace lc {

	bool ConsequenceLog::record(const std::string& flag, int day, const std::string& phase)
	{
		if (flag.empty() || day < 1)
		{
			return false; // a consequence needs a name and a real day
		}
		const bool already = std::any_of(log_.begin(), log_.end(),
			 [&](const ConsequenceEntry& entry)
			{
				return entry.flag == flag && entry.day == day;
			});
		if (already)
		{
			return false;
		}
		log_.push_back({ flag, day, phase });
		return true;
	}

	bool ConsequenceLog::has(const std::string& flag) const
	{
		return std::any_of(log_.begin(), log_.end(),
			[&](const ConsequenceEntry& entry) { return entry.flag == flag; });
	}

	int ConsequenceLog::count(const std::string& flag) const
	{
		return static_cast<int>(std::count_if(log_.begin(), log_.end(),
			[&](const ConsequenceEntry& entry) { return entry.flag == flag; }));
	}

} // namespace lc
