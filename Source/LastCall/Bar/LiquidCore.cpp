// LAST CALL - liquid core implementation. See header for the contract.

#include "Bar/LiquidCore.h"

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

		// Validate one `{ id: {...} }` entry of the `liquids` map.
		bool parse_entry(const std::string& id, const JsonValue& body, LiquidSpec& out,
			std::string* error)
		{
			const std::string where = "liquid `" + id + "`: ";
			if (body.kind != JsonValue::Kind::Object)
			{
				return refuse(error, where + "not an object");
			}

			LiquidSpec spec;
			spec.id = id;
			const JsonValue* name = body.find("display_name");
			if (!name || name->kind != JsonValue::Kind::Object
				|| !name->get_string("en", spec.name_en) || spec.name_en.empty()
				|| !name->get_string("ru", spec.name_ru) || spec.name_ru.empty())
			{
				return refuse(error, where + "`display_name` needs en and ru");
			}
			if (!body.get_number("viscosity", spec.viscosity) || spec.viscosity <= 0.0)
			{
				return refuse(error, where + "`viscosity` must be positive");
			}
			if (!body.get_number("density", spec.density) || spec.density < 0.5 || spec.density > 2.0)
			{
				return refuse(error, where + "`density` must be within 0.5..2.0");
			}
			if (!body.get_number("foam", spec.foam_fraction)
				|| spec.foam_fraction < 0.0 || spec.foam_fraction > 1.0)
			{
				return refuse(error, where + "`foam` must be within 0..1");
			}
			if (!body.get_string("color", spec.color) || spec.color.size() != 7
				|| spec.color[0] != '#')
			{
				return refuse(error, where + "`color` must look like #rrggbb");
			}

			out = std::move(spec);
			return true;
		}

		// Parse a whole document into validated entries (duplicates inside the
		// document are refused; duplicates against an existing library are the
		// caller's job).
		bool parse_document(const std::string& json_text, std::vector<LiquidSpec>& out,
			std::string* error)
		{
			JsonValue doc;
			std::string parse_error;
			if (!json_parse(json_text, doc, &parse_error))
			{
				return refuse(error, "bad JSON: " + parse_error);
			}
			if (doc.kind != JsonValue::Kind::Object)
			{
				return refuse(error, "liquid document is not an object");
			}

			const JsonValue* liquids = doc.find("liquids");
			if (!liquids || liquids->kind != JsonValue::Kind::Object || liquids->object.empty())
			{
				return refuse(error, "`liquids` must be a non-empty object");
			}

			std::vector<LiquidSpec> parsed;
			for (const auto& member : liquids->object)
			{
				LiquidSpec spec;
				if (!parse_entry(member.first, member.second, spec, error))
				{
					return false;
				}
				for (const LiquidSpec& existing : parsed)
				{
					if (existing.id == spec.id)
					{
						return refuse(error, "duplicate liquid id `" + spec.id + "`");
					}
				}
				parsed.push_back(std::move(spec));
			}
			out = std::move(parsed);
			return true;
		}

	} // namespace

	// --- library ---------------------------------------------------------------------

	bool LiquidLibrary::load(const std::string& json_text, std::string* error)
	{
		std::vector<LiquidSpec> parsed;
		if (!parse_document(json_text, parsed, error))
		{
			return false;
		}
		for (LiquidSpec& spec : parsed)
		{
			if (find(spec.id))
			{
				return refuse(error, "duplicate liquid id `" + spec.id + "`");
			}
			liquids_.push_back(std::move(spec));
		}
		return true;
	}

	bool LiquidLibrary::add(const std::string& json_text, std::vector<std::string>* errors)
	{
		std::vector<LiquidSpec> parsed;
		std::string error;
		if (!parse_document(json_text, parsed, &error))
		{
			if (errors)
			{
				errors->push_back(error);
			}
			return false;
		}
		for (const LiquidSpec& spec : parsed)
		{
			if (find(spec.id))
			{
				if (errors)
				{
					errors->push_back("duplicate liquid id `" + spec.id + "`");
				}
				return false;
			}
		}
		for (LiquidSpec& spec : parsed)
		{
			liquids_.push_back(std::move(spec));
		}
		return true;
	}

	int LiquidLibrary::load_directory(const std::string& directory, std::vector<std::string>* errors)
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

			std::vector<LiquidSpec> parsed;
			std::string error;
			if (!parse_document(buffer.str(), parsed, &error))
			{
				if (errors)
				{
					errors->push_back(file.filename().string() + ": " + error);
				}
				continue;
			}
			bool clean = true;
			for (const LiquidSpec& spec : parsed)
			{
				if (find(spec.id))
				{
					if (errors)
					{
						errors->push_back(file.filename().string() + ": duplicate liquid id `"
							+ spec.id + "`");
					}
					clean = false;
					break;
				}
			}
			if (!clean)
			{
				continue;
			}
			for (LiquidSpec& spec : parsed)
			{
				liquids_.push_back(std::move(spec));
			}
			loaded += static_cast<int>(parsed.size());
		}
		return loaded;
	}

	const LiquidSpec* LiquidLibrary::find(const std::string& liquid_id) const
	{
		for (const LiquidSpec& spec : liquids_)
		{
			if (spec.id == liquid_id)
			{
				return &spec;
			}
		}
		return nullptr;
	}

	std::vector<std::string> LiquidLibrary::ids() const
	{
		std::vector<std::string> out;
		out.reserve(liquids_.size());
		for (const LiquidSpec& spec : liquids_)
		{
			out.push_back(spec.id);
		}
		return out;
	}

	// --- glass -----------------------------------------------------------------------

	void LayeredGlass::configure(double capacity_ml, double foam_decay_per_s)
	{
		capacity_ml_ = capacity_ml;
		foam_decay_per_s_ = foam_decay_per_s;
	}

	double LayeredGlass::volume_ml() const
	{
		double total = 0.0;
		for (const LiquidLayer& layer : layers_)
		{
			total += layer.volume_ml;
		}
		return total;
	}

	double LayeredGlass::free_ml() const
	{
		const double taken = volume_ml() + foam_ml_;
		return taken >= capacity_ml_ ? 0.0 : capacity_ml_ - taken;
	}

	double LayeredGlass::pour(const LiquidSpec& spec, double ml, double time_s)
	{
		if (ml <= 0.0)
		{
			return 0.0;
		}
		// Foam shares the capacity with the liquid.
		const double room = free_ml();
		const double taken = ml < room ? ml : room;
		const double spilled = ml - taken;
		if (spilled > 0.0)
		{
			spills_.push_back({ time_s, spilled, spec.id });
		}
		if (taken <= 0.0)
		{
			return 0.0;
		}

		// Merge into an existing stratum of the same liquid...
		for (LiquidLayer& layer : layers_)
		{
			if (layer.liquid_id == spec.id)
			{
				layer.volume_ml += taken;
				foam_ml_ += taken * spec.foam_fraction;
				clip_foam();
				return taken;
			}
		}

		// ...or insert a new stratum by density: heaviest stays at the bottom.
		LiquidLayer layer;
		layer.liquid_id = spec.id;
		layer.density = spec.density;
		layer.volume_ml = taken;
		const auto position = std::find_if(layers_.begin(), layers_.end(),
			[&spec](const LiquidLayer& existing) { return existing.density < spec.density; });
		layers_.insert(position, layer);

		foam_ml_ += taken * spec.foam_fraction;
		clip_foam();
		return taken;
	}

	void LayeredGlass::clip_foam()
	{
		const double room = capacity_ml_ - volume_ml();
		if (foam_ml_ > room)
		{
			foam_ml_ = room > 0.0 ? room : 0.0;
		}
	}

	void LayeredGlass::tick(double dt_s)
	{
		if (dt_s <= 0.0 || foam_ml_ <= 0.0)
		{
			return;
		}
		const double keep = 1.0 - foam_decay_per_s_ * dt_s;
		foam_ml_ = keep <= 0.0 ? 0.0 : foam_ml_ * keep;
		if (foam_ml_ < 1e-6)
		{
			foam_ml_ = 0.0;
		}
	}

} // namespace lc
