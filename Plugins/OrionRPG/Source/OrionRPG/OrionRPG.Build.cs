// Copyright 2025 Ivan Chandra. All Rights Reserved.
using EpicGames.Core;  // IMPORTANT - CONTAINS JsonObject 
using UnrealBuildTool;

public class OrionRPG : ModuleRules
{
    public OrionRPG(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

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
                "CommonUI",
                "UMG",
                "CommonInput",
                "EnhancedInput",
                "InputCore",
                "DeveloperSettings",
                "LevelSequence",
                "MovieScene",
                "CinematicCamera",
                "OrionCombat"

            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "GameplayTags",
                "MovieSceneTracks",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
			}
            );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("EditorStyle");
        }
    }
}