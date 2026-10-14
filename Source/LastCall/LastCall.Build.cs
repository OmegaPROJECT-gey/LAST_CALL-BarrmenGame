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

		// Subsystem folders (Core/, Bar/, Dialogue/, ...) are included as
		// "Core/Foo.h" everywhere - the module root must be on the path.
		PublicIncludePaths.Add(ModuleDirectory);
	}
}
