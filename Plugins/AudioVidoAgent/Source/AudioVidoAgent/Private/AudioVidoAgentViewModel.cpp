#include "AudioVidoAgentViewModel.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
constexpr int64 MaxWriteBytes = 2 * 1024 * 1024;
constexpr int64 MaxReadBytes = 512 * 1024;
constexpr int32 MaxListedFiles = 300;

bool IsAllowedExtension(const FString& Extension)
{
    static const TSet<FString> Allowed = {
        TEXT("h"), TEXT("hpp"), TEXT("cpp"), TEXT("c"), TEXT("cs"),
        TEXT("ini"), TEXT("json"), TEXT("uplugin"), TEXT("uproject"),
        TEXT("txt"), TEXT("md"), TEXT("py"), TEXT("usf"), TEXT("ush")
    };
    return Allowed.Contains(Extension.ToLower());
}

int32 CountLines(const FString& Value)
{
    if (Value.IsEmpty()) return 0;
    int32 Count = 1;
    for (const TCHAR Character : Value)
    {
        Count += Character == TEXT('\n') ? 1 : 0;
    }
    return Count;
}

struct FLcsMatch
{
    int32 OldIndex;
    int32 NewIndex;
};

TArray<int32> LcsForward(const TArray<FString>& OldLines, int32 OldBegin, int32 OldEnd,
    const TArray<FString>& NewLines, int32 NewBegin, int32 NewEnd)
{
    const int32 NewCount = NewEnd - NewBegin;
    TArray<int32> Previous, Current;
    Previous.Init(0, NewCount + 1);
    Current.Init(0, NewCount + 1);
    for (int32 OldIndex = OldBegin; OldIndex < OldEnd; ++OldIndex)
    {
        for (int32 Offset = 1; Offset <= NewCount; ++Offset)
        {
            Current[Offset] = OldLines[OldIndex] == NewLines[NewBegin + Offset - 1]
                ? Previous[Offset - 1] + 1
                : FMath::Max(Previous[Offset], Current[Offset - 1]);
        }
        Swap(Previous, Current);
        Current.Init(0, NewCount + 1);
    }
    return Previous;
}

TArray<int32> LcsBackward(const TArray<FString>& OldLines, int32 OldBegin, int32 OldEnd,
    const TArray<FString>& NewLines, int32 NewBegin, int32 NewEnd)
{
    const int32 NewCount = NewEnd - NewBegin;
    TArray<int32> Previous, Current;
    Previous.Init(0, NewCount + 1);
    Current.Init(0, NewCount + 1);
    for (int32 OldIndex = OldEnd - 1; OldIndex >= OldBegin; --OldIndex)
    {
        for (int32 Offset = 1; Offset <= NewCount; ++Offset)
        {
            Current[Offset] = OldLines[OldIndex] == NewLines[NewEnd - Offset]
                ? Previous[Offset - 1] + 1
                : FMath::Max(Previous[Offset], Current[Offset - 1]);
        }
        Swap(Previous, Current);
        Current.Init(0, NewCount + 1);
    }
    return Previous;
}

void FindLcsMatches(const TArray<FString>& OldLines, int32 OldBegin, int32 OldEnd,
    const TArray<FString>& NewLines, int32 NewBegin, int32 NewEnd, TArray<FLcsMatch>& OutMatches)
{
    if (OldBegin >= OldEnd || NewBegin >= NewEnd) return;
    if (OldEnd - OldBegin == 1)
    {
        for (int32 NewIndex = NewBegin; NewIndex < NewEnd; ++NewIndex)
        {
            if (OldLines[OldBegin] == NewLines[NewIndex])
            {
                OutMatches.Add({OldBegin, NewIndex});
                break;
            }
        }
        return;
    }

    const int32 OldMiddle = OldBegin + (OldEnd - OldBegin) / 2;
    const TArray<int32> Left = LcsForward(OldLines, OldBegin, OldMiddle, NewLines, NewBegin, NewEnd);
    const TArray<int32> Right = LcsBackward(OldLines, OldMiddle, OldEnd, NewLines, NewBegin, NewEnd);
    const int32 NewCount = NewEnd - NewBegin;
    int32 BestOffset = 0;
    int32 BestLength = -1;
    for (int32 Offset = 0; Offset <= NewCount; ++Offset)
    {
        const int32 Length = Left[Offset] + Right[NewCount - Offset];
        if (Length > BestLength)
        {
            BestLength = Length;
            BestOffset = Offset;
        }
    }
    const int32 NewMiddle = NewBegin + BestOffset;
    FindLcsMatches(OldLines, OldBegin, OldMiddle, NewLines, NewBegin, NewMiddle, OutMatches);
    FindLcsMatches(OldLines, OldMiddle, OldEnd, NewLines, NewMiddle, NewEnd, OutMatches);
}

FString BuildLcsDiffBody(const TArray<FString>& OldLines, const TArray<FString>& NewLines,
    int32& OutAdded, int32& OutRemoved)
{
    TArray<FLcsMatch> Matches;
    FindLcsMatches(OldLines, 0, OldLines.Num(), NewLines, 0, NewLines.Num(), Matches);
    Matches.Add({OldLines.Num(), NewLines.Num()});

    FString Result;
    int32 OldCursor = 0;
    int32 NewCursor = 0;
    OutAdded = 0;
    OutRemoved = 0;
    for (const FLcsMatch& Match : Matches)
    {
        while (OldCursor < Match.OldIndex)
        {
            Result += TEXT("- ") + OldLines[OldCursor++] + TEXT("\n");
            ++OutRemoved;
        }
        while (NewCursor < Match.NewIndex)
        {
            Result += TEXT("+ ") + NewLines[NewCursor++] + TEXT("\n");
            ++OutAdded;
        }
        if (OldCursor < OldLines.Num() && NewCursor < NewLines.Num())
        {
            Result += TEXT("  ") + OldLines[OldCursor] + TEXT("\n");
            ++OldCursor;
            ++NewCursor;
        }
    }
    return Result;
}
}

void FAudioVidoAgentViewModel::SetState(EAudioVidoState NewState, const FString& Detail)
{
    State = NewState;
    StatusDetail = Detail;
    OnChanged.Broadcast();
}

void FAudioVidoAgentViewModel::AddMessage(bool bFromUser, const FText& Text)
{
    Messages.Add({bFromUser, Text, FDateTime::Now()});
    OnChanged.Broadcast();
}

bool FAudioVidoAgentViewModel::ValidateRelativePath(const FString& RelativePath, FString& OutAbsolutePath, FString& OutError) const
{
    FString Normalized = RelativePath;
    FPaths::NormalizeFilename(Normalized);
    FPaths::CollapseRelativeDirectories(Normalized);
    while (Normalized.StartsWith(TEXT("./"))) Normalized.RightChopInline(2);

    if (Normalized.IsEmpty() || !FPaths::IsRelative(Normalized) || Normalized.Contains(TEXT("..")))
    {
        OutError = TEXT("The path must be project-relative and cannot contain traversal.");
        return false;
    }

    const bool bAllowedRoot = Normalized.StartsWith(TEXT("Source/"), ESearchCase::IgnoreCase)
        || Normalized.StartsWith(TEXT("Config/"), ESearchCase::IgnoreCase)
        || Normalized.StartsWith(TEXT("Plugins/"), ESearchCase::IgnoreCase);
    if (!bAllowedRoot || !IsAllowedExtension(FPaths::GetExtension(Normalized)))
    {
        OutError = TEXT("This path is outside AudioVido's approved text-file scope.");
        return false;
    }

    const FString ProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    OutAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectRoot, Normalized));
    FPaths::NormalizeFilename(OutAbsolutePath);
    FString NormalizedRoot = ProjectRoot;
    FPaths::NormalizeFilename(NormalizedRoot);
    if (!OutAbsolutePath.StartsWith(NormalizedRoot, ESearchCase::IgnoreCase))
    {
        OutError = TEXT("The resolved path escapes the current project.");
        return false;
    }
    return true;
}

bool FAudioVidoAgentViewModel::ListApprovedFiles(const FString& RelativeRoot, TArray<FString>& OutFiles, FString& OutError) const
{
    FString Root = RelativeRoot.IsEmpty() ? TEXT("Source") : RelativeRoot;
    FPaths::NormalizeFilename(Root);
    FPaths::CollapseRelativeDirectories(Root);
    while (Root.StartsWith(TEXT("./"))) Root.RightChopInline(2);
    if (!FPaths::IsRelative(Root) || Root.Contains(TEXT(".."))
        || !(Root == TEXT("Source") || Root.StartsWith(TEXT("Source/"), ESearchCase::IgnoreCase)
            || Root == TEXT("Config") || Root.StartsWith(TEXT("Config/"), ESearchCase::IgnoreCase)
            || Root == TEXT("Plugins") || Root.StartsWith(TEXT("Plugins/"), ESearchCase::IgnoreCase)))
    {
        OutError = TEXT("List path is outside Source, Config, or Plugins.");
        return false;
    }
    const FString ProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    const FString AbsoluteRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectRoot, Root));
    TArray<FString> Found;
    IFileManager::Get().FindFilesRecursive(Found, *AbsoluteRoot, TEXT("*"), true, false, false);
    for (const FString& Absolute : Found)
    {
        if (!IsAllowedExtension(FPaths::GetExtension(Absolute))) continue;
        FString Relative = Absolute;
        FPaths::MakePathRelativeTo(Relative, *ProjectRoot);
        FPaths::NormalizeFilename(Relative);
        OutFiles.Add(Relative);
        if (OutFiles.Num() >= MaxListedFiles) break;
    }
    OutFiles.Sort();
    return true;
}

bool FAudioVidoAgentViewModel::ReadApprovedTextFile(const FString& RelativePath, FString& OutContent, FString& OutError) const
{
    FString AbsolutePath;
    if (!ValidateRelativePath(RelativePath, AbsolutePath, OutError)) return false;
    const int64 Size = IFileManager::Get().FileSize(*AbsolutePath);
    if (Size < 0)
    {
        OutError = TEXT("The requested file does not exist.");
        return false;
    }
    if (Size > MaxReadBytes)
    {
        OutError = TEXT("The requested file exceeds the 512 KB read limit.");
        return false;
    }
    if (!FFileHelper::LoadFileToString(OutContent, *AbsolutePath))
    {
        OutError = TEXT("The requested file could not be read as text.");
        return false;
    }
    return true;
}

bool FAudioVidoAgentViewModel::StageWrite(const FString& RelativePath, const FString& Content, FString& OutError)
{
    if (Content.Len() * sizeof(TCHAR) > MaxWriteBytes)
    {
        OutError = TEXT("The proposed file exceeds the 2 MB staging limit.");
        return false;
    }
    FString AbsolutePath;
    if (!ValidateRelativePath(RelativePath, AbsolutePath, OutError)) return false;

    FString Original;
    const bool bExists = FPaths::FileExists(AbsolutePath);
    if (bExists && !FFileHelper::LoadFileToString(Original, *AbsolutePath))
    {
        OutError = TEXT("The existing file could not be read safely.");
        return false;
    }

    FString Key = RelativePath;
    FPaths::NormalizeFilename(Key);
    TSharedPtr<FAudioVidoStagedFile>* Existing = StagedFiles.FindByPredicate(
        [&Key](const TSharedPtr<FAudioVidoStagedFile>& Item) { return Item->RelativePath.Equals(Key, ESearchCase::IgnoreCase); });
    TSharedPtr<FAudioVidoStagedFile> File = Existing ? *Existing : MakeShared<FAudioVidoStagedFile>();
    File->RelativePath = Key;
    File->OriginalContent = Original;
    File->StagedContent = Content;
    File->bCreated = !bExists;
    File->bSelected = true;
    File->Error.Empty();
    TArray<FString> OldLines, NewLines;
    Original.ParseIntoArrayLines(OldLines, false);
    Content.ParseIntoArrayLines(NewLines, false);
    if (Original.IsEmpty()) OldLines.Reset();
    if (Content.IsEmpty()) NewLines.Reset();
    BuildLcsDiffBody(OldLines, NewLines, File->AddedLines, File->RemovedLines);
    if (!Existing) StagedFiles.Add(File);
    SetState(EAudioVidoState::AwaitingReview, FString::Printf(TEXT("%d file(s) staged for review"), StagedFiles.Num()));
    return true;
}

FString FAudioVidoAgentViewModel::BuildUnifiedDiff(const FAudioVidoStagedFile& File) const
{
    TArray<FString> OldLines, NewLines;
    File.OriginalContent.ParseIntoArrayLines(OldLines, false);
    File.StagedContent.ParseIntoArrayLines(NewLines, false);
    if (File.OriginalContent.IsEmpty()) OldLines.Reset();
    if (File.StagedContent.IsEmpty()) NewLines.Reset();
    FString Result = FString::Printf(TEXT("--- a/%s\n+++ b/%s\n"), *File.RelativePath, *File.RelativePath);
    int32 Added = 0, Removed = 0;
    return Result + BuildLcsDiffBody(OldLines, NewLines, Added, Removed);
}

int32 FAudioVidoAgentViewModel::GetSelectedCount() const
{
    return StagedFiles.FilterByPredicate([](const auto& Item) { return Item->bSelected; }).Num();
}

void FAudioVidoAgentViewModel::SelectAll(bool bSelected)
{
    for (const auto& File : StagedFiles) File->bSelected = bSelected;
    OnChanged.Broadcast();
}

void FAudioVidoAgentViewModel::DiscardSelected()
{
    StagedFiles.RemoveAll([](const auto& Item) { return Item->bSelected; });
    SetState(StagedFiles.IsEmpty() ? EAudioVidoState::Idle : EAudioVidoState::AwaitingReview,
        StagedFiles.IsEmpty() ? TEXT("Selected changes were discarded") : TEXT("Review the remaining staged files"));
}

bool FAudioVidoAgentViewModel::ApplySelected(TArray<FString>& OutApplied, TArray<FString>& OutFailed)
{
    SetState(EAudioVidoState::Applying, TEXT("Applying selected files with backups"));
    const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"));
    const FString BackupRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AudioVidoAgentBackups"), Stamp);

    for (const auto& File : StagedFiles)
    {
        if (!File->bSelected) continue;
        FString AbsolutePath, Error;
        if (!ValidateRelativePath(File->RelativePath, AbsolutePath, Error))
        {
            File->Error = Error; OutFailed.Add(File->RelativePath); continue;
        }
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsolutePath), true);
        if (FPaths::FileExists(AbsolutePath))
        {
            const FString BackupPath = FPaths::Combine(BackupRoot, File->RelativePath);
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(BackupPath), true);
            if (IFileManager::Get().Copy(*BackupPath, *AbsolutePath, true, true) != COPY_OK)
            {
                File->Error = TEXT("Backup creation failed; the file was not changed."); OutFailed.Add(File->RelativePath); continue;
            }
        }
        const FString TempPath = AbsolutePath + TEXT(".audiovido.tmp");
        if (!FFileHelper::SaveStringToFile(File->StagedContent, *TempPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)
            || !IFileManager::Get().Move(*AbsolutePath, *TempPath, true, true, false, true))
        {
            IFileManager::Get().Delete(*TempPath, false, true);
            File->Error = TEXT("Atomic replacement failed; the staged file remains available."); OutFailed.Add(File->RelativePath); continue;
        }
        OutApplied.Add(File->RelativePath);
    }
    StagedFiles.RemoveAll([&OutApplied](const auto& Item) { return OutApplied.Contains(Item->RelativePath); });
    const bool bAllSucceeded = OutFailed.IsEmpty();
    SetState(bAllSucceeded ? EAudioVidoState::Complete : EAudioVidoState::Error,
        bAllSucceeded ? FString::Printf(TEXT("Applied %d file(s); backups saved under Saved/AudioVidoAgentBackups"), OutApplied.Num())
                      : FString::Printf(TEXT("Applied %d file(s); %d failed and remain staged"), OutApplied.Num(), OutFailed.Num()));
    return bAllSucceeded;
}

void FAudioVidoAgentViewModel::NewTask()
{
    Messages.Reset();
    SetState(EAudioVidoState::Idle, StagedFiles.IsEmpty() ? TEXT("Ready for a new task") : TEXT("New task started; staged changes were preserved"));
}
