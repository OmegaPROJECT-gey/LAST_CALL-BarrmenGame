// LAST CALL - dialogue runner core unit tests.
//
// Runs without Unreal: scripts/test-dialogue.sh compiles this against the
// runner, the JSON reader and the test harness with g++. Drives the real
// files in Data/Dialogue (harris, kane, shadow) end to end.

#include "Dialogue/DialogueRunnerCore.h"

#include "test_support.h"

#include <filesystem>
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
	// --- choice data by index (the UE wrapper reads text keys through it) -----------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(real_dialogue, nullptr) && runner.start(world), "reload");
		runner.advance(world);
		const std::vector<int> allowed = runner.available_choices(world);
		expect(!allowed.empty(), "choices to read");
		const lc::DialogueChoiceData* first = runner.choice(allowed.front());
		expect(first != nullptr && first->id == "accept"
			&& first->text_key == "dlg.harris.choice.accept",
			"choice data reads by index");
		expect(runner.choice(-1) == nullptr && runner.choice(99) == nullptr,
			"choice data refuses out-of-range indices");
	}

	// --- the whole scene list on disk (mirror of the CI lint) ----------------------
	{
		DialogueRunner library;
		std::vector<std::string> errors;
		const int loaded = library.load_directory(
			lc_test::repo_root() + "/Data/Dialogue", &errors);
		expect(loaded >= 7 && errors.empty(), "every dialogue in Data/Dialogue loads");
		// The last file in sorted order wins; derive it from disk so the
		// assertion never needs retyping when a new scene lands.
		std::string newest;
		std::error_code dir_ec;
		const std::filesystem::directory_iterator end;
		for (std::filesystem::directory_iterator it(
				 lc_test::repo_root() + "/Data/Dialogue", dir_ec);
			!dir_ec && it != end; it.increment(dir_ec))
		{
			const std::filesystem::path& path = it->path();
			if (path.extension() == ".json" && path.stem().string() > newest)
			{
				newest = path.stem().string();
			}
		}
		expect(!newest.empty() && library.id() == newest,
			"load order is sorted, the newest scene stays loaded");

		DialogueRunner missing;
		errors.clear();
		expect(missing.load_directory("/no/such/directory", &errors) == 0 && !errors.empty(),
			"a missing directory reports an error");
	}
	// --- has_item condition: the Thursday scene (C2) --------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(
			read_repo_file("Data/Dialogue/act1_shadow_first_thursday.json"), nullptr)
			&& runner.start(world), "act1_shadow_first_thursday loads and starts");
		runner.advance(world); // into the order
		expect(runner.available_choices(world).size() == 2,
			"without the drink the serving choice is hidden");
		world.items.push_back("black_manhattan");
		expect(runner.available_choices(world).size() == 3,
			"holding the Black Manhattan reveals the serving choice");
		expect(runner.choose("serve", world), "the served route is taken");
		expect_near(world.standing_of("syndicate"), 1.0, 1e-9,
			"serving raised syndicate standing");
		bool quest_started = false;
		for (const lc::AppliedEffect& effect : runner.applied())
		{
			if (effect.type == "quest" && effect.target == "shadow_shield"
				&& effect.state == "started")
			{
				quest_started = true;
			}
		}
		expect(quest_started, "the shield quest is routed to the engine");
	}
	// --- act 2, beat 12: Kane sells the third fragment (C2) -------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_kane_price.json"), nullptr)
			&& runner.start(world), "act2_kane_price loads and starts");
		runner.advance(world); // into the offer
		expect(runner.available_choices(world).size() == 2,
			"without psychology the counter choice is hidden");
		world.skills.push_back("psychology_1");
		expect(runner.available_choices(world).size() == 3,
			"psychology_1 reveals the counter choice");
		expect(runner.choose("answer", world), "the honest answer is taken");
		expect(world.has_flag("kane_price_paid"), "the trade is recorded as a story flag");
		bool item_granted = false;
		for (const lc::AppliedEffect& effect : runner.applied())
		{
			if (effect.type == "item" && effect.target == "fragment_03")
			{
				item_granted = true;
				expect_near(effect.delta, 1.0, 1e-9, "one fragment granted");
			}
		}
		expect(item_granted, "the third fragment is granted for the engine to bank");
		expect_near(world.standing_of("civilian"), 1.0, 1e-9, "honesty warmed The Room");
		expect(runner.at_end() && runner.current_text_key() == "dlg.kane.price.paid",
			"the trade ends on Kane's last line");
	}
	// --- act 2, beat 14: heat as a routed consequence (C2) --------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_shadow_test.json"), nullptr)
			&& runner.start(world), "act2_shadow_test loads and starts");
		runner.advance(world); // into the test
		expect(runner.choose("confess", world), "the volunteered loyalty is taken");
		expect_near(world.standing_of("syndicate"), 2.0, 1e-9,
			"unsolicited loyalty warmed the Syndicate");
		bool heat_raised = false;
		for (const lc::AppliedEffect& effect : runner.applied())
		{
			if (effect.type == "heat")
			{
				heat_raised = true;
				expect_near(effect.delta, 1.0, 1e-9, "heat +1 routed");
			}
		}
		expect(heat_raised, "the heat effect is in the applied list for the engine");
		expect(runner.at_end() && runner.current_text_key() == "dlg.shadow.test.loyal",
			"the confession ends on Shadow's reply");
	}
	// --- act 1, beat 7 aftermath: Harris and the break-in (C2) ----------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_harris_breakin.json"), nullptr)
			&& runner.start(world), "act1_harris_breakin loads and starts");
		runner.advance(world); // into the report
		expect(runner.available_choices(world).size() == 3,
			"the break-in report has three answers");
		expect(runner.choose("withhold", world), "the quiet refusal is taken");
		expect(world.has_flag("harris_withheld_breakin"), "withholding is recorded");
		expect_near(world.standing_of("bureau"), -1.0, 1e-9,
			"withholding cost bureau standing");
		expect(runner.at_end() && runner.current_text_key() == "dlg.harris.breakin.withheld",
			"Harris ends on the warning");
	}
	// --- act 3, beat 19: the unmasking, gated by syndicate trust (C2) ---------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_shadow_unmasked.json"), nullptr)
			&& runner.start(world), "act3_shadow_unmasked loads and starts");
		runner.advance(world); // into the reveal
		expect(runner.available_choices(world).size() == 2,
			"without syndicate trust the press choice is hidden");
		world.add_standing("syndicate", 2.0);
		expect(runner.available_choices(world).size() == 3, "trust reveals the press choice");
		expect(runner.choose("press", world), "you press for the name");
		expect(world.has_flag("shadow_named_chain"),
			"the hint about who is really above Shadow is recorded");
		expect(runner.at_end() && runner.current_text_key() == "dlg.shadow.unmasked.named",
			"the press route ends on the hint");
	}
	// --- act 1, beat 4a: the Syndicate's drop (C3) ---------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_syndicate_drop.json"), nullptr)
			&& runner.start(world), "act1_syndicate_drop loads and starts");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"the drop has three ways to take it");
		expect(runner.choose("take", world), "the drop is taken quietly");
		expect(world.has_flag("syndicate_drop_taken"), "the drop is recorded as a flag");
		expect(world.has_flag("act1_syndicate_drop_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.vera.drop.taken",
			"the drop ends on Vera's last line");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_syndicate_drop.json"), nullptr)
			&& !fresh.start(world), "the drop does not play twice");
	}
	// --- act 1, beat 4b: the Ring's delivery (C3) ----------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_ring_delivery.json"), nullptr)
			&& runner.start(world), "act1_ring_delivery loads and starts");
		runner.advance(world); // into the choice
		expect(runner.choose("refuse", world), "another door is asked for");
		expect(world.has_flag("ring_delivery_refused"), "the refusal is recorded as a flag");
		expect(world.has_flag("act1_ring_delivery_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.sal.delivery.refused",
			"the refusal ends on Sal's last line");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_ring_delivery.json"), nullptr)
			&& !fresh.start(world), "the delivery does not play twice");
	}
	// --- act 1, beat 4c: the Station's blind eye (C3) ------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_station_blind_eye.json"), nullptr)
			&& runner.start(world), "act1_station_blind_eye loads and starts");
		runner.advance(world); // into the choice
		expect(runner.choose("nod_understood", world), "the arrangement is understood");
		expect(world.has_flag("station_understood"), "the nod is recorded as a flag");
		expect(world.has_flag("act1_station_blind_eye_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.agnes.rent.understood",
			"the nod ends on Mrs. Boyle's last line");
		expect_near(world.standing_of("station"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_station_blind_eye.json"), nullptr)
			&& !fresh.start(world), "the rent day does not play twice");
	}
	// --- act 2, C3: Vera's favor - the register (arc 1 of 3) ----------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_vera_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the drop relationship");
		world.set_flag("act1_syndicate_drop_played", true);
		expect(runner.start(world), "act2_vera_favor starts once the drop exists");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the register");
		expect(runner.choose("play_along", world), "the quiet night is rung up");
		expect(world.has_flag("vera_launder_helped"), "the help is recorded as a flag");
		expect(world.has_flag("act2_vera_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.vera.favor.helped",
			"the favor ends on Vera's count");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Vera's fork - the count book (arc 1 ending) -------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_vera_fork.json"), nullptr)
			&& !runner.start(world), "the book waits for the favor");
		world.set_flag("act2_vera_favor_played", true);
		expect(runner.start(world), "act3_vera_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the book");
		expect(runner.choose("sell_harris", world), "the book goes where the Bureau pays");
		expect(world.has_flag("vera_book_sold"), "the sale is recorded as a flag");
		expect(world.has_flag("act3_vera_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.vera.book.sold",
			"the fork ends on the tremor");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 2, C3: Sal's favor - the back room (arc 2 of 3) ----------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_sal_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the delivery relationship");
		world.set_flag("act1_ring_delivery_played", true);
		expect(runner.start(world), "act2_sal_favor starts once the delivery exists");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the back room");
		expect(runner.choose("host", world), "the roof is lent");
		expect(world.has_flag("sal_meeting_hosted"), "the hosting is recorded as a flag");
		expect(world.has_flag("act2_sal_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.sal.vote.hosted",
			"the favor ends on Sal's advance tab");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Sal's fork - the season crate (arc 2 ending) ------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_sal_fork.json"), nullptr)
			&& !runner.start(world), "the crate waits for the favor");
		world.set_flag("act2_sal_favor_played", true);
		expect(runner.start(world), "act3_sal_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the crate");
		expect(runner.choose("refuse", world), "another freezer is asked for");
		expect(world.has_flag("sal_crate_refused"), "the refusal is recorded as a flag");
		expect(world.has_flag("act3_sal_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.sal.crate.refused",
			"the fork ends on the count");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 2, C3: Mrs. Boyle's favor - the lease (arc 3 of 3) -------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_agnes_favor.json"), nullptr)
			&& !runner.start(world), "the lease waits for the rent day");
		world.set_flag("act1_station_blind_eye_played", true);
		expect(runner.start(world), "act2_agnes_favor starts once the blind eye exists");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the lease");
		expect(runner.choose("sign", world), "the line is initialed");
		expect(world.has_flag("agnes_lease_signed"), "the signature is recorded as a flag");
		expect(world.has_flag("act2_agnes_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.agnes.lease.signed",
			"the favor ends on the landlady's pleasure");
		expect_near(world.standing_of("station"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Mrs. Boyle's fork - the notebook (arc 3 ending) ---------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_agnes_fork.json"), nullptr)
			&& !runner.start(world), "the notebook waits for the favor");
		world.set_flag("act2_agnes_favor_played", true);
		expect(runner.start(world), "act3_agnes_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the notebook");
		expect(runner.choose("cover", world), "the notebook is closed");
		expect(world.has_flag("agnes_covered"), "the cover is recorded as a flag");
		expect(world.has_flag("act3_agnes_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.agnes.warn.covered",
			"the fork ends on the rent under the till");
		expect_near(world.standing_of("station"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Dana's arrival (arc 4 of 12) ----------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_dana_arrival.json"), nullptr)
			&& runner.start(world), "act1_dana_arrival loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to meet the press");
		expect(runner.choose("quote", world), "a line is given");
		expect(world.has_flag("dana_quote_given"), "the quote is recorded as a flag");
		expect(world.has_flag("act1_dana_arrival_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.dana.arrive.quoted",
			"the evening ends on the notebook");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_dana_arrival.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Dana's favor - the uncle (arc 4) ------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_dana_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the arrival");
		world.set_flag("act1_dana_arrival_played", true);
		expect(runner.start(world), "act2_dana_favor starts after the arrival");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the uncle");
		expect(runner.choose("talk", world), "the uncle is talked about");
		expect(world.has_flag("dana_uncle_talked"), "the talk is recorded as a flag");
		expect(world.has_flag("act2_dana_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.dana.favor.talked",
			"the favor ends on the shorthand");
		expect_near(world.standing_of("bureau"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Dana's fork - the photograph (arc 4 ending) -------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_dana_fork.json"), nullptr)
			&& !runner.start(world), "the photograph waits for the favor");
		world.set_flag("act2_dana_favor_played", true);
		expect(runner.start(world), "act3_dana_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the front page");
		expect(runner.choose("let_print", world), "the story runs");
		expect(world.has_flag("dana_story_printed"), "the print is recorded as a flag");
		expect(world.has_flag("act3_dana_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.dana.fork.printed",
			"the fork ends on six columns");
		expect_near(world.standing_of("bureau"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Mack's first shift (arc 5 of 12) ------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_mack_first_shift.json"), nullptr)
			&& runner.start(world), "act1_mack_first_shift loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the door");
		expect(runner.choose("let_watch", world), "the door is lent");
		expect(world.has_flag("mack_watch_ok"), "the watch is recorded as a flag");
		expect(world.has_flag("act1_mack_first_shift_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.mack.door.watching",
			"the evening ends on the unblinking door");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_mack_first_shift.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Mack's favor - the sponsor line (arc 5) -----------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_mack_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the first shift");
		world.set_flag("act1_mack_first_shift_played", true);
		expect(runner.start(world), "act2_mack_favor starts after the first shift");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the sponsor line");
		expect(runner.choose("sign", world), "the name is signed");
		expect(world.has_flag("mack_sponsored"), "the sponsorship is recorded as a flag");
		expect(world.has_flag("act2_mack_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.mack.sponsor.signed",
			"the favor ends on the boxer's signature");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Mack's fork - the identification (arc 5 ending) ---------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_mack_fork.json"), nullptr)
			&& !runner.start(world), "the notebook waits for the favor");
		world.set_flag("act2_mack_favor_played", true);
		expect(runner.start(world), "act3_mack_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the identification");
		expect(runner.choose("alibi", world), "the alibi is given");
		expect(world.has_flag("mack_defended"), "the defense is recorded as a flag");
		expect(world.has_flag("act3_mack_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.mack.id.defended",
			"the fork ends on the till's agreement");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Denny's return (arc 6 of 12) ----------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_denny_return.json"), nullptr)
			&& runner.start(world), "act1_denny_return loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the doorway");
		expect(runner.choose("welcome", world), "the pour lands early");
		expect(world.has_flag("denny_welcomed"), "the welcome is recorded as a flag");
		expect(world.has_flag("act1_denny_return_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.denny.return.welcomed",
			"the evening ends on the turned key");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_denny_return.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Denny's favor - the shifts (arc 6) ----------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_denny_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the return");
		world.set_flag("act1_denny_return_played", true);
		expect(runner.start(world), "act2_denny_favor starts after the return");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the shifts");
		expect(runner.choose("hire", world), "the early shift is given");
		expect(world.has_flag("denny_hired"), "the hire is recorded as a flag");
		expect(world.has_flag("act2_denny_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.denny.shifts.hired",
			"the favor ends on the tied apron");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Denny's fork - the memories (arc 6 ending) --------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_denny_fork.json"), nullptr)
			&& !runner.start(world), "the envelope waits for the favor");
		world.set_flag("act2_denny_favor_played", true);
		expect(runner.start(world), "act3_denny_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the envelope");
		expect(runner.choose("protect", world), "the memories are kept");
		expect(world.has_flag("denny_protected"), "the protection is recorded as a flag");
		expect(world.has_flag("act3_denny_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.denny.memory.protected",
			"the fork ends on the unopened envelope");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Amos's sermon (arc 7 of 12) -----------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_amos_sermon.json"), nullptr)
			&& runner.start(world), "act1_amos_sermon loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the sermon");
		expect(runner.choose("let_preach", world), "the floor is given");
		expect(world.has_flag("amos_sermon_ok"), "the floor is recorded as a flag");
		expect(world.has_flag("act1_amos_sermon_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.amos.sermon.preaching",
			"the evening ends on the converted stools");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_amos_sermon.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Amos's favor - the shelter (arc 7) ----------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_amos_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the sermon");
		world.set_flag("act1_amos_sermon_played", true);
		expect(runner.start(world), "act2_amos_favor starts after the sermon");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the shelter");
		expect(runner.choose("open", world), "the back room opens");
		expect(world.has_flag("amos_shelter_ok"), "the shelter is recorded as a flag");
		expect(world.has_flag("act2_amos_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.amos.shelter.opened",
			"the favor ends on the drying coats");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Amos's fork - the faces notebook (arc 7 ending) ---------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_amos_fork.json"), nullptr)
			&& !runner.start(world), "the notebook waits for the favor");
		world.set_flag("act2_amos_favor_played", true);
		expect(runner.start(world), "act3_amos_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the notebook");
		expect(runner.choose("break_log", world), "the log is broken");
		expect(world.has_flag("amos_log_broken"), "the break is recorded as a flag");
		expect(world.has_flag("act3_amos_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.amos.notes.broken",
			"the fork ends on the burning pages");
		expect_near(world.standing_of("station"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Ivy's corner (arc 8 of 12) ------------------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_ivy_wifi.json"), nullptr)
			&& runner.start(world), "act1_ivy_wifi loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the outlet");
		expect(runner.choose("trust", world), "the outlet is left alone");
		expect(world.has_flag("ivy_wifi_ok"), "the trust is recorded as a flag");
		expect(world.has_flag("act1_ivy_wifi_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.ivy.wifi.trusted",
			"the evening ends on the renamed network");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_ivy_wifi.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Ivy's favor - the dead drop (arc 8) ---------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_ivy_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the corner");
		world.set_flag("act1_ivy_wifi_played", true);
		expect(runner.start(world), "act2_ivy_favor starts after the corner");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the drive");
		expect(runner.choose("hold", world), "the drive is held");
		expect(world.has_flag("ivy_drop_held"), "the hold is recorded as a flag");
		expect(world.has_flag("act2_ivy_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.ivy.drop.held",
			"the favor ends on the four hours");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Ivy's fork - the extraction (arc 8 ending) --------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_ivy_fork.json"), nullptr)
			&& !runner.start(world), "the extraction waits for the favor");
		world.set_flag("act2_ivy_favor_played", true);
		expect(runner.start(world), "act3_ivy_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the bag");
		expect(runner.choose("shelter", world), "the stockroom opens");
		expect(world.has_flag("ivy_hidden"), "the hiding is recorded as a flag");
		expect(world.has_flag("act3_ivy_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.ivy.extract.hidden",
			"the fork ends on the cat in the stockroom");
		expect_near(world.standing_of("ring"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 1, C3: Walter's first pour (arc 9 of 12) -----------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act1_walter_first.json"), nullptr)
			&& runner.start(world), "act1_walter_first loads and starts cold");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the signature");
		expect(runner.choose("honor", world), "the recipe is poured as chalked");
		expect(world.has_flag("walter_signature_poured"), "the pour is recorded as a flag");
		expect(world.has_flag("act1_walter_first_played"), "the evening records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.walter.first.honored",
			"the evening ends on the toast to chalk");
		expect_near(world.standing_of("civilian"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
		DialogueRunner fresh;
		expect(fresh.load(read_repo_file("Data/Dialogue/act1_walter_first.json"), nullptr)
			&& !fresh.start(world), "the evening does not play twice");
	}
	// --- act 2, C3: Walter's favor - the basement (arc 9) -------------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act2_walter_favor.json"), nullptr)
			&& !runner.start(world), "the favor waits for the first pour");
		world.set_flag("act1_walter_first_played", true);
		expect(runner.start(world), "act2_walter_favor starts after the first pour");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the basement");
		expect(runner.choose("open", world), "the basement opens");
		expect(world.has_flag("walter_basement_opened"), "the opening is recorded as a flag");
		expect(world.has_flag("act2_walter_favor_played"), "the favor records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.walter.basement.opened",
			"the favor ends on the honest lock");
		expect_near(world.standing_of("bureau"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- act 3, C3: Walter's fork - the index (arc 9 ending) ----------------------
	{
		DialogueRunner runner;
		DialogueWorld world;
		expect(runner.load(read_repo_file("Data/Dialogue/act3_walter_fork.json"), nullptr)
			&& !runner.start(world), "the index waits for the favor");
		world.set_flag("act2_walter_favor_played", true);
		expect(runner.start(world), "act3_walter_fork starts after the favor");
		runner.advance(world); // into the choice
		expect(runner.available_choices(world).size() == 3,
			"three ways to answer the index");
		expect(runner.choose("burn", world), "the index burns");
		expect(world.has_flag("walter_index_burned"), "the burning is recorded as a flag");
		expect(world.has_flag("act3_walter_fork_played"), "the fork records itself done");
		expect(runner.at_end() && runner.current_text_key() == "dlg.walter.index.burned",
			"the fork ends on the curling keys");
		expect_near(world.standing_of("syndicate"), 0.0, 1e-9,
			"the scene moves no standing - the city's table does");
	}
	// --- phase condition ---------------------------------------------------------------
	{
		DialogueRunner runner;
		std::string error;
		expect(runner.load("{\"entry\":\"n\",\"nodes\":{\"n\":{\"text_key\":\"t\","
			"\"conditions\":[{\"type\":\"phase\",\"id\":\"Night\"}],\"end\":true}}}", &error),
			"a phase condition parses");
		DialogueWorld day;
		expect(!runner.start(day), "an unset phase does not match");
		day.phase = "Shift";
		expect(!runner.start(day), "the wrong phase keeps the node closed");
		day.phase = "Night";
		expect(runner.start(day), "the right phase opens the node");
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
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"conditions\":[{\"type\":\"phase\"}]}}}"),
		"phase condition without id refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"conditions\":[{\"type\":\"phase\",\"id\":\"Night\"},"
		"{\"type\":\"phase\",\"id\":\"Shift\"}]}}}"),
		"two phase conditions on one owner refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"effects\":[{\"type\":\"item\"}]}}}"),
		"item effect without id refused");
	expect(refuses("{\"entry\":\"a\",\"nodes\":{\"a\":{\"text_key\":\"x\","
		"\"effects\":[{\"type\":\"heat\"}]}}}"),
		"heat effect without delta refused");
	expect(refuses("not json"), "garbage refused");

	return lc_test::report("dialogue runner");
}
