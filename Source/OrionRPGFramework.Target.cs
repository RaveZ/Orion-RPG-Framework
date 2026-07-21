// Copyright 2025 Ivan Chandra. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OrionRPGFrameworkTarget : TargetRules
{
	public OrionRPGFrameworkTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6; 
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.AddRange( new string[] { "OrionRPGFramework" } );
	}
}
