#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AVPlayerController.generated.h"

UCLASS()
class AUDIOVIDOLIVINGCITY_API AAVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAVPlayerController();

protected:
	virtual void SetupInputComponent() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void HandlePrimaryClick();
	void HandleEscape();
};