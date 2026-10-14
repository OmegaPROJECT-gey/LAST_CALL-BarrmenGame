// LAST CALL - social consequences core implementation. See header for the contract.

#include "Social/SocialCore.h"

#include "Core/ConsequenceLogCore.h"
#include "Core/JsonCore.h"

#include <algorithm>
#include <cmath>

namespace lc {

	namespace {

		bool refuse(std::string* error, const std::string& reason)
		{
			if (error)
			{
				*error = reason;
			}
			return false;
		}

		bool read_localized(const JsonValue& body, const std::string& where,
			std::string& en, std::string& ru, std::string* error)
		{
			const JsonValue* name = body.find("display_name");
			if (!name)
			{
				name = body.find("note");
			}
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", en) || en.empty()
				|| !name->get_string("ru", ru) || ru.empty())
			{
				return refuse(error, where + "needs a bilingual `display_name`/`note` (en and ru)");
			}
			return true;
		}

	} // namespace

	// --- library -----------------------------------------------------------------------

	bool FactionLibrary::load(const std::string& json_text, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "faction document is not an object");
		}

		const JsonValue* factions = doc.find("factions");
		if (!factions || factions->kind != JsonValue::Kind::Object || factions->object.empty())
		{
			return refuse(error, "`factions` must be a non-empty object");
		}

		for (const auto& member : factions->object)
		{
			FactionSpec spec;
			spec.id = member.first;
			const std::string where = "faction `" + spec.id + "`: ";
			const JsonValue& body = member.second;
			if (body.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "not an object");
			}
			if (!read_localized(body, where, spec.name_en, spec.name_ru, error))
			{
				return false;
			}
			if (!body.get_number("initial_standing", spec.initial_standing))
			{
				return refuse(error, where + "`initial_standing` must be a number");
			}
			if (find(spec.id))
			{
				return refuse(error, "duplicate faction id `" + spec.id + "`");
			}
			factions_.push_back(std::move(spec));
		}

		if (const JsonValue* reactions = doc.find("reactions"))
		{
			if (reactions->kind != JsonValue::Kind::Array)
			{
				return refuse(error, "`reactions` must be an array");
			}
			for (const JsonValue& entry : reactions->array)
			{
				if (entry.kind != JsonValue::Kind::Object)
				{
					return refuse(error, "reaction is not an object");
				}

				Reaction reaction;
				if (!entry.get_string("flag", reaction.flag) || reaction.flag.empty())
				{
					return refuse(error, "reaction without `flag`");
				}
				const std::string where = "reaction `" + reaction.flag + "`: ";

				if (const JsonValue* standing = entry.find("standing"))
				{
					if (standing->kind != JsonValue::Kind::Object || standing->object.empty())
					{
						return refuse(error, where + "`standing` must be a non-empty object");
					}
					for (const auto& delta : standing->object)
					{
						if (!find(delta.first))
						{
							return refuse(error, where + "unknown faction `" + delta.first + "`");
						}
						if (delta.second.kind != JsonValue::Kind::Number)
						{
							return refuse(error, where + "standing deltas must be numbers");
						}
						reaction.standing.emplace_back(delta.first, delta.second.number);
					}
				}
				else
				{
					return refuse(error, where + "`standing` is required");
				}

				if (!read_localized(entry, where, reaction.note_en, reaction.note_ru, error))
				{
					return false;
				}

				if (reaction_for(reaction.flag))
				{
					return refuse(error, "duplicate reaction flag `" + reaction.flag + "`");
				}
				reactions_.push_back(std::move(reaction));
			}
		}
		return true;
	}

	const FactionSpec* FactionLibrary::find(const std::string& faction_id) const
	{
		for (const FactionSpec& spec : factions_)
		{
			if (spec.id == faction_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> FactionLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(factions_.size());
		for (const FactionSpec& spec : factions_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	const Reaction* FactionLibrary::reaction_for(const std::string& flag) const
	{
		for (const Reaction& reaction : reactions_)
		{
			if (reaction.flag == flag)
			{
				return &reaction;
			}
		}
		return nullptr;
	}

	// --- ledger ------------------------------------------------------------------------

	StandingLedger::StandingLedger(const FactionLibrary& library)
		: library_(&library)
	{
		standing_.reserve(library.factions().size());
		for (const FactionSpec& spec : library.factions())
		{
			standing_.emplace_back(spec.id, spec.initial_standing);
		}
	}

	double StandingLedger::standing(const std::string& faction_id) const
	{
		for (const auto& entry : standing_)
		{
			if (entry.first == faction_id)
			{
				return entry.second;
			}
		}
		return 0.0;
	}

	void StandingLedger::set_standing(const std::string& faction_id, double value)
	{
		for (auto& entry : standing_)
		{
			if (entry.first == faction_id)
			{
				entry.second = value;
				return;
			}
		}
		standing_.emplace_back(faction_id, value); // factions added after load (future arcs)
	}

	bool StandingLedger::applied(const std::string& flag) const
	{
		return std::find(applied_.begin(), applied_.end(), flag) != applied_.end();
	}

	int StandingLedger::apply_flag(const std::string& flag)
	{
		const Reaction* reaction = library_->reaction_for(flag);
		if (!reaction || applied(flag))
		{
			return 0;
		}
		applied_.push_back(flag);
		for (const auto& delta : reaction->standing)
		{
			for (auto& entry : standing_)
			{
				if (entry.first == delta.first)
				{
					entry.second += delta.second;
					break;
				}
			}
		}
		return static_cast<int>(reaction->standing.size());
	}

	int StandingLedger::apply_log(const ConsequenceLog& log)
	{
		int moved = 0;
		for (const ConsequenceEntry& entry : log.entries())
		{
			moved += apply_flag(entry.flag);
		}
		return moved;
	}

} // namespace lc
