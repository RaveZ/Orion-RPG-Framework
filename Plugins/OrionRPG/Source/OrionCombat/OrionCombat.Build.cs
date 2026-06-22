// Copyright 2025 Ivan Chandra. All Rights Reserved.

using UnrealBuildTool;

public class OrionCombat : ModuleRules
{
	public OrionCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"OrionCombat/Private",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"AIModule",
				"PhysicsCore",
				"GameplayAbilities", 
				"AnimGraphRuntime",
				"CoreUObject",
				"InputCore",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore", 
				"EnhancedInput", 
				"GameplayTags", 
				"GameplayTasks",
                "NavigationSystem",
				"Niagara", 
				"OrionRPG",
				"GameplayStateTreeModule"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
