// Copyright 2025 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;

public class OrionCombatEditor : ModuleRules
{
	public OrionCombatEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] 
			{
			}
			);
		
		PrivateIncludePaths.AddRange(
			new string[] 
			{
                "OrionCombatEditor/Private",
                "OrionCombatEditor/Public",
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "OrionRPG",
                "OrionCombat",
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
				"Kismet",
				"AIGraph",
				"EditorStyle",
                "AssetTools",
                "SlateCore",
                "Slate",
                "AssetDefinition",
				"GameplayTags",
				"GameplayAbilities",
				"GameplayAbilitiesEditor",
				"CinematicCamera",
				"LevelEditor",
                "AssetRegistry"
            }
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
   {
				"AssetTools",
				"GraphEditor",
				"PropertyEditor",
				"EditorStyle",
				"Kismet",
				"BlueprintGraph",
				"KismetWidgets",
				"ApplicationCore",
				"ToolMenus",
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"TypedElementFramework",
				"TypedElementRuntime",
				"Slate",
				"SlateCore",
				"AIGraph",
				"UMG",
				"Sequencer",
				"SequencerCore",
                "MovieScene",
				"MovieSceneTools",
				"MovieSceneTracks",
				"LevelSequence",
				"LevelSequenceEditor",
				"AdvancedPreviewScene",
				"EditorWidgets", "Dialog_System_Editor", "Dialog_System_Editor"
   }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
