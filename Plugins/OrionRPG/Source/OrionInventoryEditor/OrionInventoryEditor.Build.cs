// Copyright 2025 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;

public class OrionInventoryEditor : ModuleRules
{
	public OrionInventoryEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "OrionInventoryEditor/Private",
                "OrionInventoryEditor/Public",
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "OrionRPG",
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
				"KismetWidgets", 
				"ApplicationCore", 
				"ToolMenus", 
				"Projects", 
				"InputCore", 
				"UnrealEd", 
				"ToolMenus", 
				"CoreUObject", 
				"Engine", 
				"Slate", 
				"SlateCore", 
				"AIGraph",
				"UMG",
   }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
