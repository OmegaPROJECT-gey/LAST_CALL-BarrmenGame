// LAST CALL - game target.

using UnrealBuildTool;
using System.Collections.Generic;

public class LastCallTarget : TargetRules
{
	public LastCallTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LastCall");
	}
}
