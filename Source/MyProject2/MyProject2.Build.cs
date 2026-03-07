// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyProject2 : ModuleRules
{
	public MyProject2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MyProject2",
			"MyProject2/Variant_Platforming",
			"MyProject2/Variant_Platforming/Animation",
			"MyProject2/Variant_Combat",
			"MyProject2/Variant_Combat/AI",
			"MyProject2/Variant_Combat/Animation",
			"MyProject2/Variant_Combat/Gameplay",
			"MyProject2/Variant_Combat/Interfaces",
			"MyProject2/Variant_Combat/UI",
			"MyProject2/Variant_SideScrolling",
			"MyProject2/Variant_SideScrolling/AI",
			"MyProject2/Variant_SideScrolling/Gameplay",
			"MyProject2/Variant_SideScrolling/Interfaces",
			"MyProject2/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
