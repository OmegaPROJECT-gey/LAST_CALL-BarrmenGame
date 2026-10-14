// LAST CALL - investigations core unit tests.
//
// Runs without Unreal: scripts/test-intel.sh compiles this against the
// intel core, the JSON reader and the harness.
// Drives the real file Data/Intel/intel.json.

#include "Night/IntelCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;

	using lc::IntelLedger;
	using lc::IntelLibrary;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		IntelLibrary scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real file on disk ------------------------------------------------------
	IntelLibrary library;
	std::string error;
	expect(library.load(read_repo_file("Data/Intel/intel.json"), &error),
		"Data/Intel/intel.json loads");
	expect(library.ids().size() == 3, "three pieces of intel on the table");

	const lc::IntelSpec* docks = library.find("docks_meet");
	expect(docks != nullptr, "the docks meeting is for sale");
	if (docks)
	{
		expect(docks->cost == 3 && docks->grants_flag == "intel_docks_meet",
			"docks: three tokens for its flag");
		expect(!docks->name_en.empty() && !docks->name_ru.empty()
			&& !docks->note_en.empty() && !docks->note_ru.empty(),
			"intel is bilingual, name and note");
	}
	expect(library.find("no_such_intel") == nullptr, "unknown intel reads null");

	// --- the ledger ----------------------------------------------------------------------
	{
		IntelLedger ledger(library);
		expect(ledger.tokens() == 0, "an empty dossier holds no tokens");

		expect(!ledger.learn("docks_meet"), "no tokens, no intel");
		expect(!ledger.can_afford("docks_meet"), "and it reads unaffordable");

		ledger.add_tokens(3);
		expect(ledger.tokens() == 3, "three tokens collected");
		expect(ledger.can_afford("docks_meet"), "now the docks are within reach");

		expect(ledger.learn("docks_meet"), "bought");
		expect(ledger.tokens() == 0, "tokens spent");
		expect(ledger.knows("docks_meet"), "and remembered");
		expect(ledger.granted_flag("docks_meet") == "intel_docks_meet",
			"the flag it grants is readable");
		expect(!ledger.can_afford("docks_meet"), "already known reads unaffordable");
		expect(!ledger.learn("docks_meet"), "and cannot be bought twice");
		expect(ledger.tokens() == 0, "no double spending");

		expect(!ledger.knows("shipment"), "the shipment route is still unknown");
		expect(ledger.granted_flag("shipment").empty(),
			"and grants nothing until learned");

		ledger.add_tokens(9);
		expect(ledger.learn("shipment") && ledger.learn("bought_cop"),
			"two more buys with a full purse");
		expect(ledger.tokens() == 9 - 5 - 4, "the purse counted down");

		ledger.add_tokens(-5), expect(ledger.tokens() == 0,
			"negative collections are ignored");
		expect(!ledger.learn("no_such_intel"), "unknown intel cannot be bought");
	}

	// --- strict refusals ------------------------------------------------------------------
	expect(refuses("{\"intel\":[]}"), "empty intel refused");
	expect(refuses("{\"intel\":[{\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"missing id refused");
	expect(refuses("{\"intel\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":-1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"negative cost refused");
	expect(refuses("{\"intel\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\"},"
		"\"cost\":1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"missing ru name refused");
	expect(refuses("{\"intel\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\"}}]}"),
		"half-translated note refused");
	expect(refuses("{\"intel\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":1,\"grants_flag\":\"Bad-Flag\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"malformed flag id refused");
	expect(refuses("{\"intel\":[{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}},"
		"{\"id\":\"x\",\"display_name\":{\"en\":\"E\",\"ru\":\"R\"},"
		"\"cost\":1,\"grants_flag\":\"f\",\"note\":{\"en\":\"n\",\"ru\":\"n\"}}]}"),
		"duplicate id refused");
	expect(refuses("not json"), "garbage refused");

	{
		IntelLibrary scratch;
		std::vector<std::string> dir_errors;
		expect(scratch.load_directory("/no/such/directory", &dir_errors) == 0
			&& !dir_errors.empty(),
			"a missing directory reports an error");
	}

	return lc_test::report("investigations");
}
