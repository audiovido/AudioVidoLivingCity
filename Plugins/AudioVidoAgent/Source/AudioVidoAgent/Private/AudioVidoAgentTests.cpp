#if WITH_DEV_AUTOMATION_TESTS

#include "AudioVidoAgentViewModel.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioVidoSandboxTest,
    "AudioVido.Agent.Safety.PathValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAudioVidoSandboxTest::RunTest(const FString&)
{
    const TSharedRef<FAudioVidoAgentViewModel> Model = MakeShared<FAudioVidoAgentViewModel>();
    FString Absolute, Error;
    TestTrue(TEXT("Source text file is accepted"), Model->ValidateRelativePath(TEXT("Source/Example.cpp"), Absolute, Error));
    TestFalse(TEXT("Traversal is rejected"), Model->ValidateRelativePath(TEXT("Source/../../secret.txt"), Absolute, Error));
    TestFalse(TEXT("Absolute path is rejected"), Model->ValidateRelativePath(TEXT("C:/secret.txt"), Absolute, Error));
    TestFalse(TEXT("Content assets are rejected"), Model->ValidateRelativePath(TEXT("Content/Test.uasset"), Absolute, Error));
    TestFalse(TEXT("Binary extensions are rejected"), Model->ValidateRelativePath(TEXT("Plugins/Test.dll"), Absolute, Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioVidoUnifiedDiffTest,
    "AudioVido.Agent.Diff.LcsInsertionAndStats",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAudioVidoUnifiedDiffTest::RunTest(const FString&)
{
    const TSharedRef<FAudioVidoAgentViewModel> Model = MakeShared<FAudioVidoAgentViewModel>();
    FAudioVidoStagedFile File;
    File.RelativePath = TEXT("Source/Example.cpp");
    File.OriginalContent = TEXT("alpha\nbeta\ngamma");
    File.StagedContent = TEXT("alpha\ninserted\nbeta\ngamma");

    const FString Diff = Model->BuildUnifiedDiff(File);
    TestTrue(TEXT("Inserted line is marked as added"), Diff.Contains(TEXT("+ inserted\n")));
    TestTrue(TEXT("Following line remains context"), Diff.Contains(TEXT("  beta\n")));
    TestFalse(TEXT("Following line is not falsely removed"), Diff.Contains(TEXT("- beta\n")));

    FString Error;
    TestTrue(TEXT("A proposed new text file can be staged"),
        Model->StageWrite(TEXT("Source/AudioVidoAgentDiffStatsTest.txt"), TEXT("one\ntwo\nthree"), Error));
    TestEqual(TEXT("New-file added count is accurate"), Model->StagedFiles[0]->AddedLines, 3);
    TestEqual(TEXT("New-file removed count is accurate"), Model->StagedFiles[0]->RemovedLines, 0);
    return true;
}

#endif
