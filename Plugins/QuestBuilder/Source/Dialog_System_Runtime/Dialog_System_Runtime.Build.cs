// Copyright 2025 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;
using EpicGames.Core;  // IMPORTANT - CONTAINS JsonObject 

public class Dialog_System_Runtime : ModuleRules
{
    public Dialog_System_Runtime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        ShadowVariableWarningLevel = WarningLevel.Error;

        PublicIncludePaths.AddRange(
            new string[] {
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                "Dialog_System_Runtime/Private",
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Quest_System_Runtime",
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

    }
}