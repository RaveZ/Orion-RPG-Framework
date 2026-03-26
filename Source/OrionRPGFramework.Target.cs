using UnrealBuildTool;
using System.Collections.Generic;

public class OrionRPGFrameworkTarget : TargetRules
{
    public OrionRPGFrameworkTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        // Suppress the upgrade warnings by matching the engine defaults
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

        // Optionally isolate this target's environment if you must change global rules
        // BuildEnvironment = TargetBuildEnvironment.Unique;

        ExtraModuleNames.AddRange(new string[] { "OrionRPGFramework" });
    }
}