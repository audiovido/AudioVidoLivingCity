#include "AVGameMode.h"
#include "AVHUD.h"
#include "AVPlayerController.h"
#include "AVCameraPawn.h"
#include "AVCityBlock.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AAVGameMode::AAVGameMode()
{
    HUDClass = AAVHUD::StaticClass();
    PlayerControllerClass = AAVPlayerController::StaticClass();
    DefaultPawnClass = AAVCameraPawn::StaticClass();
}

void AAVGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (GetWorld() && !UGameplayStatics::GetActorOfClass(GetWorld(), AAVCityBlock::StaticClass()))
    {
        GetWorld()->SpawnActor<AAVCityBlock>(FVector::ZeroVector, FRotator::ZeroRotator);
    }
}