using UnrealBuildTool;

public class AudioVidoLivingCity : ModuleRules
{
    public AudioVidoLivingCity(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore" });
    }
}
