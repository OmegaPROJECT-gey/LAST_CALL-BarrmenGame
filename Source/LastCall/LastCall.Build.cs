// LAST CALL - game runtime module.
// Architecture: docs/ARCHITECTURE.md (EN) / docs/ARCHITECTURE.ru.md (RU)
// License: MIT for code (LICENSE); game content proprietary.

using UnrealBuildTool;

public class LastCall : ModuleRules
{
	public LastCall(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UMG",
			"Niagara"
		});

		// Data-driven content lives outside Content/ and is loaded through
		// DataTables compiled from Data/*.json (ARCHITECTURE.md 7).
		PublicIncludePaths.AddRange(new string[]
		{
			// keep module root include clean; subsystem folders are private
		});
	}
}
