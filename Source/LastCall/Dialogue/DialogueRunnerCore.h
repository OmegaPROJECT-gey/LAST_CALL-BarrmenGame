// LAST CALL - dialogue runner core (ARCHITECTURE.md, section 3).
//
// The engine-independent half of ULCDialogueRunner: loads a dialogue tree
// from Data/Dialogue/*.json with the shared JSON reader, walks it against
// a world state (story flags, faction standing, owned skills, held items,
// the current phase), applies choice/node effects and records what the
// engine-side systems must act on (mood deltas, quests, heat, granted
// items). Engine-independent C++17: the UE wrapper binds widgets, voices
// and the real faction systems to the same code the tests drive.
//
// License: MIT (LICENSE).

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lc {

	/**
	 * What the runner reads conditions against and writes story flags and
	 * faction standing into. The UE side owns the real systems; the runner
	 * only needs this much. Mood, quest, heat and item effects are not
	 * applied here - they land in DialogueRunner::applied() for the caller
	 * to route (items are read back through `items`).
	 */
	struct DialogueWorld
	{
		std::vector<std::pair<std::string, bool>> flags;
		std::vector<std::pair<std::string, double>> standing; // faction -> value
		std::vector<std::string> skills;                      // owned skill ids
		std::vector<std::string> items;                       // held item ids
		std::string phase; // current phase ("Morning"...), empty when unset

		bool has_flag(const std::string& flag) const;
		void set_flag(const std::string& flag, bool value);
		double standing_of(const std::string& faction) const; // missing faction -> 0
		void add_standing(const std::string& faction, double delta);
		bool owns_skill(const std::string& skill_id) const;
		bool owns_item(const std::string& item_id) const;
	};

	/** Conditions in data form, evaluated by the runner. */
	struct DialogueConditionSet
	{
		std::vector<std::pair<std::string, bool>> flags;      // flag -> required value
		std::vector<std::pair<std::string, double>> standing; // faction -> min
		std::vector<std::pair<std::string, bool>> skills;     // skill id -> owned
		std::vector<std::pair<std::string, bool>> items;      // item id -> held
		std::string phase; // required phase; empty when the data does not care
	};

	/**
	 * A mood/quest/heat/item effect the engine must route to its systems.
	 * type "mood": target = customer id, delta = mood delta.
	 * type "quest": target = quest id, state = "started", ...
	 * type "heat":  delta = heat delta (positive draws attention).
	 * type "item":  target = item id, delta = count granted.
	 */
	struct AppliedEffect
	{
		std::string type;
		std::string target;
		std::string state;
		double delta = 0.0;
	};

	struct DialogueChoiceData
	{
		std::string id;
		std::string text_key;
		std::string next;
		DialogueConditionSet conditions;
		// Effects in data form.
		std::vector<std::pair<std::string, bool>> flag_effects;       // flag -> value
		std::vector<std::pair<std::string, double>> standing_effects; // faction -> delta
		std::vector<AppliedEffect> routed_effects;                    // mood/quest/heat/item
	};

	struct DialogueNodeData
	{
		std::string id;
		std::string speaker;
		std::string text_key;
		std::string next;
		bool end = false;
		DialogueConditionSet conditions;
		std::vector<std::pair<std::string, bool>> flag_effects;
		std::vector<std::pair<std::string, double>> standing_effects;
		std::vector<AppliedEffect> routed_effects;
		std::vector<DialogueChoiceData> choices;
	};

	class DialogueRunner
	{
	public:
		/**
		 * Parse a dialogue document. Strict: the entry node exists, every
		 * `next` target exists, node ids are unique, choices carry id,
		 * text_key and next. `error` explains the refusal.
		 */
		bool load(const std::string& json_text, std::string* error = nullptr);

		/**
		 * Load every `*.json` in a directory (the whole scene list, as the
		 * other libraries do). Each refusal lands in `errors` with its
		 * file name; the count of loaded trees is returned. The runner
		 * keeps the last tree loaded.
		 */
		int load_directory(const std::string& directory,
			std::vector<std::string>* errors = nullptr);

		/** Reset to the entry node; applies the entry node's effects. */
		bool start(DialogueWorld& world);

		const std::string& current_text_key() const;
		const std::string& current_speaker() const;
		bool at_end() const;
		bool can_advance() const; // a linear `next` exists

		/** Indices (into the node's choice list) of choices the world allows. */
		std::vector<int> available_choices(const DialogueWorld& world) const;

		/** Follow the linear `next`, applying node effects. False when there is none. */
		bool advance(DialogueWorld& world);

		/** Take a choice by id, applying its effects. False when unknown or filtered out. */
		bool choose(const std::string& choice_id, DialogueWorld& world);

		/** Mood/quest/heat/item effects applied so far, for the engine to route. */
		const std::vector<AppliedEffect>& applied() const { return applied_; }

		/** Flags recorded by the data's interrupt_recording block (empty when absent). */
		std::string interrupt_flag_ignored() const { return interrupt_ignored_; }
		std::string interrupt_flag_heard() const { return interrupt_heard_; }

		const std::string& id() const { return id_; }

	private:
		const DialogueNodeData* node(const std::string& id) const;
		void apply_effects(const std::vector<std::pair<std::string, bool>>& flag_effects,
			const std::vector<std::pair<std::string, double>>& standing_effects,
			const std::vector<AppliedEffect>& routed, DialogueWorld& world);
		bool conditions_pass(const DialogueWorld& world,
			const DialogueConditionSet& conditions) const;

		std::string id_;
		std::string entry_;
		std::string current_;
		bool loaded_ = false;
		std::vector<DialogueNodeData> nodes_;
		std::vector<AppliedEffect> applied_;
		std::string interrupt_ignored_;
		std::string interrupt_heard_;
	};

} // namespace lc
