using UnrealBuildTool;

public class AudioVidoAgent : ModuleRules
{
    public AudioVidoAgent(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "Slate", "SlateCore",
            "InputCore", "UnrealEd", "ToolMenus", "Projects", "HTTP",
            "Json", "ApplicationCore"
        });
    }
}
