#pragma once

#include "CoreMinimal.h"

enum class EAudioVidoState : uint8
{
    Idle, Checking, Working, AwaitingReview, Applying, Complete, Stopped, Offline, Error
};

struct FAudioVidoMessage
{
    bool bFromUser = false;
    FText Text;
    FDateTime Timestamp = FDateTime::Now();
};

struct FAudioVidoStagedFile
{
    FString RelativePath;
    FString OriginalContent;
    FString StagedContent;
    FString Error;
    bool bSelected = true;
    bool bCreated = false;
    int32 AddedLines = 0;
    int32 RemovedLines = 0;
};

DECLARE_MULTICAST_DELEGATE(FOnAudioVidoStateChanged);

class FAudioVidoAgentViewModel : public TSharedFromThis<FAudioVidoAgentViewModel>
{
public:
    EAudioVidoState State = EAudioVidoState::Idle;
    FString StatusDetail = TEXT("Ready for a new task");
    FString ApiKey;
    bool bUseOllama = true;
    FString ModelId = TEXT("qwen2.5-coder:14b");
    TArray<TSharedPtr<FString>> AvailableModels;
    TArray<FAudioVidoMessage> Messages;
    TArray<TSharedPtr<FAudioVidoStagedFile>> StagedFiles;
    FOnAudioVidoStateChanged OnChanged;

    void SetState(EAudioVidoState NewState, const FString& Detail);
    void AddMessage(bool bFromUser, const FText& Text);
    bool StageWrite(const FString& RelativePath, const FString& Content, FString& OutError);
    bool ValidateRelativePath(const FString& RelativePath, FString& OutAbsolutePath, FString& OutError) const;
    bool ListApprovedFiles(const FString& RelativeRoot, TArray<FString>& OutFiles, FString& OutError) const;
    bool ReadApprovedTextFile(const FString& RelativePath, FString& OutContent, FString& OutError) const;
    FString BuildUnifiedDiff(const FAudioVidoStagedFile& File) const;
    int32 GetSelectedCount() const;
    void SelectAll(bool bSelected);
    void DiscardSelected();
    bool ApplySelected(TArray<FString>& OutApplied, TArray<FString>& OutFailed);
    void NewTask();
};
