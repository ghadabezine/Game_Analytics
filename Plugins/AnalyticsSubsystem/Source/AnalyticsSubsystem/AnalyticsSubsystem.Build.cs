using UnrealBuildTool;

public class AnalyticsSubsystem : ModuleRules
{
    public AnalyticsSubsystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}