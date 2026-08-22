#include "AudioVidoProviderService.h"

#include "AudioVidoAgentViewModel.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FAudioVidoProviderService::FAudioVidoProviderService(TSharedRef<FAudioVidoAgentViewModel> InViewModel)
    : ViewModel(InViewModel)
{
}

void FAudioVidoProviderService::Send(const FString& Prompt)
{
    const TSharedPtr<FAudioVidoAgentViewModel> Model = ViewModel.Pin();
    if (!Model.IsValid() || ActiveRequest.IsValid()) return;
    bCancelled = false;
    if (ToolStep == 0)
    {
        OriginalPrompt = Prompt;
        AgentContext.Empty();
    }
    const FString EffectivePrompt = ToolStep == 0 ? Prompt : OriginalPrompt + TEXT("\n\nSAFE TOOL TRANSCRIPT:\n") + AgentContext;

    ActiveRequest = FHttpModule::Get().CreateRequest();
    ActiveRequest->SetURL(Model->bUseOllama
        ? TEXT("http://127.0.0.1:11434/api/chat")
        : TEXT("https://openrouter.ai/api/v1/chat/completions"));
    ActiveRequest->SetVerb(TEXT("POST"));
    ActiveRequest->SetTimeout(600.0f);
    ActiveRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    if (!Model->bUseOllama)
    {
        ActiveRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Model->ApiKey);
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("model"), Model->ModelId);
    // Ollama's local proxy can terminate long non-streaming generations after
    // roughly 30 seconds. Keep streaming enabled and assemble its NDJSON body
    // in HandleResponse; IHttpRequest still invokes the completion delegate
    // once the full response has arrived.
    Root->SetBoolField(TEXT("stream"), Model->bUseOllama);
    if (Model->bUseOllama)
    {
        Root->SetStringField(TEXT("format"), TEXT("json"));
        TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
        Options->SetNumberField(TEXT("num_predict"), 8192);
        Root->SetObjectField(TEXT("options"), Options);
    }
    TArray<TSharedPtr<FJsonValue>> Messages;
    TSharedRef<FJsonObject> System = MakeShared<FJsonObject>();
    System->SetStringField(TEXT("role"), TEXT("system"));
    System->SetStringField(TEXT("content"), TEXT(
        "You are AudioVido Agent, a native Unreal Engine 5.8 coding agent. Return JSON only: "
        "{\"message\":\"brief progress\",\"actions\":[{\"type\":\"list\",\"path\":\"Plugins/AudioVidoAgent\"},{\"type\":\"read\",\"path\":\"Plugins/.../File.cpp\"}],"
        "\"writes\":[{\"path\":\"Plugins/...\",\"content\":\"complete file\"}]}. "
        "Use list/read actions to inspect before proposing writes. Request exactly one inspection action per turn so local context stays bounded. "
        "After inspection, put complete file replacements in the top-level writes array. Never put write operations in actions. "
        "Only Source, Config, and Plugins text files are available. Writes are staged for human approval and are never auto-applied. "
        "Do not request shell, delete, rename, binaries, assets, external paths, secrets, or builds."));
    Messages.Add(MakeShared<FJsonValueObject>(System));
    TSharedRef<FJsonObject> User = MakeShared<FJsonObject>();
    User->SetStringField(TEXT("role"), TEXT("user"));
    User->SetStringField(TEXT("content"), EffectivePrompt);
    Messages.Add(MakeShared<FJsonValueObject>(User));
    Root->SetArrayField(TEXT("messages"), Messages);
    Root->SetNumberField(TEXT("temperature"), 0.1);

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Root, Writer);
    ActiveRequest->SetContentAsString(Body);
    ActiveRequest->OnProcessRequestComplete().BindSP(this, &FAudioVidoProviderService::HandleResponse);
    Model->SetState(EAudioVidoState::Working, TEXT("AudioVido is working..."));
    if (!ActiveRequest->ProcessRequest())
    {
        ActiveRequest.Reset();
        Model->SetState(EAudioVidoState::Offline, TEXT("The provider request could not be started. Your draft is preserved."));
    }
}

void FAudioVidoProviderService::RefreshLocalModels()
{
    const TSharedPtr<FAudioVidoAgentViewModel> Model = ViewModel.Pin();
    if (!Model.IsValid() || ActiveRequest.IsValid()) return;
    ActiveRequest = FHttpModule::Get().CreateRequest();
    ActiveRequest->SetURL(TEXT("http://127.0.0.1:11434/api/tags"));
    ActiveRequest->SetVerb(TEXT("GET"));
    ActiveRequest->OnProcessRequestComplete().BindSP(this, &FAudioVidoProviderService::HandleModelsResponse);
    Model->SetState(EAudioVidoState::Checking, TEXT("Checking local Ollama models..."));
    if (!ActiveRequest->ProcessRequest())
    {
        ActiveRequest.Reset();
        Model->SetState(EAudioVidoState::Offline, TEXT("Ollama is not reachable at 127.0.0.1:11434."));
    }
}

void FAudioVidoProviderService::HandleModelsResponse(FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
{
    ActiveRequest.Reset();
    const TSharedPtr<FAudioVidoAgentViewModel> Model = ViewModel.Pin();
    if (!Model.IsValid()) return;
    if (!bSucceeded || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        Model->SetState(EAudioVidoState::Offline, TEXT("Ollama is offline. Start Ollama and test the connection again."));
        return;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Root) || !Root.IsValid())
    {
        Model->SetState(EAudioVidoState::Error, TEXT("Ollama returned an invalid model list."));
        return;
    }
    const TArray<TSharedPtr<FJsonValue>>* Models = nullptr;
    Model->AvailableModels.Reset();
    if (Root->TryGetArrayField(TEXT("models"), Models))
    {
        for (const TSharedPtr<FJsonValue>& Value : *Models)
        {
            FString Name;
            if (Value->AsObject().IsValid() && Value->AsObject()->TryGetStringField(TEXT("name"), Name))
                Model->AvailableModels.Add(MakeShared<FString>(Name));
        }
    }
    const bool bCurrentExists = Model->AvailableModels.ContainsByPredicate(
        [Model](const TSharedPtr<FString>& Name) { return Name.IsValid() && *Name == Model->ModelId; });
    if (!bCurrentExists && !Model->AvailableModels.IsEmpty()) Model->ModelId = *Model->AvailableModels[0];
    Model->SetState(Model->AvailableModels.IsEmpty() ? EAudioVidoState::Error : EAudioVidoState::Idle,
        Model->AvailableModels.IsEmpty()
            ? TEXT("Ollama is online but no local models are installed.")
            : FString::Printf(TEXT("Ollama connected · %d local model(s)"), Model->AvailableModels.Num()));
}

void FAudioVidoProviderService::Cancel()
{
    if (ActiveRequest.IsValid())
    {
        ActiveRequest->CancelRequest();
        ActiveRequest.Reset();
        bCancelled = true;
        ToolStep = 0;
        AgentContext.Empty();
        if (const auto Model = ViewModel.Pin()) Model->SetState(EAudioVidoState::Stopped, TEXT("The request was stopped. No files were changed."));
    }
}

void FAudioVidoProviderService::HandleResponse(FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
{
    ActiveRequest.Reset();
    const TSharedPtr<FAudioVidoAgentViewModel> Model = ViewModel.Pin();
    if (!Model.IsValid()) return;
    if (bCancelled) return;
    if (!bSucceeded || !Response.IsValid())
    {
        Model->SetState(EAudioVidoState::Offline, TEXT("Network failure. No files were changed; retry when the provider is available."));
        return;
    }
    const int32 Code = Response->GetResponseCode();
    if (Code < 200 || Code >= 300)
    {
        Model->SetState(EAudioVidoState::Error, Model->bUseOllama
            ? FString::Printf(TEXT("Ollama returned HTTP %d. Verify the selected local model."), Code)
            : FString::Printf(TEXT("OpenRouter returned HTTP %d. Check the session key and model in Settings."), Code));
        return;
    }

    FString Content;
    if (Model->bUseOllama)
    {
        TArray<FString> Chunks;
        Response->GetContentAsString().ParseIntoArrayLines(Chunks, true);
        bool bSawMessage = false;
        for (const FString& Chunk : Chunks)
        {
            TSharedPtr<FJsonObject> Envelope;
            if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Chunk), Envelope) || !Envelope.IsValid()) continue;
            const TSharedPtr<FJsonObject>* MessageObject = nullptr;
            FString Piece;
            if (Envelope->TryGetObjectField(TEXT("message"), MessageObject) && MessageObject && (*MessageObject).IsValid()
                && (*MessageObject)->TryGetStringField(TEXT("content"), Piece))
            {
                Content += Piece;
                bSawMessage = true;
            }
        }
        if (!bSawMessage)
        {
            Model->SetState(EAudioVidoState::Error, TEXT("Ollama returned no message content."));
            return;
        }
    }
    else
    {
        TSharedPtr<FJsonObject> Envelope;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Envelope) || !Envelope.IsValid())
        {
            Model->SetState(EAudioVidoState::Error, TEXT("The provider returned invalid JSON. No files were changed."));
            return;
        }
        const TSharedPtr<FJsonObject>* MessageObject = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Choices = nullptr;
        if (!Envelope->TryGetArrayField(TEXT("choices"), Choices) || Choices->IsEmpty()
            || !(*Choices)[0].IsValid() || !(*Choices)[0]->AsObject().IsValid()
            || !(*Choices)[0]->AsObject()->TryGetObjectField(TEXT("message"), MessageObject)
            || !MessageObject || !(*MessageObject).IsValid()
            || !(*MessageObject)->TryGetStringField(TEXT("content"), Content))
        {
            Model->SetState(EAudioVidoState::Error, TEXT("OpenRouter returned no usable message content."));
            return;
        }
    }
    Content.TrimStartAndEndInline();
    if (Content.StartsWith(TEXT("```")))
    {
        int32 FirstNewline = INDEX_NONE, LastFence = INDEX_NONE;
        Content.FindChar(TEXT('\n'), FirstNewline);
        LastFence = Content.Find(TEXT("```"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
        if (FirstNewline != INDEX_NONE && LastFence > FirstNewline) Content = Content.Mid(FirstNewline + 1, LastFence - FirstNewline - 1);
    }
    TSharedPtr<FJsonObject> Action;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Content), Action) || !Action.IsValid())
    {
        Model->SetState(EAudioVidoState::Error, TEXT("The provider returned an invalid action response. No files were changed."));
        return;
    }
    FString Explanation;
    Action->TryGetStringField(TEXT("message"), Explanation);
    if (!Explanation.IsEmpty()) Model->AddMessage(false, FText::FromString(Explanation));

    // Local coding models do not always follow the schema perfectly. In
    // particular, they may return writes beside a final inspection action, or
    // encode a staged write as an action. Always collect safe staged writes
    // before deciding whether another inspection turn is required.
    int32 Accepted = 0;
    auto StageWriteObject = [&Model, &Accepted](const TSharedPtr<FJsonObject>& Write)
    {
        if (!Write.IsValid()) return;
        FString Path, ProposedContent, Error;
        if (!Write->TryGetStringField(TEXT("path"), Path))
            Write->TryGetStringField(TEXT("file_path"), Path);
        if (!Write->TryGetStringField(TEXT("content"), ProposedContent))
            Write->TryGetStringField(TEXT("code"), ProposedContent);
        if (!Path.IsEmpty() && (Write->HasField(TEXT("content")) || Write->HasField(TEXT("code"))))
        {
            if (Model->StageWrite(Path, ProposedContent, Error)) ++Accepted;
            else if (!Error.IsEmpty())
                Model->AddMessage(false, FText::FromString(TEXT("Rejected unsafe path: ") + Path + TEXT(" — ") + Error));
        }
    };

    const TArray<TSharedPtr<FJsonValue>>* Writes = nullptr;
    if (Action->TryGetArrayField(TEXT("writes"), Writes))
        for (const TSharedPtr<FJsonValue>& Value : *Writes) StageWriteObject(Value.IsValid() ? Value->AsObject() : nullptr);

    const TArray<TSharedPtr<FJsonValue>>* Actions = nullptr;
    if (Action->TryGetArrayField(TEXT("actions"), Actions) && !Actions->IsEmpty())
    {
        FString Results;
        int32 Executed = 0;
        for (const TSharedPtr<FJsonValue>& Value : *Actions)
        {
            // Local Ollama's proxy can time out while evaluating very large
            // multi-file transcripts before it emits the first stream chunk.
            // One tool result per turn keeps each inspection reviewable and
            // prevents a model from flooding its own context window.
            if (Executed >= 1) break;
            const TSharedPtr<FJsonObject> Tool = Value->AsObject();
            if (!Tool.IsValid()) continue;
            FString Type, Path, Error;
            Tool->TryGetStringField(TEXT("type"), Type);
            Tool->TryGetStringField(TEXT("path"), Path);
            if (Type.IsEmpty()) Tool->TryGetStringField(TEXT("action"), Type);
            if (Path.IsEmpty()) Tool->TryGetStringField(TEXT("target"), Path);
            if (Type == TEXT("write") || Type == TEXT("stage_write") || Type == TEXT("stage"))
            {
                StageWriteObject(Tool);
                ++Executed;
                continue;
            }
            if (Type == TEXT("list"))
            {
                TArray<FString> Files;
                const bool bOk = Model->ListApprovedFiles(Path, Files, Error);
                Results += FString::Printf(TEXT("\nLIST %s: %s\n%s"), *Path, bOk ? TEXT("OK") : *Error, *FString::Join(Files, TEXT("\n")));
            }
            else if (Type == TEXT("read"))
            {
                FString FileContent;
                const bool bOk = Model->ReadApprovedTextFile(Path, FileContent, Error);
                Results += FString::Printf(TEXT("\nREAD %s: %s\n%s"), *Path, bOk ? TEXT("OK") : *Error, bOk ? *FileContent : TEXT(""));
            }
            else
            {
                Results += FString::Printf(TEXT("\nREJECTED ACTION %s: only list/read are allowed."), *Type);
            }
            ++Executed;
        }
        if (Accepted > 0)
        {
            ToolStep = 0;
            AgentContext.Empty();
            Model->SetState(EAudioVidoState::AwaitingReview,
                FString::Printf(TEXT("%d file(s) staged for review"), Model->StagedFiles.Num()));
            return;
        }
        ++ToolStep;
        if (ToolStep >= 10)
        {
            ToolStep = 0;
            AgentContext.Empty();
            Model->SetState(EAudioVidoState::Error, TEXT("Stopped after the safe 10-step tool limit. No files were applied."));
            return;
        }
        AgentContext += TEXT("\nAGENT ACTION:\n") + Content + TEXT("\nACTUAL TOOL RESULTS:\n") + Results;
        Model->SetState(EAudioVidoState::Working, FString::Printf(TEXT("Agent inspected project files · step %d/10"), ToolStep));
        Send(OriginalPrompt);
        return;
    }
    ToolStep = 0;
    AgentContext.Empty();
    if (Accepted == 0) Model->SetState(EAudioVidoState::Complete, TEXT("Request completed. No file changes were proposed."));
}
