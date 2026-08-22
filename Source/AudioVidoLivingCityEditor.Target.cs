using UnrealBuildTool;

public class AudioVidoLivingCityEditorTarget : TargetRules
{
    public AudioVidoLivingCityEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("AudioVidoLivingCity");
    }
}
