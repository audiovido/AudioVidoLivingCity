#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AVGameMode.generated.h"

UCLASS()
class AUDIOVIDOLIVINGCITY_API AAVGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AAVGameMode();
    virtual void BeginPlay() override;
};
