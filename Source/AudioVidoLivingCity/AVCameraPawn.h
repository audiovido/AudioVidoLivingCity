#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AVCameraPawn.generated.h"

UCLASS()
class AUDIOVIDOLIVINGCITY_API AAVCameraPawn : public APawn
{
	GENERATED_BODY()
public:
	AAVCameraPawn();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void FocusAt(const FVector& VenueLocation);
	void ResetView();
private:
	UPROPERTY() class UCameraComponent* Camera;
	UPROPERTY() class UPostProcessComponent* PostProcess;
	FVector TargetLocation;
	FRotator TargetRotation;
	FVector HomeLocation;
	FRotator HomeRotation;
	float MoveForwardInput=0.f, MoveRightInput=0.f;
	void MoveForward(float V){MoveForwardInput=V;}
	void MoveRight(float V){MoveRightInput=V;}
};