// LAST CALL - liquid core (ARCHITECTURE.md, section 4; ROADMAP A1).
//
// The engine-independent half of layered drinks: loads liquid properties
// (viscosity, density, foam, colour) from Data/Liquids/*.json with the
// shared JSON reader, and keeps a layered glass state - strata ordered by
// density (heavy sinks, light floats), same-liquid strata merge, foam head
// grows on pour and dissipates over time, overflow lands in a spill log
// the engine turns into decals and Niagara splashes.
//
// Viscosity feeds the pour solver (`normalized_flow_from_tilt`), so the
// same tilt pours thick liqueur slower than whiskey - the connection is
// asserted end-to-end in tools/tests/liquids.
//
// Engine-independent C++17: the UE side owns rendering; the rules live
// here. License: MIT (LICENSE).

#pragma once

#include <string>
#include <vector>

namespace lc {

	struct LiquidSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		double viscosity = 1.0;    // multiplier on flow (water = 1, syrups > 1)
		double density = 1.0;      // g/ml: decides where the stratum sits
		double foam_fraction = 0.0; // foam volume created per poured volume
		std::string color;         // "#rrggbb"
	};

	/** Every liquid the bar can pour, from Data/Liquids/liquids.json. */
	class LiquidLibrary
	{
	public:
		bool load(const std::string& json_text, std::string* error = nullptr);
		int load_directory(const std::string& directory, std::vector<std::string>* errors);
		bool add(const std::string& json_text, std::vector<std::string>* errors);

		const LiquidSpec* find(const std::string& liquid_id) const;
		std::vector<std::string> ids() const;

	private:
		std::vector<LiquidSpec> liquids_;
	};

	struct LiquidLayer
	{
		std::string liquid_id;
		double density = 1.0;
		double volume_ml = 0.0;
	};

	struct SpillEvent
	{
		double time_s = 0.0;
		double volume_ml = 0.0;
		std::string liquid_id;
	};

	/**
	 * A glass while it holds a layered drink. Strata are kept sorted by
	 * density (heaviest at the bottom); pouring a liquid the glass already
	 * has merges into that stratum instead of stacking twice. Foam is a
	 * single head above the liquid and decays on tick(). Capacity covers
	 * liquid + foam; overflow is recorded as a spill event and refused.
	 */
	class LayeredGlass
	{
	public:
		void configure(double capacity_ml, double foam_decay_per_s);

		/** Take `ml` of `spec`; returns how much actually entered. */
		double pour(const LiquidSpec& spec, double ml, double time_s);

		/** Foam dissipates: foam *= (1 - decay * dt), clamped at zero. */
		void tick(double dt_s);

		double volume_ml() const;  // liquid only
		double free_ml() const;    // capacity - liquid - foam
		double foam_ml() const { return foam_ml_; }
		const std::vector<LiquidLayer>& layers() const { return layers_; } // bottom -> top
		const std::vector<SpillEvent>& spills() const { return spills_; }

	private:
		void clip_foam(); // foam never exceeds what is left of capacity

		double capacity_ml_ = 150.0;
		double foam_decay_per_s_ = 0.12;
		double foam_ml_ = 0.0;
		std::vector<LiquidLayer> layers_;  // sorted by density, descending
		std::vector<SpillEvent> spills_;
	};

} // namespace lc
