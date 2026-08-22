#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

class FAudioVidoAgentViewModel;

class FAudioVidoProviderService : public TSharedFromThis<FAudioVidoProviderService>
{
public:
    explicit FAudioVidoProviderService(TSharedRef<FAudioVidoAgentViewModel> InViewModel);
    void Send(const FString& Prompt);
    void RefreshLocalModels();
    void Cancel();
    bool IsBusy() const { return ActiveRequest.IsValid(); }

private:
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
    void HandleModelsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
    TWeakPtr<FAudioVidoAgentViewModel> ViewModel;
    TSharedPtr<class IHttpRequest> ActiveRequest;
    FString OriginalPrompt;
    FString AgentContext;
    int32 ToolStep = 0;
    bool bCancelled = false;
};
