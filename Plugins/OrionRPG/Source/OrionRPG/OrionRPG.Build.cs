// Copyright 2025 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;
using EpicGames.Core;  // IMPORTANT - CONTAINS JsonObject 

public class OrionRPG : ModuleRules
{
    public OrionRPG(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;

        PublicIncludePaths.AddRange(
            new string[] {
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                "OrionRPG/Private",
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "CommonUI",
                "UMG"
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "GameplayTags",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
			}
            );

        // Add pre-processor macros for the OrionRPGFramework plugin based on enabled state (optional plugin)
        PublicDefinitions.Add("WITH_ORION_COMBAT_FRAMEWORK=0");
        if (JsonObject.TryRead(Target.ProjectFile, out var rawObject))
        {
            if (rawObject.TryGetObjectArrayField("Plugins", out var pluginObjects))
            {
                foreach (JsonObject pluginObject in pluginObjects)
                {
                    pluginObject.TryGetStringField("Name", out var pluginName);

                    pluginObject.TryGetBoolField("Enabled", out var pluginEnabled);

                    if (pluginName == "OrionCombatFramework" && pluginEnabled)
                    {
                        PrivateDependencyModuleNames.Add("OrionCombatFramework");
                        PublicDefinitions.Add("WITH_ORION_COMBAT_FRAMEWORK=1");
                        PublicDefinitions.Remove("WITH_ORION_COMBAT_FRAMEWORK=0");
                    }
                }
            }
        }

    }
}