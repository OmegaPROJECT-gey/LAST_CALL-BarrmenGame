// LAST CALL - the recipe library wrapper. See header for the contract.

#include "Bar/ULCRecipeLibrary.h"

#include "Core/SkillsCore.h"
#include "LastCall.h"
#include "Misc/Paths.h"

namespace
{
	std::string RecipesDirectoryUtf8()
	{
		const FString Directory =
			FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Recipes"));
		FTCHARToUTF8 Converter(*Directory);
		return std::string(Converter.Get(), Converter.Length());
	}

	FString Utf8ToFString(const std::string& Text)
	{
		return FString(UTF8_TO_TCHAR(Text.c_str()));
	}
} // namespace

void ULCRecipeLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadRecipes();
}

int32 ULCRecipeLibrary::LoadRecipes()
{
	lc::RecipeLibrary Fresh;
	std::vector<std::string> Errors;
	const int Loaded = Fresh.load_directory(RecipesDirectoryUtf8(), &Errors);
	for (const std::string& Error : Errors)
	{
		UE_LOG(LogLastCall, Warning, TEXT("Recipe refused: %s"), *Utf8ToFString(Error));
	}

	// The wall must add up to one message or the story cannot read it.
	Fragments.clear();
	std::string WallError;
	if (!lc::collect_secret_fragments(Fresh, Fragments, &WallError))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Wall of recipes refused: %s"),
			*Utf8ToFString(WallError));
		Fragments.clear();
	}

	Library = std::move(Fresh);
	UE_LOG(LogLastCall, Log, TEXT("Recipe book: %d recipes, %d wall fragments"),
		Loaded, static_cast<int32>(Fragments.size()));
	return Loaded;
}

TArray<FString> ULCRecipeLibrary::GetRecipeIds() const
{
	TArray<FString> Ids;
	for (const std::string& Id : Library.ids())
	{
		Ids.Add(Utf8ToFString(Id));
	}
	return Ids;
}

bool ULCRecipeLibrary::HasRecipe(const FString& RecipeId) const
{
	return Library.find(TCHAR_TO_UTF8(*RecipeId)) != nullptr;
}

FString ULCRecipeLibrary::GetNameEn(const FString& RecipeId) const
{
	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	return Record ? Utf8ToFString(Record->name_en) : FString();
}

FString ULCRecipeLibrary::GetNameRu(const FString& RecipeId) const
{
	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	return Record ? Utf8ToFString(Record->name_ru) : FString();
}

FString ULCRecipeLibrary::GetGlass(const FString& RecipeId) const
{
	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	return Record ? Utf8ToFString(Record->glass) : FString();
}

int32 ULCRecipeLibrary::GetStepCount(const FString& RecipeId) const
{
	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	return Record ? static_cast<int32>(Record->steps.size()) : 0;
}

bool ULCRecipeLibrary::GetEffects(const FString& RecipeId, float& MoodBonus,
	float& TipMultiplier, float& XpMultiplier) const
{
	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	if (!Record)
	{
		return false;
	}
	MoodBonus = static_cast<float>(Record->effects.mood_bonus);
	TipMultiplier = static_cast<float>(Record->effects.tip_multiplier);
	XpMultiplier = static_cast<float>(Record->effects.xp_multiplier);
	return true;
}

TArray<FLCWallFragment> ULCRecipeLibrary::GetWallFragments() const
{
	TArray<FLCWallFragment> Result;
	Result.Reserve(static_cast<int32>(Fragments.size()));
	for (const lc::SecretFragment& Fragment : Fragments)
	{
		FLCWallFragment Row;
		Row.Index = Fragment.index;
		Row.Of = Fragment.of;
		Row.RecipeId = Utf8ToFString(Fragment.recipe_id);
		Row.TextKey = Utf8ToFString(Fragment.text_key);
		Row.RevealFlag = Utf8ToFString(Fragment.reveal_flag);
		Result.Add(Row);
	}
	return Result;
}

FLCComboResult ULCRecipeLibrary::EvaluateCombo(const FString& RecipeId,
	const TArray<FLCPourSample>& Samples, float ComboWindowMult) const
{
	FLCComboResult Result;

	const lc::RecipeRecord* Record = Library.find(TCHAR_TO_UTF8(*RecipeId));
	if (!Record)
	{
		Result.Quality = ELCComboQuality::Ruined;
		Result.bWrongIngredient = true;
		return Result;
	}

	std::vector<lc::PourSample> CoreSamples;
	CoreSamples.reserve(Samples.Num());
	for (const FLCPourSample& Sample : Samples)
	{
		lc::PourSample Core;
		Core.time_s = Sample.TimeS;
		Core.flow = Sample.Flow;
		FTCHARToUTF8 Converter(*Sample.Ingredient);
		Core.ingredient.assign(Converter.Get(), Converter.Length());
		CoreSamples.push_back(std::move(Core));
	}

	// A6: the skill multiplier rides into the rating (1.0 = as written).
	lc::SkillSummary Skills;
	Skills.combo_window_mult = ComboWindowMult;
	const lc::RecipeSpec Spec =
		lc::apply_combo_windows(Skills, lc::to_solver_spec(*Record));
	const lc::ComboResult Combo = lc::evaluate_combo(Spec, CoreSamples);
	switch (Combo.quality)
	{
	case lc::ComboQuality::Ruined:   Result.Quality = ELCComboQuality::Ruined;   break;
	case lc::ComboQuality::Fine:     Result.Quality = ELCComboQuality::Fine;     break;
	case lc::ComboQuality::Good:     Result.Quality = ELCComboQuality::Good;     break;
	case lc::ComboQuality::Critical: Result.Quality = ELCComboQuality::Critical; break;
	}
	Result.StepsMatched = Combo.steps_matched;
	Result.bContinuityBroken = Combo.continuity_broken;
	Result.bWrongIngredient = Combo.wrong_ingredient;
	Result.MeanFlowError = static_cast<float>(Combo.mean_flow_error);
	return Result;
}
