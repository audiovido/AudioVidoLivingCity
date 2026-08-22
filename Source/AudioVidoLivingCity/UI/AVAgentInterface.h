#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AVAgentInterface.generated.h"

UCLASS()
class AUDIOVIDOLIVINGCITY_API UAVAgentInterface : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateProjectStatus(const FString& ProjectName, const FString& Status);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateModelInfo(const FString& ModelName, const FString& ModelRole);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateConversation(const FString& Message);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateObjective(const FString& Objective);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdatePlan(const TArray<FString>& PlanSteps);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateExecutionProgress(float Progress);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateLiveState(bool bIsLive);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateChanges(const TArray<FString>& Changes);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateVisualQA(const FString& VisualQAStatus);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateRuntimeState(const FString& State);
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent Interface") void UpdateViewportCapture(const FString& CapturePath);
};
