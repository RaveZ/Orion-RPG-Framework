// Copyright 2025 Ivan Chandra. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OrionRPGFrameworkEditorTarget : TargetRules
{
	public OrionRPGFrameworkEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "OrionRPGFramework" } );
	}
}
