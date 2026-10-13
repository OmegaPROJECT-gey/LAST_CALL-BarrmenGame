// LAST CALL - game target.

using UnrealBuildTool;
using System.Collections.Generic;

public class LastCallTarget : TargetRules
{
	public LastCallTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LastCall");
	}
}
