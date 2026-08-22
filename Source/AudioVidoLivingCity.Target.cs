using UnrealBuildTool;

public class AudioVidoLivingCityTarget : TargetRules
{
    public AudioVidoLivingCityTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("AudioVidoLivingCity");
    }
}
