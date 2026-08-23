#include "AVPlayerController.h"
#include "Components/InputComponent.h"
#include "AVCityBlock.h"
#include "AVCameraPawn.h"
#include "AVHUD.h"
#include "Kismet/GameplayStatics.h"

AAVPlayerController::AAVPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

bool AAVPlayerController::AutomationDismissWelcome()
{
    AAVHUD* HUD = Cast<AAVHUD>(GetHUD());

    if (!HUD)
    {
        return false;
    }

    HUD->DismissWelcome();
    return !HUD->IsWelcomeVisible();
}

bool AAVPlayerController::SelectVenueByIndex(int32 Index)
{
    AAVCityBlock* City = Cast<AAVCityBlock>(
        UGameplayStatics::GetActorOfClass(
            GetWorld(),
            AAVCityBlock::StaticClass()
        )
    );

    AAVCameraPawn* CameraPawn = Cast<AAVCameraPawn>(GetPawn());
    AAVHUD* HUD = Cast<AAVHUD>(GetHUD());

    if (!City || !CameraPawn || !HUD)
    {
        return false;
    }

    const TArray<FAVVenueData>& Venues = City->GetVenues();

    if (!Venues.IsValidIndex(Index))
    {
        return false;
    }

    const FAVVenueData& Venue = Venues[Index];
    CameraPawn->FocusAt(Venue.WorldLocation);
    HUD->SetSelectedVenue(Venue);

    return true;
}

bool AAVPlayerController::AutomationSelectVenue(int32 Index)
{
    return SelectVenueByIndex(Index);
}

bool AAVPlayerController::AutomationClearVenue()
{
    AAVHUD* HUD = Cast<AAVHUD>(GetHUD());
    AAVCameraPawn* CameraPawn = Cast<AAVCameraPawn>(GetPawn());

    if (!HUD || !CameraPawn)
    {
        return false;
    }

    HUD->ClearSelectedVenue();
    CameraPawn->ResetView();

    return true;
}

void AAVPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction(TEXT("PrimaryClick"), IE_Pressed, this, &AAVPlayerController::HandlePrimaryClick);
	InputComponent->BindAction(TEXT("Escape"), IE_Pressed, this, &AAVPlayerController::HandleEscape);
}

void AAVPlayerController::MoveForward(float Value)
{
}

void AAVPlayerController::MoveRight(float Value)
{
}

void AAVPlayerController::HandlePrimaryClick()
{
AAVCityBlock* City = Cast<AAVCityBlock>(UGameplayStatics::GetActorOfClass(GetWorld(), AAVCityBlock::StaticClass()));
    AAVCameraPawn* CameraPawn = Cast<AAVCameraPawn>(GetPawn());
    if (!City || !CameraPawn) return;
    AAVHUD* HUD = Cast<AAVHUD>(GetHUD());
    float MouseX = 0.f, MouseY = 0.f;
    if (HUD && HUD->IsWelcomeVisible() && GetMousePosition(MouseX, MouseY))
    {
        int32 ViewW = 0, ViewH = 0;
        GetViewportSize(ViewW, ViewH);
        const float ContinueLeft = ViewW * 0.5f - 170.f;
        const float ContinueRight = ViewW * 0.5f - 20.f;
        const float ContinueTop = ViewH * 0.5f + 28.f;
        const float ContinueBottom = ViewH * 0.5f + 76.f;
        if (MouseX >= ContinueLeft && MouseX <= ContinueRight && MouseY >= ContinueTop && MouseY <= ContinueBottom)
        {
            HUD->DismissWelcome();
            return;
        }
        return;
    }

    if (HUD && HUD->HasSelectedVenue() && GetMousePosition(MouseX, MouseY))
    {
        int32 ViewW = 0, ViewH = 0;
        GetViewportSize(ViewW, ViewH);
        const float BackLeft = ViewW - 336.f;
        const float BackRight = ViewW - 240.f;
        const float BackTop = 372.f;
        const float BackBottom = 400.f;
        if (MouseX >= BackLeft && MouseX <= BackRight && MouseY >= BackTop && MouseY <= BackBottom)
        {
            HUD->ClearSelectedVenue();
            CameraPawn->ResetView();
            return;
        }
    }
    MouseX = 0.f;
    MouseY = 0.f;
    if (GetMousePosition(MouseX, MouseY) && MouseX >= 24.f && MouseX <= 244.f)
    {
        const TArray<FAVVenueData>& Venues = City->GetVenues();
        int32 Index = INDEX_NONE;
        if (MouseY >= 165.f && MouseY < 213.f) Index = 0;
        else if (MouseY >= 213.f && MouseY < 261.f) Index = 1;
        else if (MouseY >= 261.f && MouseY < 309.f) Index = 2;
        else if (MouseY >= 309.f && MouseY < 357.f) Index = 3;
        if (SelectVenueByIndex(Index))
        {
            return;
        }
    }

    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, true, Hit) || !Hit.GetComponent()) return;

    for (const FName& Tag : Hit.GetComponent()->ComponentTags)
    {
        const TArray<FAVVenueData>& Venues = City->GetVenues();

        for (int32 Index = 0; Index < Venues.Num(); ++Index)
        {
            if (Venues[Index].Id == Tag)
            {
                SelectVenueByIndex(Index);
                return;
            }
        }
    }
}

void AAVPlayerController::HandleEscape()
{
    AAVHUD* HUD = Cast<AAVHUD>(GetHUD());
    AAVCameraPawn* CameraPawn = Cast<AAVCameraPawn>(GetPawn());

    if (HUD && CameraPawn)
    {
        HUD->ClearSelectedVenue();
        CameraPawn->ResetView();
        SetInputMode(FInputModeGameAndUI());
        bShowMouseCursor = true;
        return;
    }

    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}













