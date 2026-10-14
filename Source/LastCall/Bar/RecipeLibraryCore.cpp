// LAST CALL - recipe library core implementation. See header for the contract.

#include "Bar/RecipeLibraryCore.h"

#include "Core/JsonCore.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

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

	} // namespace

	bool load_recipe(const std::string& json_text, RecipeRecord& out, std::string* error)
	{
		JsonValue doc;
		std::string parse_error;
		if (!json_parse(json_text, doc, &parse_error))
		{
			return refuse(error, "bad JSON: " + parse_error);
		}
		if (doc.kind != JsonValue::Kind::Object)
		{
			return refuse(error, "recipe document is not an object");
		}

		RecipeRecord record;
		if (!doc.get_string("id", record.id) || record.id.empty())
		{
			return refuse(error, "missing or empty `id`");
		}
		if (!doc.get_string("glass", record.glass) || record.glass.empty())
		{
			return refuse(error, "missing or empty `glass`");
		}

		const JsonValue* steps = doc.find("steps");
		if (!steps || steps->kind != JsonValue::Kind::Array || steps->array.empty())
		{
			return refuse(error, "`steps` must be a non-empty array");
		}
		for (std::size_t i = 0; i < steps->array.size(); ++i)
		{
			const JsonValue& step = steps->array[i];
			const std::string where = "step " + std::to_string(i) + ": ";
			if (step.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "not an object");
			}

			MixStep mix;
			if (!step.get_string("ingredient", mix.ingredient) || mix.ingredient.empty())
			{
				return refuse(error, where + "missing `ingredient`");
			}

			const JsonValue* window = step.find("flow_window");
			if (!window || window->kind != JsonValue::Kind::Array || window->array.size() != 2)
			{
				return refuse(error, where + "`flow_window` must be [min, max]");
			}
			if (window->array[0].kind != JsonValue::Kind::Number
				|| window->array[1].kind != JsonValue::Kind::Number)
			{
				return refuse(error, where + "`flow_window` values must be numbers");
			}
			mix.flow_min = window->array[0].number;
			mix.flow_max = window->array[1].number;
			if (mix.flow_min < 0.0 || mix.flow_max > 1.0 || mix.flow_min > mix.flow_max)
			{
				return refuse(error, where + "flow window out of range (0..1, min <= max)");
			}
			record.steps.push_back(mix);

			std::string layer = "base";
			step.get_string("layer", layer); // optional, defaults to base
			record.layers.push_back(layer);
		}

		double continuity = 0.0;
		if (doc.get_number("continuity_break_seconds", continuity))
		{
			if (continuity < 0.0)
			{
				return refuse(error, "`continuity_break_seconds` is negative");
			}
			record.continuity_break_s = continuity;
		}

		const JsonValue* effects = doc.find("effects");
		if (effects && effects->kind == JsonValue::Kind::Object)
		{
			effects->get_number("mood_bonus", record.effects.mood_bonus);
			if (effects->get_number("tip_multiplier", record.effects.tip_multiplier)
				&& record.effects.tip_multiplier <= 0.0)
			{
				return refuse(error, "`tip_multiplier` must be positive");
			}
			effects->get_number("xp_multiplier", record.effects.xp_multiplier);
		}

		const JsonValue* story = doc.find("story");
		if (story && story->kind == JsonValue::Kind::Object)
		{
			story->get_bool("act1_hook", record.act1_hook);
		}

		const JsonValue* secret = doc.find("secret");
		if (secret)
		{
			if (secret->kind != JsonValue::Kind::Object)
			{
				return refuse(error, "`secret` must be an object");
			}
			if (!secret->get_int("fragment", record.secret_fragment)
				|| !secret->get_int("of", record.secret_of)
				|| record.secret_fragment < 1
				|| record.secret_of < record.secret_fragment)
			{
				return refuse(error, "`secret` needs whole numbers with 1 <= fragment <= of");
			}
			if (!secret->get_string("text_key", record.secret_text_key)
				|| record.secret_text_key.empty())
			{
				return refuse(error, "`secret` needs `text_key`");
			}
			secret->get_string("reveal_flag", record.secret_reveal_flag); // optional
			record.has_secret = true;
		}

		out = std::move(record);
		return true;
	}

	RecipeSpec to_solver_spec(const RecipeRecord& recipe)
	{
		RecipeSpec spec;
		spec.id = recipe.id;
		spec.steps = recipe.steps;
		spec.continuity_break_s = recipe.continuity_break_s;
		return spec;
	}

	bool RecipeLibrary::add(const std::string& json_text, std::vector<std::string>* errors)
	{
		RecipeRecord record;
		std::string error;
		if (!load_recipe(json_text, record, &error))
		{
			if (errors)
			{
				errors->push_back(error);
			}
			return false;
		}
		if (find(record.id))
		{
			if (errors)
			{
				errors->push_back("duplicate id `" + record.id + "`");
			}
			return false;
		}
		records_.push_back(std::move(record));
		return true;
	}

	int RecipeLibrary::load_directory(const std::string& directory, std::vector<std::string>* errors)
	{
		namespace fs = std::filesystem;

		int loaded = 0;
		std::error_code ec;
		fs::directory_iterator it(directory, ec);
		if (ec)
		{
			if (errors)
			{
				errors->push_back(directory + ": cannot read directory: " + ec.message());
			}
			return 0;
		}

		std::vector<fs::path> files;
		for (const auto& entry : it)
		{
			std::error_code entry_ec;
			if (entry.is_regular_file(entry_ec) && entry.path().extension() == ".json")
			{
				files.push_back(entry.path());
			}
		}
		std::sort(files.begin(), files.end());

		for (const fs::path& file : files)
		{
			std::ifstream input(file);
			if (!input)
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": unreadable");
				}
				continue;
			}
			std::stringstream buffer;
			buffer << input.rdbuf();

			RecipeRecord record;
			std::string error;
			if (!load_recipe(buffer.str(), record, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			if (find(record.id))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": duplicate id `" + record.id + "`");
				}
				continue;
			}
			records_.push_back(std::move(record));
			++loaded;
		}
		return loaded;
	}

	const RecipeRecord* RecipeLibrary::find(const std::string& id) const
	{
		for (const RecipeRecord& record : records_)
		{
			if (record.id == id)
			{
				return &record;
			}
		}
		return nullptr;
	}

	std::vector<std::string> RecipeLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(records_.size());
		for (const RecipeRecord& record : records_)
		{
			out.push_back(record.id);
		}
		return out;
	}

	bool collect_secret_fragments(const RecipeLibrary& library,
		std::vector<SecretFragment>& out, std::string* error)
	{
		std::vector<SecretFragment> fragments;
		for (const RecipeRecord& record : library.recipes())
		{
			if (!record.has_secret)
			{
				continue;
			}
			SecretFragment fragment;
			fragment.recipe_id = record.id;
			fragment.index = record.secret_fragment;
			fragment.of = record.secret_of;
			fragment.text_key = record.secret_text_key;
			fragment.reveal_flag = record.secret_reveal_flag;
			fragments.push_back(std::move(fragment));
		}

		std::sort(fragments.begin(), fragments.end(),
			[](const SecretFragment& a, const SecretFragment& b) { return a.index < b.index; });

		for (std::size_t i = 0; i < fragments.size(); ++i)
		{
			if (i > 0 && fragments[i].index == fragments[i - 1].index)
			{
				if (error)
				{
					*error = "fragment " + std::to_string(fragments[i].index)
						+ " is claimed by both `" + fragments[i - 1].recipe_id
						+ "` and `" + fragments[i].recipe_id + "`";
				}
				return false;
			}
			if (fragments[i].of != fragments[0].of)
			{
				if (error)
				{
					*error = "`" + fragments[i].recipe_id + "` promises "
						+ std::to_string(fragments[i].of)
						+ " fragments, but the wall has "
						+ std::to_string(fragments[0].of);
				}
				return false;
			}
		}

		out = std::move(fragments);
		return true;
	}

} // namespace lc
