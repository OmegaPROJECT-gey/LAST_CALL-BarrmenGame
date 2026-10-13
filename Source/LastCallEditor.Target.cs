// LAST CALL - editor target.

using UnrealBuildTool;
using System.Collections.Generic;

public class LastCallEditorTarget : TargetRules
{
	public LastCallEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LastCall");
		ExtraModuleNames.Add("LastCallTools");
	}
}
