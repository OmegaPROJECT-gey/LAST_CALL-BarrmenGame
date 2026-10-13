// LAST CALL - editor-only tool module (import, validate, cook helpers).
// Everything that belongs outside the shipped runtime lives here.

using UnrealBuildTool;

public class LastCallTools : ModuleRules
{
	public LastCallTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"LastCall"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Json",
			"JsonUtilities"
		});
	}
}
