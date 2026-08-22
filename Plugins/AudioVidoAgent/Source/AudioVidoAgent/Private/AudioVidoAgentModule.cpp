#include "AudioVidoAgentStyle.h"
#include "SAudioVidoAgentPanel.h"

#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "AudioVidoAgentModule"

class FAudioVidoAgentModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FAudioVidoAgentStyle::Initialize();
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName,
            FOnSpawnTab::CreateRaw(this, &FAudioVidoAgentModule::SpawnTab))
            .SetDisplayName(LOCTEXT("TabTitle", "AudioVido Agent"))
            .SetTooltipText(LOCTEXT("TabTooltip", "Open the review-first AudioVido development workspace"))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAudioVidoAgentModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
        FAudioVidoAgentStyle::Shutdown();
    }

private:
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SAudioVidoAgentPanel)];
    }

    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
        FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("WindowLayout"));
        Section.AddMenuEntry(TEXT("OpenAudioVidoAgent"), LOCTEXT("MenuLabel", "AudioVido Agent"),
            LOCTEXT("MenuTooltip", "Open AudioVido Agent"), FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([] { FGlobalTabmanager::Get()->TryInvokeTab(TabName); })));
    }

    static const FName TabName;
};

const FName FAudioVidoAgentModule::TabName(TEXT("AudioVidoAgent"));
IMPLEMENT_MODULE(FAudioVidoAgentModule, AudioVidoAgent)

#undef LOCTEXT_NAMESPACE
