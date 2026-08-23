#include "AVCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"

AAVCameraPawn::AAVCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CinematicCamera"));
	SetRootComponent(Camera);
	Camera->FieldOfView = 63.f;
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("NightGrade"));
	PostProcess->SetupAttachment(Camera);
	PostProcess->bUnbound = true;
	PostProcess->Settings.bOverride_VignetteIntensity = true;
	PostProcess->Settings.VignetteIntensity = .42f;
	PostProcess->Settings.bOverride_BloomIntensity = true;
	PostProcess->Settings.BloomIntensity = .7f;
	PostProcess->Settings.bOverride_ColorSaturation = true;
	PostProcess->Settings.ColorSaturation = FVector4(.9f, .94f, 1.f, 1.f);
	HomeLocation = FVector(0, -2250, 930);
	HomeRotation = FRotator(-17, 90, 0);
	TargetLocation = HomeLocation;
	TargetRotation = HomeRotation;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AAVCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	SetActorLocationAndRotation(HomeLocation, HomeRotation, false, nullptr, ETeleportType::TeleportPhysics);
	TargetLocation = HomeLocation;
	TargetRotation = HomeRotation;
}

void AAVCameraPawn::Tick(float Dt)
{
	Super::Tick(Dt);
	FVector FreeMove = GetActorForwardVector() * MoveForwardInput * Dt * 430.f + GetActorRightVector() * MoveRightInput * Dt * 430.f;
	if (!FreeMove.IsNearlyZero())
	{
		TargetLocation += FVector(FreeMove.X, FreeMove.Y, 0);
		TargetLocation.X = FMath::Clamp(TargetLocation.X, -700.f, 700.f);
		TargetLocation.Y = FMath::Clamp(TargetLocation.Y, -2400.f, -1050.f);
	}
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLocation, Dt, 2.3f));
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, Dt, 2.5f));
}

void AAVCameraPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);
	Input->BindAxis(TEXT("MoveForward"), this, &AAVCameraPawn::MoveForward);
	Input->BindAxis(TEXT("MoveRight"), this, &AAVCameraPawn::MoveRight);
}

void AAVCameraPawn::FocusAt(const FVector& P)
{
    const float SideSign = P.Y > 0.f ? -1.f : 1.f;

    // Pull farther away and lift the camera so it does not intersect venue geometry.
    TargetLocation = P + FVector(
        -420.f,
        SideSign * 1180.f,
        520.f
    );

    const FVector LookAt = P + FVector(0.f, 0.f, 170.f);
    TargetRotation = (LookAt - TargetLocation).Rotation();
}

void AAVCameraPawn::ResetView()
{
	TargetLocation = HomeLocation;
	TargetRotation = HomeRotation;
}
