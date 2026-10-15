// LAST CALL - skill tree core unit tests.
//
// Runs without Unreal: scripts/test-skills.sh compiles this against the
// core, the JSON reader and the test harness with g++. Drives the real
// file Data/Skills/skills.json and cross-checks the dialogue skill ids.

#include "Core/SkillsCore.h"
#include "Dialogue/DialogueRunnerCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::SkillSummary;
	using lc::SkillTree;

	std::string read_repo_file(const std::string& relative)
	{
		std::ifstream file(lc_test::repo_root() + "/" + relative);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool refuses(const std::string& text)
	{
		SkillTree scratch;
		std::string error;
		return !scratch.load(text, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- the real file on disk -----------------------------------------------------
	SkillTree tree;
	std::string error;
	expect(tree.load(read_repo_file("Data/Skills/skills.json"), &error),
		"Data/Skills/skills.json loads");
	expect(tree.ids().size() == 8, "four branches of two nodes each");

	const lc::SkillNode* flair_2 = tree.find("flair_2");
	expect(flair_2 != nullptr && flair_2->branch == "flair" && flair_2->cost == 250.0,
		"flair_2 lives in the flair branch and costs 250");
	expect(flair_2->requirements.size() == 1 && flair_2->requirements[0] == "flair_1",
		"flair_2 requires flair_1");
	expect(flair_2->has_effect && flair_2->effect.type == "tip_multiplier"
		&& flair_2->effect.mult == 1.15, "flair_2 lifts tips by 15%");
	expect(!tree.find("no_such_skill"), "unknown ids read null");

	// --- what a fresh player can buy ------------------------------------------------
	{
		const std::vector<std::string> owned;
		const std::vector<std::string> buyable = tree.available(owned);
		expect(buyable.size() == 4, "a fresh player can buy the four roots");
		expect(!tree.can_buy("flair_2", owned, 1000.0),
			"...but not flair_2 without flair_1");
	}
	{
		std::vector<std::string> owned = { "flair_1" };
		double money = 500.0;
		double cost = 0.0;
		expect(tree.can_buy("flair_2", owned, money, &cost) && cost == 250.0,
			"flair_1 bought, flair_2 unlocked at cost 250");
		expect(!tree.can_buy("flair_2", owned, 100.0), "100 is not enough for 250");
		expect(!tree.can_buy("flair_1", owned, 1000.0), "owned nodes are not re-buyable");
		expect(tree.buy("flair_2", owned, money), "the purchase goes through");
		expect_near(money, 250.0, 1e-9, "250 deducted");
		expect(owned.size() == 2, "the node joined the owned set");
		expect(!tree.buy("flair_2", owned, money), "no double buying");
		money = 0.0;
		expect(!tree.buy("psychology_1", owned, money), "no money, no skills");
	}

	// --- effect aggregation ------------------------------------------------------------
	{
		const std::vector<std::string> owned = {
			"flair_1", "flair_2", "steady_1", "steady_2", "alchemy_1", "psychology_1"
		};
		const SkillSummary summary = tree.summarize(owned);
		expect_near(summary.service_speed_mult, 0.9, 1e-9, "service speed 0.9");
		expect_near(summary.tip_multiplier, 1.15, 1e-9, "tips 1.15");
		expect_near(summary.pour_shake_mult, 0.7, 1e-9, "shake 0.7");
		expect_near(summary.combo_window_mult, 1.2, 1e-9, "combo window 1.2");
		expect(summary.reveal_tells && !summary.reveal_lies,
			"psychology_1 reveals tells, not lies");
		expect(summary.unlocked_ingredients.size() == 2
			&& summary.unlocked_ingredients[0] == "laxative"
			&& summary.unlocked_ingredients[1] == "sedative",
			"alchemy unlocks sorted: laxative, sedative");

		const SkillSummary empty = tree.summarize({});
		expect_near(empty.service_speed_mult, 1.0, 1e-9, "no skills: neutral speed");
		expect_near(empty.tip_multiplier, 1.0, 1e-9, "no skills: neutral tips");
		expect(!empty.reveal_tells && empty.unlocked_ingredients.empty(),
			"no skills: nothing revealed, nothing unlocked");
	}

	// --- dialogue skill ids must exist in the tree ------------------------------------
	{
		lc::DialogueRunner dialogue;
		expect(dialogue.load(read_repo_file("Data/Dialogue/act1_harris_meeting.json"), &error),
			"the real dialogue loads");
		lc::DialogueWorld world;
		expect(dialogue.start(world), "and starts");
		// The refuse choice gates on `psychology_1` - a skill the tree knows.
		expect(tree.find("psychology_1") != nullptr,
			"the dialogue's skill condition names a real skill node");
	}

	// --- A6: the summary as the pour/serve simulation feels it ---------------------
	{
		// Real nodes, real multipliers: steady_2 widens the speed
		// windows by 1.2 - GAME-DESIGN 5.1's "longer combo windows".
		const SkillSummary steady = tree.summarize({ "steady_1", "steady_2" });
		expect_near(steady.combo_window_mult, 1.2, 1e-9,
			"steady_2 composes to 1.2x windows");

		lc::RecipeSpec spec;
		spec.id = "wired";
		spec.continuity_break_s = 4.0;
		spec.steps = { { "gin", 0.4, 0.7 }, { "tonic", 0.0, 0.5 }, { "lime", 0.8, 1.0 } };
		const lc::RecipeSpec wide = lc::apply_combo_windows(steady, spec);

		// Center 0.55, half 0.15 x 1.2 = 0.18 -> [0.37, 0.73].
		expect_near(wide.steps[0].flow_min, 0.37, 1e-9,
			"the window opens on the low side");
		expect_near(wide.steps[0].flow_max, 0.73, 1e-9,
			"and on the high side");
		// Center 0.25, half 0.3 -> [-0.05, 0.55], clamped at the floor.
		expect_near(wide.steps[1].flow_min, 0.0, 1e-9,
			"a window cannot pour below zero");
		expect_near(wide.steps[1].flow_max, 0.55, 1e-9,
			"the rest of the growth lands above");
		// Center 0.9, half 0.12 -> [0.78, 1.02], clamped at the brim.
		expect_near(wide.steps[2].flow_min, 0.78, 1e-9,
			"the growth lands below");
		expect_near(wide.steps[2].flow_max, 1.0, 1e-9,
			"a window cannot pour past the brim");
		expect(wide.id == spec.id && wide.continuity_break_s == spec.continuity_break_s,
			"the rest of the recipe is untouched");
		expect(spec.steps[0].flow_min == 0.4 && spec.steps[0].flow_max == 0.7,
			"the input spec is never touched");
	}
	{
		// Nothing bought: the simulation feels nothing.
		const SkillSummary plain = tree.summarize({});
		lc::RecipeSpec spec;
		spec.steps = { { "gin", 0.4, 0.7 } };
		const lc::RecipeSpec unchanged = lc::apply_combo_windows(plain, spec);
		expect(unchanged.steps[0].flow_min == 0.4 && unchanged.steps[0].flow_max == 0.7,
			"an empty summary leaves the windows alone");
	}
	{
		// Hand-built summaries are not judged, but a tightening must
		// not invert the window it shrinks.
		SkillSummary tight;
		tight.combo_window_mult = 0.5;
		lc::RecipeSpec spec;
		spec.steps = { { "gin", 0.4, 0.7 } }; // center 0.55, half 0.075
		const lc::RecipeSpec shrunk = lc::apply_combo_windows(tight, spec);
		expect_near(shrunk.steps[0].flow_min, 0.475, 1e-9,
			"half the width off the low side");
		expect_near(shrunk.steps[0].flow_max, 0.625, 1e-9,
			"and off the high side");
		expect(shrunk.steps[0].flow_min < shrunk.steps[0].flow_max,
			"a tighter window is still a window");
		SkillSummary broken;
		broken.combo_window_mult = 0.0;
		const lc::RecipeSpec untouched = lc::apply_combo_windows(broken, spec);
		expect(untouched.steps[0].flow_min == 0.4 && untouched.steps[0].flow_max == 0.7,
			"a non-positive multiplier moves nothing");
	}
	{
		// Flair's tips: 15% on top of whatever the serve earned -
		// GAME-DESIGN 5.1's "higher tips".
		const SkillSummary flair = tree.summarize({ "flair_1", "flair_2" });
		expect_near(flair.tip_multiplier, 1.15, 1e-9,
			"flair_2 composes to 1.15x tips");

		lc::ServeOutcome correct;
		correct.correct = true;
		correct.tip_multiplier = 1.4; // a good recipe on top
		const lc::ServeOutcome paid = lc::apply_skill_tips(flair, correct);
		expect_near(paid.tip_multiplier, 1.61, 1e-9, "1.4 x 1.15");
		expect(paid.correct && paid.mood_bonus == correct.mood_bonus,
			"only the payout moves");

		lc::ServeOutcome wrong;
		wrong.tip_multiplier = 0.0;
		expect(lc::apply_skill_tips(flair, wrong).tip_multiplier == 0.0,
			"a wrong serve still pays nothing");
		expect_near(lc::apply_skill_tips(SkillSummary{}, correct).tip_multiplier, 1.4, 1e-9,
			"skills nobody bought change no tips");
	}

	// --- strict refusals -------------------------------------------------------------------
	expect(refuses("{}"), "missing branches refused");
	expect(refuses("{\"branches\":{}}"), "empty branches refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[]}}}"), "empty nodes refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"cost\":1}]}}}"), "node without id refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\"}]}}}"),
		"node without cost refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\",\"cost\":-1}]}}}"),
		"negative cost refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":["
			"{\"id\":\"x\",\"cost\":1,\"requires\":[\"ghost\"]}]}}}"),
		"unknown requirement refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":["
			"{\"id\":\"x\",\"cost\":1},{\"id\":\"x\",\"cost\":2}]}}}"),
		"duplicate node id refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\",\"cost\":1,"
			"\"effect\":{\"type\":\"service_speed\"}}]}}}"),
		"mult effect without mult refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\",\"cost\":1,"
			"\"effect\":{\"type\":\"service_speed\",\"mult\":0}}]}}}"),
		"zero mult refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\",\"cost\":1,"
			"\"effect\":{\"type\":\"unlock_ingredients\"}}]}}}"),
		"unlock without ids refused");
	expect(refuses("{\"branches\":{\"b\":{\"nodes\":[{\"id\":\"x\",\"cost\":1,"
			"\"effect\":{\"type\":\"flying\"}}]}}}"), "unknown effect type refused");
	expect(refuses("not json"), "garbage refused");

	return lc_test::report("skill tree");
}
