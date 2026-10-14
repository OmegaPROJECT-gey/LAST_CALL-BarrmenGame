// LAST CALL - night mode core (ROADMAP B1).
//
// After closing, the bar is a level: which of the night's activities the
// player may walk into is a data table (Data/Night/activities.json)
// gated by the story flags the days have recorded (GAME-DESIGN 4:
// orders to go, debt runs, investigations - available depending on
// story flags). And the night's hard rule: death is consequences, not a
// reload prompt - heat, a hit to standing and hospital days you sit out
// (balance section "night"), then the story resumes on a later day.
//
// Engine-independent C++17; the UE side owns the input set, cover and
// the level itself. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	class ConsequenceLog; // Core/ConsequenceLogCore.h

	struct NightActivity
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::string required_flag; // "" = always open, else a recorded story flag
	};

	/** Night activities from Data/Night/activities.json. */
	class NightLibrary
	{
	public:
		/**
		 * Parse the document. Strict: lowercase ids, bilingual names,
		 * flag ids (or the empty string); duplicate ids are refused.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors = nullptr);

		const NightActivity* find(const std::string& id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<NightActivity>& activities() const { return activities_; }

	private:
		std::vector<NightActivity> activities_;
	};

	/** Is this activity open for a player with this consequence history? */
	bool available(const NightActivity& activity, const ConsequenceLog& log);

	/** Every activity open right now, in table order. */
	std::vector<std::string> available_activities(const NightLibrary& library,
		const ConsequenceLog& log);

	// Tuning mirrors the "night" section of Data/Balance/core.json.
	struct NightTuning
	{
		double death_heat = 3.0;          // heat the city remembers
		double death_standing = -2.0;     // every faction cools off
		int death_hospital_days = 2;      // days the story sits out
	};

	struct DeathConsequences
	{
		double heat = 0.0;
		double standing = 0.0;
		int hospital_days = 0;
	};

	/** What dying costs (GAME-DESIGN 4: consequences, not a reload). */
	DeathConsequences death_consequences(const NightTuning& tuning);

	/** The day the story resumes on after hospital days are sat out. */
	int resume_day(int died_on_day, const NightTuning& tuning);

} // namespace lc
