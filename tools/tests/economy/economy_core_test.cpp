// LAST CALL - economy core unit tests.
//
// Runs without Unreal: scripts/test-economy.sh compiles this against the
// economy core, the liquid library, the JSON reader and the test harness.
// Drives the real Data/Economy/suppliers.json + Data/Liquids/liquids.json.

#include "Bar/LiquidCore.h"
#include "Economy/EconomyCore.h"

#include "test_support.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::MorningLedger;
	using lc::PriceTuning;
	using lc::SupplierLibrary;
	using lc::SupplierSpec;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		SupplierLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

	bool upgrade_refuses(const std::string& text)
	{
		lc::UpgradeLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

	std::string supplier(const std::string& id, const std::string& tier,
		double price_mult, int stock_limit, double night_risk,
		const std::string& sells)
	{
		return "{\"suppliers\":{\"" + id + "\":{"
			"\"display_name\":{\"en\":\"E\",\"ru\":\"Р\"},"
			"\"tier\":\"" + tier + "\","
			"\"price_mult\":" + std::to_string(price_mult) + ","
			"\"stock_limit\":" + std::to_string(stock_limit) + ","
			"\"night_risk\":" + std::to_string(night_risk) + ","
			"\"sells\":" + sells + "}}}";
	}

} // namespace

int main()
{
	PriceTuning tuning;
	tuning.drift_amplitude = lc_test::balance_number("price_drift_amplitude");
	expect(tuning.drift_amplitude > 0.0, "drift amplitude is tuned in core.json");

	// --- the real file on disk -----------------------------------------------------
	SupplierLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Economy/suppliers.json"), &error),
		"Data/Economy/suppliers.json loads");
	expect(library.ids().size() == 3, "three suppliers on the morning list");

	const SupplierSpec* pete = library.find("moonshine_pete");
	expect(pete != nullptr, "Moonshine Pete is listed");
	if (pete)
	{
		expect(pete->tier == "cheap" && std::fabs(pete->price_mult - 0.75) < 1e-9,
			"cheap tier, three quarters of the price");
		expect(pete->stock_limit == 40 && std::fabs(pete->night_risk - 0.35) < 1e-9,
			"40 units a morning, 0.35 of night risk");
		expect(pete->sells_liquid("whiskey_rye") && !pete->sells_liquid("orange_oil"),
			"he sells whiskey, not orange oil");
		expect(pete->name_ru == "\u041b\u0443\u043d\u043d\u044b\u0439 \u041f\u0438\u0442",
			"bilingual name reads");
	}
	expect(library.find("no_such_supplier") == nullptr, "unknown ids read null");

	// --- every ware exists in the Liquids library --------------------------------------
	{
		lc::LiquidLibrary liquids;
		expect(liquids.load(read_repo_file("Data/Liquids/liquids.json"), &error),
			"the liquid catalogue loads");
		bool all_stocked = true;
		for (const SupplierSpec& spec : library.suppliers())
		{
			for (const std::string& ware : spec.sells)
			{
				if (!liquids.find(ware))
				{
					all_stocked = false;
				}
			}
		}
		expect(all_stocked, "every supplier's wares exist in Data/Liquids");
	}

	// --- price drift --------------------------------------------------------------------
	{
		const double amplitude = tuning.drift_amplitude;
		for (int day = 0; day < 7; ++day)
		{
			for (int index = 0; index < 3; ++index)
			{
				const double drift = lc::price_drift(day, index, tuning);
				if (drift < 1.0 - amplitude - 1e-9 || drift > 1.0 + amplitude + 1e-9)
				{
					expect(false, "drift stays inside the amplitude band");
					break;
				}
			}
		}
		expect(lc::price_drift(3, 1, tuning) == lc::price_drift(3, 1, tuning),
			"the drift is deterministic for a given day and supplier");

		double min_seen = 2.0;
		double max_seen = 0.0;
		for (int day = 0; day < 7; ++day)
		{
			const double drift = lc::price_drift(day, 0, tuning);
			min_seen = std::min(min_seen, drift);
			max_seen = std::max(max_seen, drift);
		}
		expect(max_seen > min_seen, "prices actually drift over a week");

		bool supplier_diverges = false;
		for (int day = 0; day < 7; ++day)
		{
			if (lc::price_drift(day, 0, tuning) != lc::price_drift(day, 1, tuning))
			{
				supplier_diverges = true;
			}
		}
		expect(supplier_diverges, "different suppliers drift differently");

		PriceTuning frozen;
		frozen.drift_amplitude = 0.0;
		expect(lc::price_drift(5, 2, frozen) == 1.0, "zero amplitude means flat prices");
	}

	// --- unit prices ---------------------------------------------------------------------
	{
		const SupplierSpec* metro = library.find("metro_wholesale");
		const SupplierSpec* vieux = library.find("vieux_cru");
		const double base = 18.0; // whiskey_rye
		const double metro_price = lc::unit_price(base, *metro, 2, tuning);
		const double vieux_price = lc::unit_price(base, *vieux, 2, tuning);
		expect(metro_price >= base * (1.0 - tuning.drift_amplitude)
			&& metro_price <= base * (1.0 + tuning.drift_amplitude),
			"standard tier quotes base price within the drift band");
		// premium: 18 x 1.6 = 28.8 before drift
		expect(vieux_price >= 28.8 * (1.0 - tuning.drift_amplitude)
			&& vieux_price <= 28.8 * (1.0 + tuning.drift_amplitude),
			"premium costs 1.6x before drift");
	}

	// --- the morning ledger ---------------------------------------------------------------
	{
		const SupplierSpec* pete_spec = library.find("moonshine_pete");
		const SupplierSpec* metro_spec = library.find("metro_wholesale");
		MorningLedger ledger(1000.0, tuning);

		expect(ledger.purchase(*pete_spec, "whiskey_rye", 18.0, 10, 1, &error),
			"ten bottles from Pete go through");
		expect(ledger.money() < 1000.0 && ledger.spent() > 0.0
			&& std::fabs(ledger.money() + ledger.spent() - 1000.0) < 1e-9,
			"the wallet and the ledger agree");
		expect(ledger.stock_used("moonshine_pete") == 10, "stock tracked per supplier");
		expect(ledger.stock_used("metro_wholesale") == 0, "other suppliers untouched");

		expect(!ledger.purchase(*pete_spec, "orange_oil", 9.0, 1, 1, &error)
			&& error.find("does not sell") != std::string::npos,
			"wares outside the list are refused");
		expect(!ledger.purchase(*pete_spec, "whiskey_rye", 18.0, 0, 1, &error)
			&& error.find("positive") != std::string::npos,
			"zero units are refused");
		expect(!ledger.purchase(*pete_spec, "whiskey_rye", 18.0, 31, 1, &error)
			&& error.find("stock limit") != std::string::npos,
			"beyond Pete's 40 units the sale is refused");
		expect(ledger.stock_used("moonshine_pete") == 10, "refused sales change nothing");

		// A wallet that cannot cover the total.
		MorningLedger poor(1.0, tuning);
		expect(!poor.purchase(*metro_spec, "whiskey_rye", 18.0, 1, 1, &error)
			&& error.find("wallet") != std::string::npos,
			"an empty wallet is refused with the shortfall named");
	}

	// --- the day's standing charges (GDD 5.4: wages are a sink) -------------
	{
		const lc::DayCostTuning costs;
		expect_near(costs.stand_cost, lc_test::balance_number("daily_stand_cost"), 1e-9,
			"stand cost synced with core.json");
		expect_near(costs.wages, lc_test::balance_number("bar_staff_wages"), 1e-9,
			"wages synced with core.json");

		// 2000 (the starting treasury) - 200 - 350 = 1450 to shop with.
		MorningLedger ledger(2000.0, tuning);
		ledger.open_day(costs);
		expect_near(ledger.money(), 1450.0, 1e-9,
			"the charges land when the morning opens");
		expect_near(ledger.spent(), 0.0, 1e-9,
			"charges are not shopping - spent() still counts purchases only");
		const SupplierSpec* metro_spec = library.find("metro_wholesale");
		expect(ledger.purchase(*metro_spec, "whiskey_rye", 18.0, 1, 1, &error),
			"shopping against the charged wallet goes through");
		expect(ledger.money() < 1450.0, "and the purchase spends what is left");

		// A day that opens in the red: the loan the design allows.
		MorningLedger red(300.0, tuning);
		red.open_day(costs);
		expect_near(red.money(), -250.0, 1e-9,
			"the wallet may open in the red");
		expect(!red.purchase(*metro_spec, "whiskey_rye", 18.0, 1, 1, &error)
			&& error.find("wallet") != std::string::npos,
			"and nothing is sold to a red wallet");
		expect_near(red.money(), -250.0, 1e-9, "the refused sale changed nothing");
	}

	// --- upgrades (GDD 2: named in the design, bought once) -------------------
	{
		lc::UpgradeLibrary upgrades;
		error.clear();
		expect(upgrades.load(read_repo_file("Data/Upgrades/upgrades.json"), &error)
			&& error.empty(),
			"the real upgrade file loads");
		expect(upgrades.upgrades().size() == 5,
			"the five upgrades the design names are all there");
		expect(upgrades.find("tap_repair") != nullptr, "GDD names `tap_repair`");
		expect(upgrades.find("mixer") != nullptr, "GDD names `mixer`");
		expect(upgrades.find("vip_zone") != nullptr, "GDD names `vip_zone`");
		expect(upgrades.find("lighting") != nullptr, "GDD names `lighting`");
		expect(upgrades.find("jukebox") != nullptr, "GDD names `jukebox`");
		expect(upgrades.find("nope") == nullptr, "an unknown id finds nothing");
		for (const lc::UpgradeSpec& spec : upgrades.upgrades())
		{
			expect(spec.cost >= 1 && !spec.name_en.empty() && !spec.name_ru.empty(),
				"every row has a whole cost and both languages");
		}

		const lc::UpgradeSpec* tap = upgrades.find("tap_repair");
		const double starting = lc_test::balance_number("starting_money");
		expect(tap && tap->cost <= starting,
			"the first upgrade is affordable on the starting treasury");

		// A funded purchase.
		MorningLedger ledger(starting, tuning);
		expect(ledger.buy_upgrade(*tap, {}, &error),
			"a funded upgrade is bought");
		expect_near(ledger.money(), starting - tap->cost, 1e-9,
			"the cost leaves the wallet");
		expect_near(ledger.spent(), tap->cost, 1e-9,
			"and joins what the morning spent");

		// Bought once: the owned set (the save's) refuses the second one.
		expect(!ledger.buy_upgrade(*tap, { tap->id }, &error)
			&& error.find("already bought") != std::string::npos,
			"an owned upgrade is refused");

		// A wallet the standing charges opened in the red.
		MorningLedger red(100.0, tuning);
		red.open_day(lc::DayCostTuning{});
		expect(!red.buy_upgrade(*tap, {}, &error)
			&& error.find("wallet") != std::string::npos,
			"a red wallet buys nothing extra");
		expect_near(red.money(), 100.0 - 550.0, 1e-9,
			"the refused upgrade changed nothing");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing suppliers refused");
	expect(refuses("{\"suppliers\":{}}"), "empty suppliers refused");
	expect(refuses(supplier("x", "luxury", 1.0, 10, 0.0, "[\"a\"]")),
		"unknown tier refused");
	expect(refuses(supplier("x", "cheap", 0.0, 10, 0.0, "[\"a\"]")),
		"zero price multiplier refused");
	expect(refuses(supplier("x", "cheap", 1.0, -1, 0.0, "[\"a\"]")),
		"negative stock limit refused");
	expect(refuses(supplier("x", "cheap", 1.0, 10, 1.5, "[\"a\"]")),
		"night risk above 1 refused");

	// --- upgrade data refusals ---------------------------------------------------------------
	expect(upgrade_refuses("{}"), "upgrades: a missing `upgrades` object refused");
	expect(upgrade_refuses("{\"upgrades\":{}}"), "upgrades: an empty set refused");
	expect(upgrade_refuses("garbage"), "upgrades: garbage refused");
	expect(upgrade_refuses(
		"{\"upgrades\":{\"x\":{\"name\":{\"en\":\"X\",\"ru\":\"Х\"},\"cost\":0}}}"),
		"upgrades: a zero cost refused");
	expect(upgrade_refuses(
		"{\"upgrades\":{\"x\":{\"name\":{\"en\":\"X\"},\"cost\":10}}}"),
		"upgrades: a name without both languages refused");
	expect(upgrade_refuses(
		"{\"upgrades\":{\"x\":{\"name\":{\"en\":\"X\",\"ru\":\"Х\"},\"cost\":\"lots\"}}}"),
		"upgrades: a non-integer cost refused");
	expect(refuses(supplier("x", "cheap", 1.0, 10, 0.0, "[]")),
		"empty wares refused");
	expect(refuses(supplier("x", "cheap", 1.0, 10, 0.0, "[\"a\",\"a\"]")),
		"duplicate wares refused");
	expect(refuses("{\"suppliers\":{\"x\":{\"display_name\":{\"en\":\"E\"},"
		"\"tier\":\"cheap\",\"price_mult\":1.0,\"stock_limit\":10,"
		"\"night_risk\":0.0,\"sells\":[\"a\"]}}}"), "missing ru name refused");
	expect(refuses("not json"), "garbage refused");

	return lc_test::report("economy");
}
