// LAST CALL - dialogue runner core unit tests.
//
// Runs without Unreal: scripts/test-dialogue.sh compiles this against the
// runner, the JSON reader and the test harness with g++. Drives the real
// file Data/Dialogue/act1_harris_meeting.json.

#include "Dialogue/DialogueRunnerCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::DialogueRunner;
	using lc::DialogueWorld;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		DialogueRunner scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	const std::string real_dialogue = read_repo_file("Data/Dialogue/act1_harris_meeting.json");

	// --- the real file on disk -----------------------------------------------------
	{
		DialogueRunner runner;
		std::string error;
		expect(runner.load(real_dialogue, &error), "act1_harris_meeting.json loads");
		expect(runner.id() == "act1_harris_meeting", "dialogue id");
		expect(runner.interrupt_flag_ignored() == "ignored_harris_meeting"
			&& runner.interrupt_flag_heard() == "heard_harris_meeting",
			"interrupt recording flags read");

		DialogueWorld world;
		expect(runner.start(world), "the dialogue starts");
		expect(runner.current_speaker() == "harris", "first speaker is Harris");
		expect(runner.current_text_key() == "dlg.harris.greet", "greeting text key");
		expect(runner.can_advance() && !runner.at_end(), "greeting is linear and not the end");

		expect(runner.advance(world), "advance to the offer");
		expect(runner.current_text_key() == "dlg.harris.offer_case", "the offer is showing");

		// Three choices; in a neutral world all three are available:
		// bureau standing 0 >= min 0, no psychology skill -> refuse allowed.
		const std::vector<int> choices = runner.available_choices(world);
		expect(choices.size() == 3, "all three choices available in a neutral world");
	}
	// --- accept route: effects apply ------------------------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		expect(runner.choose("accept", world), "accept is taken");
		expect_near(world.standing_of("bureau"), 1.0, 1e-9,
			"accept raised bureau standing by 1");
		expect(world.has_flag("heard_harris_case"), "accept recorded the story flag");
		expect(runner.at_end() && runner.current_text_key() == "dlg.harris.accepted",
			"accept lands on the ending node");
		expect(!runner.can_advance(), "an ending node has no linear advance");
		expect(!runner.choose("accept", world), "choices are dead at the end");

		// The quest effect was routed, not applied to the world by the runner.
		bool quest_started = false;
		for (const lc::AppliedEffect& effect : runner.applied())
		{
			if (effect.type == "quest" && effect.target == "act1_ear_for_bureau"
				&& effect.state == "started")
			{
				quest_started = true;
			}
		}
		expect(quest_started, "the quest effect is in the applied list for the engine");
	}
	// --- refuse route -----------------------------------------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		expect(runner.choose("refuse", world), "refuse is taken");
		expect_near(world.standing_of("bureau"), -1.0, 1e-9, "refuse cost bureau standing");
		expect(runner.at_end() && runner.current_text_key() == "dlg.harris.refused",
			"refuse lands on its ending node");
	}
	// --- pour-first route: conditional node + routed mood --------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		expect(runner.choose("pour_first", world), "pour_first is taken");
		expect(runner.current_text_key() == "dlg.harris.poured",
			"the conditional 'poured' node is entered (its flag was just set)");
		bool mood_found = false;
		for (const lc::AppliedEffect& effect : runner.applied())
		{
			if (effect.type == "mood" && effect.target == "harris")
			{
				mood_found = true;
				expect_near(effect.delta, 0.15, 1e-9, "mood delta is 0.15");
			}
		}
		expect(mood_found, "the mood effect was routed");
		expect(runner.advance(world)
			&& runner.current_text_key() == "dlg.harris.offer_case",
			"poured leads back to the offer");
		expect(runner.advance(world) && runner.at_end(),
			"...and on into the accepted ending");
	}
	// --- condition filtering ------------------------------------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		world.skills.push_back("psychology_1");
		const std::vector<int> filtered = runner.available_choices(world);
		expect(filtered.size() == 2, "owning psychology hides the refuse choice");
		expect(!runner.choose("refuse", world), "a hidden choice cannot be taken");
	}
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		world.set_flag("bureau_hates_you", true);
		world.add_standing("bureau", -5.0);
		const std::vector<int> filtered = runner.available_choices(world);
		expect(filtered.size() == 2, "bureau standing below the minimum hides accept");
		expect(!runner.choose("accept", world), "a standing-gated choice cannot be taken");
	}
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		expect(!runner.choose("no_such_choice", world), "unknown choice ids are refused");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing entry refused");
	expect(refuses("{\"entry\":\"a\"}"), "missing nodes refused");
	expect(refuses("{\"entry\":\"ghost\",\"nodes\":{\"a\":{\"text_key\":\"x\"}}}"),
		"entry pointing nowhere refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\",\"next\":\"ghost\"}}}"),
		"unknown next target refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"choices\":[{\"id\":\"c\",\"text_key\":\"t\",\"next\":\"ghost\"}]}}}"),
		"choice into nowhere refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"speaker\":\"x\"}}}"),
		"missing text_key refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"conditions\":[{\"type\":\"weather\",\"x\":1}]}}}"),
		"unknown condition type refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"effects\":[{\"type\":\"karma\",\"delta\":1}]}}}"),
		"unknown effect type refused");
	expect(refuses("not json"), "garbage refused");

	return lc_test::report("dialogue runner");
}
