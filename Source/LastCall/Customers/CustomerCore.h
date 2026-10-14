// LAST CALL - customer persona core (ROADMAP A4: "customers as people").
//
// The engine-independent half of regulars: loads Data/Customers/*.json
// (persona, taste, schedule, faction, arc) with the shared JSON reader,
// answers whether the player's Psychology skills reveal the body-language
// tell (GAME-DESIGN.md 3.2), and scores the hostility of a room - the
// rules behind ULCThreatSensor, banding the score calm/elevated/danger
// so the noir music and lighting can react.
//
// Engine-independent C++17: the UE side spawns actors and reads
// animation; the reading rules live here. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct CustomerPersona
	{
		std::string tell; // thirsty | lonely | hostile | nervous | fake
		double mood_baseline = 0.5;
		double patience_baseline = 0.5;
		bool will_tell_story = false;
	};

	struct CustomerRecord
	{
		std::string id;
		std::string archetype; // "regular_major" | ... (free string, as in the schema)
		std::string name_en;
		std::string name_ru;
		std::string seat;

		// "schedule": days 1..7, phases they come in, arrival hour hint.
		bool has_schedule = false;
		std::vector<int> days;
		std::vector<std::string> phases;
		int arrival_hour = 18;

		// "taste"
		std::string order; // recipe id they ask for
		std::string glass;
		std::vector<std::string> acceptable_substitutes;

		CustomerPersona persona;
		std::string faction; // syndicate | ring | bureau | station | civilian
		std::string arc;
	};

	/** Every regular the bar knows, from the files in Data/Customers. */
	class CustomerLibrary
	{
	public:
		/**
		 * Parse one customer document. Strict: id and faction as in the
		 * schema, bilingual name, a taste (order + glass), a persona with
		 * a known tell and 0..1 baselines, and - when present - a sane
		 * schedule (days 1..7, known phases, hour 0..23).
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors);

		const CustomerRecord* find(const std::string& customer_id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<CustomerRecord>& customers() const { return customers_; }

	private:
		std::vector<CustomerRecord> customers_;
	};

	// --- reading the room (GAME-DESIGN 3.2) --------------------------------

	// The SkillSummary of Core/SkillsCore (composed owned skill effects).
	struct TellReadiness
	{
		bool reveal_tells = false; // psychology_1: reads the true tell
		bool reveal_lies = false;  // psychology_2: sees through a performed one
	};

	/**
	 * Does the player see which tell this customer carries? Psychology 1
	 * reveals honest tells; a `fake` customer performs, so only Psychology 2
	 * (reveal_lies) exposes them - without the skills the player reads
	 * animation only.
	 */
	bool tell_revealed(const CustomerPersona& persona, const TellReadiness& readiness);

	// --- threat sensor (the engine half lives in ULCThreatSensor) ---------

	// Tuning mirrors the "threat" section of Data/Balance/core.json.
	struct ThreatTuning
	{
		double hostile_tell_weight = 0.45;  // one hostile patron
		double low_mood_weight = 0.2;       // a patron at mood 0 counts this much
		double low_mood_threshold = 0.3;    // below this mood patrons drag the room down
		double band_elevated = 0.35;        // >= : music sits a little lighter
		double band_danger = 0.7;           // >= : the room can tip into a fight
	};

	// What the sensor knows about one patron right now (not what the
	// player managed to read - the room is hostile either way).
	struct PatronRead
	{
		std::string tell;
		double mood = 0.5;
	};

	enum class ThreatBand
	{
		Calm,
		Elevated,
		Danger
	};

	// Additive, capped at 1: hostile tells and low moods stack until the
	// room is a hair from a fight. An empty room scores 0.
	double threat_score(const std::vector<PatronRead>& room, const ThreatTuning& tuning);

	ThreatBand threat_band(double score, const ThreatTuning& tuning);

	// --- arrival ---------------------------------------------------------

	// Does this customer show up on this day of this phase?
	bool arrives_on(const CustomerRecord& customer, int day, const std::string& phase);

} // namespace lc
