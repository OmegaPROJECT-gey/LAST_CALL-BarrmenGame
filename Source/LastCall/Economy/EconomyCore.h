// LAST CALL - economy core (ROADMAP A3; GAME-DESIGN.md section 2).
//
// The engine-independent half of the morning purchasing phase: loads
// Data/Economy/suppliers.json (tiers, price multipliers, daily stock
// limits, night risk, wares) with the shared JSON reader, quotes unit
// prices as base price x supplier multiplier x deterministic daily drift,
// and keeps a morning ledger that refuses wares a supplier does not sell,
// stock beyond the supplier's limit and money the wallet does not have.
// The morning opens with the day's standing charges (stand + wages,
// GAME-DESIGN 5.4): the wallet may go red, which is the loan.
//
// Ice, upgrades and loan terms stay future rows; this is the buying loop
// the morning screen needs to be a real decision (GAME-DESIGN 2).
//
// Engine-independent C++17: the UE side owns the screen; the rules live
// here. License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lc {

	struct SupplierSpec
	{
		std::string id;
		std::string name_en;
		std::string name_ru;
		std::string tier;        // cheap | standard | premium
		double price_mult = 1.0; // cheap booze is cheaper, premium costs more
		int stock_limit = 0;     // units one supplier sells per morning
		double night_risk = 0.0; // how much this tier worsens the night crowd
		std::vector<std::string> sells;
		int index = 0;           // load order; feeds the daily price drift

		bool sells_liquid(const std::string& liquid_id) const;
	};

	/** Every supplier the player can buy from, from Data/Economy/suppliers.json. */
	class SupplierLibrary
	{
	public:
		bool load(const std::string& json_text, std::string* error = nullptr);

		const SupplierSpec* find(const std::string& supplier_id) const;
		std::vector<std::string> ids() const; // load order
		const std::vector<SupplierSpec>& suppliers() const { return suppliers_; }

	private:
		std::vector<SupplierSpec> suppliers_;
	};

	// Tuning mirrors Data/Balance/core.json ("economy"); the tests assert
	// the two stay in sync.
	struct PriceTuning
	{
		double drift_amplitude = 0.15; // prices wander within +/- this fraction
	};

	// The day's standing charges (GAME-DESIGN 5.4: stock, wages,
	// repairs, upgrades, bribes are the money sinks). Mirrors
	// Data/Balance/core.json ("economy"); the tests assert the two stay
	// in sync.
	struct DayCostTuning
	{
		double stand_cost = 200.0; // daily_stand_cost: the pitch, per day
		double wages = 350.0;      // bar_staff_wages: the staff, per day
	};

	/**
	 * Deterministic daily drift in [1 - a, 1 + a]: the same (day, supplier)
	 * always quotes the same factor, different days and suppliers diverge.
	 */
	double price_drift(int day, int supplier_index, const PriceTuning& tuning);

	/** Unit price of one bottle: base price x supplier multiplier x drift. */
	double unit_price(double base_price, const SupplierSpec& supplier, int day,
		const PriceTuning& tuning);

	/**
	 * One morning's shopping: wallet plus per-supplier stock limits.
	 * purchase() refuses wares outside the supplier's list, stock beyond
	 * the daily limit and totals the wallet cannot cover - `error` says why.
	 */
	class MorningLedger
	{
	public:
		MorningLedger(double money, const PriceTuning& tuning);

		/**
		 * Open the morning: the standing charges (stand + wages, GDD
		 * 5.4) land before any shopping. The wallet may open in the
		 * red - a run in the red is the loan the design allows - and
		 * every later purchase then refuses what the red wallet
		 * cannot cover. The charges are not shopping: spent() stays
		 * stock-only.
		 */
		void open_day(const DayCostTuning& costs);

		bool purchase(const SupplierSpec& supplier, const std::string& liquid_id,
			double base_price, int units, int day, std::string* error = nullptr);

		double money() const { return money_; }
		double spent() const { return spent_; }
		int stock_used(const std::string& supplier_id) const;

	private:
		double money_;
		double spent_ = 0.0;
		PriceTuning tuning_;
		std::vector<std::pair<std::string, int>> stock_; // supplier id -> units bought
	};

} // namespace lc
