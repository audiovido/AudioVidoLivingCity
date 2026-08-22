#include "SAudioVidoAgentPanel.h"

#include "AudioVidoAgentStyle.h"
#include "AudioVidoAgentViewModel.h"
#include "AudioVidoProviderService.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AudioVidoAgentPanel"

namespace
{
const FSlateBrush* Brush(const FName Name) { return FAudioVidoAgentStyle::Get().GetBrush(Name); }
FSlateColor Color(const FName Name) { return FAudioVidoAgentStyle::Get().GetSlateColor(Name); }

FText StateLabel(EAudioVidoState State)
{
    switch (State)
    {
    case EAudioVidoState::Checking: return LOCTEXT("Checking", "Checking");
    case EAudioVidoState::Working: return LOCTEXT("Working", "Working");
    case EAudioVidoState::AwaitingReview: return LOCTEXT("AwaitingReview", "Awaiting review");
    case EAudioVidoState::Applying: return LOCTEXT("Applying", "Applying");
    case EAudioVidoState::Complete: return LOCTEXT("Complete", "Complete");
    case EAudioVidoState::Stopped: return LOCTEXT("Stopped", "Stopped");
    case EAudioVidoState::Offline: return LOCTEXT("Offline", "Offline");
    case EAudioVidoState::Error: return LOCTEXT("Error", "Error");
    default: return LOCTEXT("Idle", "Ready");
    }
}

FText ChangeSummary(const TSharedPtr<FAudioVidoAgentViewModel>& Model)
{
    int32 Added = 0;
    int32 Removed = 0;
    for (const TSharedPtr<FAudioVidoStagedFile>& File : Model->StagedFiles)
    {
        Added += File->AddedLines;
        Removed += File->RemovedLines;
    }
    return FText::Format(LOCTEXT("ChangeSummary", "{0} files  ·  +{1}  −{2}")
        , Model->StagedFiles.Num(), Added, Removed);
}
}

void SAudioVidoAgentPanel::Construct(const FArguments&)
{
    ViewModel = MakeShared<FAudioVidoAgentViewModel>();
    Provider = MakeShared<FAudioVidoProviderService>(ViewModel.ToSharedRef());
    ChangeHandle = ViewModel->OnChanged.AddSP(this, &SAudioVidoAgentPanel::Refresh);

    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(Brush("AudioVido.Background"))
        .Padding(0)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[BuildHeader()]
            + SVerticalBox::Slot().FillHeight(1.f)
            [
                SNew(SSplitter)
                .PhysicalSplitterHandleSize(1.f)
                + SSplitter::Slot().Value(.16f).MinSize(48.f)[BuildSessionRail()]
                + SSplitter::Slot().Value(.49f).MinSize(300.f)[BuildConversation()]
                + SSplitter::Slot().Value(.35f).MinSize(320.f)[BuildReview()]
            ]
            + SVerticalBox::Slot().AutoHeight()[BuildStatusBar()]
        ]
    ];
    Refresh();
    Provider->RefreshLocalModels();
}

SAudioVidoAgentPanel::~SAudioVidoAgentPanel()
{
    if (Provider.IsValid()) Provider->Cancel();
    if (ViewModel.IsValid()) ViewModel->OnChanged.Remove(ChangeHandle);
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildHeader()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Surface")).Padding(FMargin(16, 8))
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [SNew(STextBlock).Text(LOCTEXT("Mark", "◆")).ColorAndOpacity(Color("AudioVido.Cyan")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 24, 0).VAlign(VAlign_Center)
        [SNew(STextBlock).Text(LOCTEXT("Title", "AudioVido")).ColorAndOpacity(Color("AudioVido.Text")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [SNew(STextBlock).Text_Lambda([] { return FText::Format(LOCTEXT("Project", "Project: {0}"), FText::FromString(FApp::GetProjectName())); }).ColorAndOpacity(Color("AudioVido.TextMuted"))]
        + SHorizontalBox::Slot().FillWidth(1.f)
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
        [SNew(SBorder).BorderImage(Brush("AudioVido.Elevated")).Padding(FMargin(8, 3))
            .Visibility_Lambda([this] { return ViewModel->StagedFiles.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
            [SNew(STextBlock).Text_Lambda([this] { return FText::Format(LOCTEXT("StagedBadge", "{0} staged"), ViewModel->StagedFiles.Num()); })
                .ColorAndOpacity(Color("AudioVido.Warning"))]]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
        [SNew(STextBlock).Text_Lambda([this] { return FText::FromString(ViewModel->ModelId); }).ColorAndOpacity(Color("AudioVido.TextMuted"))]
        + SHorizontalBox::Slot().AutoWidth().Padding(12, 0).VAlign(VAlign_Center)
        [SNew(STextBlock).Text_Lambda([this]
            {
                const bool bNeedsKey = !ViewModel->bUseOllama && ViewModel->ApiKey.IsEmpty();
                return FText::Format(LOCTEXT("Connection", "● {0}"), bNeedsKey ? LOCTEXT("NeedsKey", "Setup required") : StateLabel(ViewModel->State));
            })
            .ColorAndOpacity_Lambda([this] { return ViewModel->State == EAudioVidoState::Offline || ViewModel->State == EAudioVidoState::Error ? Color("AudioVido.Error") : Color("AudioVido.Success"); })]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
        [SNew(SButton).ToolTipText(LOCTEXT("SettingsTip", "Provider, model, and safety settings")).OnClicked_Lambda([this] { OpenSettings(); return FReply::Handled(); })
            [SNew(STextBlock).Text(LOCTEXT("Settings", "Settings"))]]
    ];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildSessionRail()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Surface")).Padding(12)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SButton).ButtonColorAndOpacity(Color("AudioVido.Cyan")).OnClicked_Lambda([this]
        {
            if (!ViewModel->StagedFiles.IsEmpty())
            {
                ViewModel->NewTask();
            }
            else ViewModel->NewTask();
            return FReply::Handled();
        })[SNew(STextBlock).Text(LOCTEXT("NewTask", "+ New task"))]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 6)
        [SNew(STextBlock).Text(LOCTEXT("Today", "TODAY")).ColorAndOpacity(Color("AudioVido.TextMuted"))]
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SBorder).BorderImage(Brush("AudioVido.Elevated")).Padding(10)
            [SNew(STextBlock).Text(LOCTEXT("CurrentSession", "Current task\nLocal editor session")).ColorAndOpacity(Color("AudioVido.Text"))]]
        + SVerticalBox::Slot().FillHeight(1.f)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
        [SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("LocalSessions", "Sessions remain local. Staged changes are preserved when starting a new task.")).
            ColorAndOpacity(Color("AudioVido.TextMuted"))]
    ];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildConversation()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Background")).Padding(16)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
        [SNew(STextBlock).Text(LOCTEXT("Conversation", "Conversation")).Font(FAppStyle::GetFontStyle("HeadingSmall")).ColorAndOpacity(Color("AudioVido.Text"))]
        + SVerticalBox::Slot().FillHeight(1.f)
        [SAssignNew(ConversationScroll, SScrollBox)
            + SScrollBox::Slot()[SAssignNew(Transcript, SVerticalBox)]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)[BuildComposer()]
    ];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildComposer()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Elevated")).Padding(10)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SBox).MinDesiredHeight(56).MaxDesiredHeight(160)
            [SAssignNew(Composer, SMultiLineEditableTextBox)
                .HintText(LOCTEXT("ComposerHint", "Ask AudioVido to change this project..."))
                .AutoWrapText(true)
                .OnKeyDownHandler(this, &SAudioVidoAgentPanel::OnComposerKeyDown)]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [SNew(STextBlock).Text(LOCTEXT("Scope", "+ Context: Source · Config · Plugins")).ColorAndOpacity(Color("AudioVido.TextMuted"))]
            + SHorizontalBox::Slot().FillWidth(1.f)
            + SHorizontalBox::Slot().AutoWidth()
            [SNew(SButton).IsEnabled_Lambda([this] { return !Composer->GetText().IsEmpty() || Provider->IsBusy(); })
                .OnClicked_Lambda([this] { if (Provider->IsBusy()) Provider->Cancel(); else SendPrompt(); return FReply::Handled(); })
                [SNew(STextBlock).Text_Lambda([this] { return Provider->IsBusy() ? LOCTEXT("Stop", "Stop") : LOCTEXT("Send", "Send →"); })]]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
        [SNew(STextBlock).Text(LOCTEXT("Keys", "Enter to send · Shift+Enter for a new line")).ColorAndOpacity(Color("AudioVido.TextMuted"))]
    ];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildReview()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Surface")).Padding(12)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[SNew(STextBlock).Text_Lambda([this] { return FText::Format(LOCTEXT("ChangesCount", "Changes  {0}"), ViewModel->StagedFiles.Num()); }).Font(FAppStyle::GetFontStyle("HeadingSmall"))]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SCheckBox).ToolTipText(LOCTEXT("SelectAllTip", "Select all staged files")).
                IsChecked_Lambda([this] { return !ViewModel->StagedFiles.IsEmpty() && ViewModel->GetSelectedCount() == ViewModel->StagedFiles.Num() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { ViewModel->SelectAll(State == ECheckBoxState::Checked); })[SNew(STextBlock).Text(LOCTEXT("SelectAll", "Select all"))]]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
        [SNew(STextBlock).Text_Lambda([this] { return ChangeSummary(ViewModel); }).ColorAndOpacity(Color("AudioVido.TextMuted"))]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 10)
        [SNew(SBox).MaxDesiredHeight(190)[SNew(SScrollBox)+SScrollBox::Slot()[SAssignNew(FileRows, SVerticalBox)]]]
        + SVerticalBox::Slot().FillHeight(1.f)
        [SNew(SBorder).BorderImage(Brush("AudioVido.Background")).Padding(8)
            [SAssignNew(DiffPreview, SMultiLineEditableTextBox).IsReadOnly(true).AlwaysShowScrollbars(true)
                .HintText(LOCTEXT("DiffHint", "Select a staged file to inspect its real unified diff."))]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text_Lambda([this] { return FText::Format(LOCTEXT("SelectedCount", "{0} selected"), ViewModel->GetSelectedCount()); }).ColorAndOpacity(Color("AudioVido.TextMuted"))]
            + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
            [SNew(SButton).IsEnabled_Lambda([this] { return ViewModel->GetSelectedCount() > 0 && !Provider->IsBusy(); }).OnClicked(this, &SAudioVidoAgentPanel::DiscardSelected)
                [SNew(STextBlock).Text(LOCTEXT("Discard", "Discard selected"))]]
            + SHorizontalBox::Slot().AutoWidth()
            [SNew(SButton).ButtonColorAndOpacity(Color("AudioVido.Cyan")).IsEnabled_Lambda([this] { return ViewModel->GetSelectedCount() > 0 && !Provider->IsBusy(); }).OnClicked(this, &SAudioVidoAgentPanel::ApplySelected)
                [SNew(STextBlock).Text(LOCTEXT("Apply", "Apply selected"))]]]
    ];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildStatusBar()
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Surface")).Padding(FMargin(12, 5))
    [SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(LOCTEXT("ReviewRequired", "▣ Review required")).ColorAndOpacity(Color("AudioVido.Cyan"))]
        + SHorizontalBox::Slot().AutoWidth().Padding(20, 0)[SNew(STextBlock).Text(LOCTEXT("Allowed", "Source · Config · Plugins only")).ColorAndOpacity(Color("AudioVido.TextMuted"))]
        + SHorizontalBox::Slot().FillWidth(1.f)
        + SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text_Lambda([this] { return FText::FromString(ViewModel->StatusDetail); }).ColorAndOpacity(Color("AudioVido.TextMuted"))]];
}

TSharedRef<SWidget> SAudioVidoAgentPanel::BuildFileRow(TSharedPtr<FAudioVidoStagedFile> File)
{
    return SNew(SBorder).BorderImage(Brush("AudioVido.Elevated")).Padding(7)
    [SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SCheckBox).IsChecked_Lambda([File] { return File->bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this, File](ECheckBoxState State) { File->bSelected = State == ECheckBoxState::Checked; ViewModel->OnChanged.Broadcast(); })]
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(6, 0)
        [SNew(SButton).ButtonStyle(FAppStyle::Get(), "SimpleButton").OnClicked_Lambda([this, File] { SelectFile(File); return FReply::Handled(); })
            [SNew(STextBlock).Text(FText::FromString(File->RelativePath)).ToolTipText(FText::FromString(File->RelativePath))]]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [SNew(STextBlock)
            .Text_Lambda([File] { return !File->Error.IsEmpty() ? LOCTEXT("FileError", "Error") : (File->bCreated ? LOCTEXT("Created", "Created") : LOCTEXT("Modified", "Modified")); })
            .ToolTipText_Lambda([File] { return File->Error.IsEmpty() ? FText::GetEmpty() : FText::FromString(File->Error); })
            .ColorAndOpacity_Lambda([File] { return !File->Error.IsEmpty() ? Color("AudioVido.Error") : (File->bCreated ? Color("AudioVido.Success") : Color("AudioVido.Warning")); })]];
}

void SAudioVidoAgentPanel::Refresh()
{
    if (!Transcript.IsValid() || !FileRows.IsValid()) return;
    Transcript->ClearChildren();
    if (ViewModel->Messages.IsEmpty())
    {
        Transcript->AddSlot().Padding(24)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[SNew(STextBlock).Text(LOCTEXT("WelcomeMark", "◆")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22)).ColorAndOpacity(Color("AudioVido.Cyan"))]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12).HAlign(HAlign_Center)[SNew(STextBlock).Text(LOCTEXT("Welcome", "What do you want to build?")).
                AutoWrapText(true).Justification(ETextJustify::Center).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12).HAlign(HAlign_Center)[SNew(STextBlock).Text(LOCTEXT("WelcomeBody", "Describe a small, reviewable change to this Unreal project.")).
                AutoWrapText(true).Justification(ETextJustify::Center).ColorAndOpacity(Color("AudioVido.TextMuted"))]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 4).HAlign(HAlign_Center)[SNew(SButton).OnClicked_Lambda([this] { Composer->SetText(LOCTEXT("ExamplePrompt", "Add an editable rotating actor component")); return FReply::Handled(); })
                [SNew(STextBlock).Text(LOCTEXT("Example", "Add an editable rotating actor component")).Justification(ETextJustify::Center)]]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4).HAlign(HAlign_Center)[SNew(SButton).OnClicked_Lambda([this] { Composer->SetText(LOCTEXT("ExplainPrompt", "Explain this module before changing it")); return FReply::Handled(); })
                [SNew(STextBlock).Text(LOCTEXT("ExplainExample", "Explain this module before changing it")).Justification(ETextJustify::Center)]]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14).HAlign(HAlign_Center)[SNew(SButton).OnClicked_Lambda([this] { Composer->SetText(LOCTEXT("RefactorPrompt", "Refactor the selected class safely")); return FReply::Handled(); })
                [SNew(STextBlock).Text(LOCTEXT("RefactorExample", "Refactor the selected class safely")).Justification(ETextJustify::Center)]]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)[SNew(STextBlock).Text(LOCTEXT("Security", "▣ Reads stay inside this project. Proposed writes require your approval.")).
                AutoWrapText(true).Justification(ETextJustify::Center).ColorAndOpacity(Color("AudioVido.TextMuted"))]];
    }
    for (const FAudioVidoMessage& Message : ViewModel->Messages)
    {
        Transcript->AddSlot().Padding(0, 4)[SNew(SBorder).BorderImage(Message.bFromUser ? Brush("AudioVido.Elevated") : Brush("AudioVido.Background")).Padding(10)
            [SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f)[SNew(STextBlock).Text(Message.bFromUser ? LOCTEXT("You", "You") : LOCTEXT("Agent", "◆ AudioVido")).ColorAndOpacity(Message.bFromUser ? Color("AudioVido.TextMuted") : Color("AudioVido.Cyan")).Justification(ETextJustify::Center)]
                    + SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::FromString(Message.Timestamp.ToString(TEXT("%H:%M")))).ColorAndOpacity(Color("AudioVido.TextMuted")).Justification(ETextJustify::Center)]]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 5)[SNew(STextBlock).Text(Message.Text).AutoWrapText(true).Justification(ETextJustify::Center)]]];
    }
    if (Provider->IsBusy())
    {
        Transcript->AddSlot().Padding(0, 8)[SNew(SBorder).BorderImage(Brush("AudioVido.Elevated")).Padding(10)
            [SNew(STextBlock).Text_Lambda([this] { return FText::Format(LOCTEXT("WorkingActivity", "◆  AudioVido is working…\n{0}"), FText::FromString(ViewModel->StatusDetail)); })
                .AutoWrapText(true).ColorAndOpacity(Color("AudioVido.Cyan")).Justification(ETextJustify::Center)]];
    }
    FileRows->ClearChildren();
    for (const auto& File : ViewModel->StagedFiles) FileRows->AddSlot().Padding(0, 2)[BuildFileRow(File)];
    if (SelectedFile.IsValid() && !ViewModel->StagedFiles.Contains(SelectedFile))
    {
        SelectedFile.Reset(); DiffPreview->SetText(FText::GetEmpty());
    }
    Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SAudioVidoAgentPanel::SelectFile(TSharedPtr<FAudioVidoStagedFile> File)
{
    SelectedFile = File;
    DiffPreview->SetText(FText::FromString(ViewModel->BuildUnifiedDiff(*File)));
}

void SAudioVidoAgentPanel::SendPrompt()
{
    const FText Prompt = Composer->GetText();
    if (Prompt.IsEmpty()) return;
    if (!ViewModel->bUseOllama && ViewModel->ApiKey.IsEmpty()) { OpenSettings(); return; }
    ViewModel->AddMessage(true, Prompt);
    Provider->Send(Prompt.ToString());
}

FReply SAudioVidoAgentPanel::OnComposerKeyDown(const FGeometry&, const FKeyEvent& KeyEvent)
{
    if (KeyEvent.IsControlDown() && KeyEvent.GetKey() == EKeys::L)
    {
        FSlateApplication::Get().SetKeyboardFocus(Composer, EFocusCause::Navigation);
        return FReply::Handled();
    }
    if (KeyEvent.IsControlDown() && KeyEvent.GetKey() == EKeys::N)
    {
        ViewModel->NewTask();
        Composer->SetText(FText::GetEmpty());
        return FReply::Handled();
    }
    if (KeyEvent.GetKey() == EKeys::Escape)
    {
        if (Provider->IsBusy()) Provider->Cancel();
        else FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
        return FReply::Handled();
    }
    if (KeyEvent.GetKey() == EKeys::Enter && !KeyEvent.IsShiftDown())
    {
        SendPrompt();
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

void SAudioVidoAgentPanel::OpenSettings()
{
    TSharedRef<SWindow> Window = SNew(SWindow).Title(LOCTEXT("SettingsTitle", "AudioVido Settings")).ClientSize(FVector2D(620, 420)).SupportsMaximize(false).SupportsMinimize(false);
    TSharedPtr<SEditableTextBox> KeyBox;
    TSharedPtr<SEditableTextBox> ModelBox;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> ModelCombo;
    TSharedPtr<FString> CurrentModel = ViewModel->AvailableModels.FindByPredicate(
        [this](const TSharedPtr<FString>& Item) { return Item.IsValid() && *Item == ViewModel->ModelId; })
        ? *ViewModel->AvailableModels.FindByPredicate([this](const TSharedPtr<FString>& Item) { return Item.IsValid() && *Item == ViewModel->ModelId; })
        : nullptr;
    Window->SetContent(SNew(SBorder).BorderImage(Brush("AudioVido.Background")).Padding(20)
    [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text_Lambda([this] { return ViewModel->bUseOllama ? LOCTEXT("OllamaHeading", "Provider · Ollama (local)") : LOCTEXT("ProviderHeading", "Provider · OpenRouter"); }).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 4)
        [SNew(SCheckBox).IsChecked(ViewModel->bUseOllama ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { ViewModel->bUseOllama = State == ECheckBoxState::Checked; })
            [SNew(STextBlock).Text(LOCTEXT("UseOllama", "Use local Ollama at 127.0.0.1:11434")).Justification(ETextJustify::Center)]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 5)[SNew(STextBlock).Text(LOCTEXT("KeyLabel", "Session API key")).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().AutoHeight()[SAssignNew(KeyBox, SEditableTextBox).IsPassword(true).Text(FText::FromString(ViewModel->ApiKey)).IsEnabled_Lambda([this] { return !ViewModel->bUseOllama; }).HintText(LOCTEXT("KeyHint", "Only needed for OpenRouter; stored in memory for this session")).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 5)[SNew(STextBlock).Text(LOCTEXT("ModelLabel", "Model ID")).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().AutoHeight()[SAssignNew(ModelBox, SEditableTextBox).Text(FText::FromString(ViewModel->ModelId)).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
        [SAssignNew(ModelCombo, SComboBox<TSharedPtr<FString>>)
            .OptionsSource(&ViewModel->AvailableModels)
            .InitiallySelectedItem(CurrentModel)
            .OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) { return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : FString())).Justification(ETextJustify::Center); })
            .OnSelectionChanged_Lambda([ModelBox](TSharedPtr<FString> Item, ESelectInfo::Type) { if (Item.IsValid()) ModelBox->SetText(FText::FromString(*Item)); })
            [SNew(STextBlock).Text(LOCTEXT("ChooseLocalModel", "Choose an installed local model...")).Justification(ETextJustify::Center)]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
        [SNew(SButton).OnClicked_Lambda([this, ModelCombo] { Provider->RefreshLocalModels(); ModelCombo->RefreshOptions(); return FReply::Handled(); })
            [SNew(STextBlock).Text(LOCTEXT("TestConnection", "Test connection and refresh models")).Justification(ETextJustify::Center)]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 24, 0, 5)[SNew(STextBlock).Text(LOCTEXT("SafetyHeading", "Safety boundary")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("SafetyText", "AudioVido can propose complete text writes under Source, Config, and Plugins. Every write remains staged until you explicitly apply it. It cannot delete files, execute commands, mutate assets, or access paths outside this project.")).Justification(ETextJustify::Center)]
        + SVerticalBox::Slot().FillHeight(1.f)
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[SNew(SButton).OnClicked_Lambda([this, Window] { ViewModel->ApiKey.Empty(); ViewModel->SetState(EAudioVidoState::Idle, TEXT("Session key forgotten")); Window->RequestDestroyWindow(); return FReply::Handled(); })[SNew(STextBlock).Text(LOCTEXT("Forget", "Forget key")).Justification(ETextJustify::Center)]]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).ButtonColorAndOpacity(Color("AudioVido.Cyan")).OnClicked_Lambda([this, Window, KeyBox, ModelBox]
            {
                ViewModel->ApiKey = KeyBox->GetText().ToString().TrimStartAndEnd();
                const FString CandidateModel = ModelBox->GetText().ToString().TrimStartAndEnd();
                if (!CandidateModel.IsEmpty()) ViewModel->ModelId = CandidateModel;
                ViewModel->SetState(EAudioVidoState::Idle, (!ViewModel->bUseOllama && ViewModel->ApiKey.IsEmpty()) ? TEXT("An OpenRouter session key is required before sending") : TEXT("Provider settings updated for this session"));
                Window->RequestDestroyWindow(); return FReply::Handled();
            })[SNew(STextBlock).Text(LOCTEXT("Continue", "Continue")).Justification(ETextJustify::Center)]]]]);
    FSlateApplication::Get().AddModalWindow(Window, SharedThis(this));
}

FReply SAudioVidoAgentPanel::ApplySelected()
{
    FString Paths;
    for (const auto& File : ViewModel->StagedFiles) if (File->bSelected) Paths += TEXT("\n• ") + File->RelativePath;
    const FText Prompt = FText::Format(LOCTEXT("ApplyConfirm", "Apply these project-relative text files?{0}\n\nExisting files are backed up before atomic replacement. No unselected file will be changed."), FText::FromString(Paths));
    if (FMessageDialog::Open(EAppMsgType::OkCancel, Prompt, LOCTEXT("ApplyTitle", "Apply selected changes")) == EAppReturnType::Ok)
    {
        TArray<FString> Applied, Failed; ViewModel->ApplySelected(Applied, Failed);
    }
    return FReply::Handled();
}

FReply SAudioVidoAgentPanel::DiscardSelected()
{
    if (FMessageDialog::Open(EAppMsgType::OkCancel,
        FText::Format(LOCTEXT("DiscardConfirm", "Discard {0} selected staged file(s)? No project file will be changed."), ViewModel->GetSelectedCount()),
        LOCTEXT("DiscardTitle", "Discard staged changes")) == EAppReturnType::Ok) ViewModel->DiscardSelected();
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
