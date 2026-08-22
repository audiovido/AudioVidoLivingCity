#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FAudioVidoAgentViewModel;
class FAudioVidoProviderService;
class SMultiLineEditableTextBox;
class SScrollBox;
class SVerticalBox;
struct FAudioVidoStagedFile;

class SAudioVidoAgentPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAudioVidoAgentPanel) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& InArgs);
    virtual ~SAudioVidoAgentPanel() override;

private:
    TSharedRef<SWidget> BuildHeader();
    TSharedRef<SWidget> BuildSessionRail();
    TSharedRef<SWidget> BuildConversation();
    TSharedRef<SWidget> BuildReview();
    TSharedRef<SWidget> BuildComposer();
    TSharedRef<SWidget> BuildStatusBar();
    TSharedRef<SWidget> BuildFileRow(TSharedPtr<FAudioVidoStagedFile> File);
    void Refresh();
    void OpenSettings();
    void SendPrompt();
    FReply OnComposerKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent);
    FReply ApplySelected();
    FReply DiscardSelected();
    void SelectFile(TSharedPtr<FAudioVidoStagedFile> File);

    TSharedPtr<FAudioVidoAgentViewModel> ViewModel;
    TSharedPtr<FAudioVidoProviderService> Provider;
    TSharedPtr<SMultiLineEditableTextBox> Composer;
    TSharedPtr<SVerticalBox> Transcript;
    TSharedPtr<SVerticalBox> FileRows;
    TSharedPtr<SMultiLineEditableTextBox> DiffPreview;
    TSharedPtr<SScrollBox> ConversationScroll;
    TSharedPtr<FAudioVidoStagedFile> SelectedFile;
    FDelegateHandle ChangeHandle;
};