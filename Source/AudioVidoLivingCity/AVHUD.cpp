#include "AVHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

AAVHUD::AAVHUD() {}

void AAVHUD::DismissWelcome()
{
    bShowWelcome = false;
}

void AAVHUD::SetSelectedVenue(const FAVVenueData& Venue)
{
    SelectedVenue = Venue;
    bHasSelectedVenue = true;
}

void AAVHUD::ClearSelectedVenue()
{
    bHasSelectedVenue = false;
}

void AAVHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GEngine) return;
    const float W = Canvas->SizeX;
    const float H = Canvas->SizeY;
    DrawRect(FLinearColor(0.015f,0.025f,0.045f,0.94f), 0, 0, W, 76);
    Canvas->DrawText(GEngine->GetLargeFont(), TEXT("AudioVido Living City"), 32, 18, 1.1f, 1.1f);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("Explore creative spaces and venues"), 34, 50);
    DrawRect(FLinearColor(0.02f,0.04f,0.07f,0.88f), 24, 110, 220, 250);
    Canvas->DrawText(GEngine->GetMediumFont(), TEXT("SPACES"), 48, 132);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("LUMIERE CINEMA"), 52, 184);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("VELVET ROOM"), 52, 232);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("THE FORUM"), 52, 280);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("COMMON GROUND"), 52, 328);
    if (bShowWelcome)
    {
    DrawRect(FLinearColor(0.025f,0.045f,0.075f,0.96f), W*0.5f-220, H*0.5f-130, 440, 260);
    Canvas->DrawText(GEngine->GetLargeFont(), TEXT("Welcome"), W*0.5f-72, H*0.5f-96, FLinearColor(1, 1, 1, 1));
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("Choose how you enter the living city."), W*0.5f-145, H*0.5f-46, FLinearColor(1, 1, 1, 1));
    DrawRect(FLinearColor(0.05f,0.72f,0.78f,1), W*0.5f-170, H*0.5f+28, 150, 48);
    DrawRect(FLinearColor(0.12f,0.16f,0.22f,1), W*0.5f+20, H*0.5f+28, 150, 48);
    Canvas->DrawText(GEngine->GetMediumFont(), TEXT("Continue"), W*0.5f-145, H*0.5f+41, FLinearColor(1, 1, 1, 1));
    Canvas->DrawText(GEngine->GetMediumFont(), TEXT("Privacy"), W*0.5f+48, H*0.5f+41, FLinearColor(1, 1, 1, 1));
    }
    if (bHasSelectedVenue && !bShowWelcome)
    {
        const float PanelX = W - 360.f;
        const float PanelY = 110.f;
        const float PanelW = 336.f;
        const float PanelH = 300.f;
        DrawRect(FLinearColor(0.018f,0.032f,0.055f,0.96f), PanelX, PanelY, PanelW, PanelH);
        DrawRect(SelectedVenue.Accent, PanelX, PanelY, 5.f, PanelH);
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Eyebrow, PanelX+24.f, PanelY+22.f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetLargeFont(), SelectedVenue.Name, PanelX+24.f, PanelY+50.f, 0.9f, 0.9f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetMediumFont(), SelectedVenue.Headline, PanelX+24.f, PanelY+96.f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Time, PanelX+24.f, PanelY+142.f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Distance, PanelX+24.f, PanelY+172.f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Presence, PanelX+24.f, PanelY+202.f, FLinearColor(1, 1, 1, 1));
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.AgeRule, PanelX+24.f, PanelY+246.f, FLinearColor(1, 1, 1, 1));
        DrawRect(FLinearColor(0.08f,0.12f,0.18f,1.f), PanelX+24.f, PanelY+262.f, 96.f, 28.f);
        Canvas->DrawText(GEngine->GetSmallFont(), TEXT("BACK"), PanelX+52.f, PanelY+269.f, FLinearColor(1, 1, 1, 1));
    }
    DrawRect(FLinearColor(0.01f,0.02f,0.035f,0.92f), 0, H-54, W, 54);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("WASD Move | Mouse Look | Click Explore | Esc Close"), 32, H-35, FLinearColor(1, 1, 1, 1));
}
