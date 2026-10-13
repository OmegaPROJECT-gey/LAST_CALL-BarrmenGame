// LAST CALL - pour solver core implementation. See header for the contract.

#include "Bar/PourSolverCore.h"

#include <algorithm>
#include <cmath>

namespace lc {

	namespace {

		double clamp01(double value)
		{
			return std::max(0.0, std::min(1.0, value));
		}

		// Distance outside [lo, hi]: 0 inside the window.
		double window_error(double flow, double lo, double hi)
		{
			if (flow < lo)
			{
				return lo - flow;
			}
			if (flow > hi)
			{
				return flow - hi;
			}
			return 0.0;
		}

	} // namespace

	double normalized_flow_from_tilt(double tilt_deg, double viscosity, const PourTuning& tuning)
	{
		if (tilt_deg <= tuning.min_angle_deg)
		{
			return 0.0;
		}
		if (viscosity <= 0.0)
		{
			return 0.0;
		}

		const double span = tuning.max_angle_deg - tuning.min_angle_deg;
		if (span <= 0.0)
		{
			return 0.0;
		}

		// Smooth ramp: eased (quadratic) from rest to saturation.
		const double t = std::min(1.0, (tilt_deg - tuning.min_angle_deg) / span);
		const double ramp = t * t * (3.0 - 2.0 * t); // smoothstep
		return clamp01(ramp / viscosity);
	}

	double flow_ml_per_s(double normalized_flow, const PourTuning& tuning)
	{
		return clamp01(normalized_flow) * tuning.base_flow_ml_per_s;
	}

	void integrate(GlassState& glass, double normalized_flow, double dt_s, const PourTuning& tuning)
	{
		if (dt_s <= 0.0)
		{
			return;
		}

		const double incoming = flow_ml_per_s(normalized_flow, tuning) * dt_s;
		const double free_space = glass.capacity_ml - glass.volume_ml;

		if (incoming <= free_space)
		{
			glass.volume_ml += incoming;
			return;
		}

		// The glass is full: everything above the threshold becomes a spill the
		// customer reacts to (GAME-DESIGN 3.1 "Failures").
		glass.volume_ml = glass.capacity_ml;
		const double overflow = incoming - free_space;
		const double spill_share = tuning.overflow_spill_threshold;
		glass.spill_ml += overflow * spill_share;
		if (overflow > 0.0)
		{
			glass.overflowed = true;
		}
	}

	ComboResult evaluate_combo(const RecipeSpec& recipe, const std::vector<PourSample>& samples)
	{
		ComboResult result;

		if (recipe.steps.empty() || samples.empty())
		{
			result.quality = ComboQuality::Ruined;
			return result;
		}

		// Walk the samples step by step: the sequence must match the recipe.
		std::size_t sample_index = 0;
		double total_error = 0.0;
		int matched = 0;
		bool sequence_ok = true;

		for (std::size_t step_index = 0; step_index < recipe.steps.size(); ++step_index)
		{
			const MixStep& step = recipe.steps[step_index];

			// Samples of this step: same ingredient, contiguous in time.
			std::vector<double> flows;
			while (sample_index < samples.size() && samples[sample_index].ingredient == step.ingredient)
			{
				// Continuity against the previous sample of the pour.
				if (sample_index > 0)
				{
					const double gap = samples[sample_index].time_s - samples[sample_index - 1].time_s;
					if (gap > recipe.continuity_break_s)
					{
						result.continuity_broken = true;
					}
				}
				flows.push_back(samples[sample_index].flow);
				++sample_index;
			}

			if (flows.empty())
			{
				sequence_ok = false;
				result.wrong_ingredient = samples[sample_index].ingredient != step.ingredient
					&& sample_index < samples.size();
				break;
			}

			int in_window = 0;
			double step_error = 0.0;
			for (double flow : flows)
			{
				const double err = window_error(flow, step.flow_min, step.flow_max);
				step_error += err;
				if (err == 0.0)
				{
					++in_window;
				}
			}
			const double ratio = static_cast<double>(in_window) / static_cast<double>(flows.size());
			total_error += step_error / static_cast<double>(flows.size());

			if (ratio >= 0.5)
			{
				++matched;
			}
			else
			{
				// Speed far outside the window: the drink is not what was ordered
				// the way it was ordered.
				sequence_ok = false;
				break;
			}
		}

		// Leftover samples mean an unexpected ingredient after the recipe ended.
		if (sequence_ok && sample_index < samples.size())
		{
			sequence_ok = false;
			result.wrong_ingredient = true;
		}

		result.steps_matched = matched;
		result.mean_flow_error = recipe.steps.empty()
			? 0.0
			: total_error / static_cast<double>(recipe.steps.size());

		if (!sequence_ok)
		{
			result.quality = ComboQuality::Ruined;
			return result;
		}

		const double steady = 1.0 - std::min(1.0, result.mean_flow_error);

		if (result.continuity_broken)
		{
			// A pause longer than the recipe allows caps the result.
			result.quality = ComboQuality::Fine;
		}
		else if (steady >= 0.9)
		{
			result.quality = ComboQuality::Critical;
		}
		else if (steady >= 0.75)
		{
			result.quality = ComboQuality::Good;
		}
		else
		{
			result.quality = ComboQuality::Fine;
		}
		return result;
	}

} // namespace lc
