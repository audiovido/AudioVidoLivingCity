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

    const FLinearColor TextPrimary(0.92f, 0.96f, 1.00f, 1.f);
    const FLinearColor TextSecondary(0.72f, 0.80f, 0.90f, 1.f);
    const FLinearColor Panel(0.018f, 0.035f, 0.060f, 0.94f);
    const FLinearColor PanelStrong(0.014f, 0.026f, 0.048f, 0.98f);
    const FLinearColor Accent(0.10f, 0.78f, 0.88f, 1.f);
    const FLinearColor ButtonSecondary(0.18f, 0.26f, 0.36f, 1.f);

    DrawRect(PanelStrong, 0, 0, W, 76);

    Canvas->DrawText(
        GEngine->GetLargeFont(),
        TEXT("AudioVido Living City"),
        32, 18, 1.1f, 1.1f,
        TextPrimary
    );

    Canvas->DrawText(
        GEngine->GetSmallFont(),
        TEXT("Explore creative spaces and venues"),
        34, 50,
        1.f, 1.f,
        TextSecondary
    );

    DrawRect(Panel, 24, 110, 220, 250);

    Canvas->DrawText(
        GEngine->GetMediumFont(),
        TEXT("SPACES"),
        48, 132,
        1.f, 1.f,
        TextPrimary
    );

    const FLinearColor VenueText = TextSecondary;

    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("LUMIERE CINEMA"), 52, 184, 1.f, 1.f, VenueText);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("VELVET ROOM"), 52, 232, 1.f, 1.f, VenueText);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("THE FORUM"), 52, 280, 1.f, 1.f, VenueText);
    Canvas->DrawText(GEngine->GetSmallFont(), TEXT("COMMON GROUND"), 52, 328, 1.f, 1.f, VenueText);

    if (bShowWelcome)
    {
        DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.46f), 0, 76, W, H - 130);

        DrawRect(
            FLinearColor(0.02f, 0.04f, 0.075f, 0.985f),
            W * 0.5f - 220,
            H * 0.5f - 130,
            440,
            260
        );

        Canvas->DrawText(
            GEngine->GetLargeFont(),
            TEXT("Welcome"),
            W * 0.5f - 72,
            H * 0.5f - 96,
            1.f, 1.f,
            TextPrimary
        );

        Canvas->DrawText(
            GEngine->GetSmallFont(),
            TEXT("Choose how you enter the living city."),
            W * 0.5f - 145,
            H * 0.5f - 46,
            1.f, 1.f,
            TextSecondary
        );

        DrawRect(Accent, W * 0.5f - 170, H * 0.5f + 28, 150, 48);
        DrawRect(ButtonSecondary, W * 0.5f + 20, H * 0.5f + 28, 150, 48);

        Canvas->DrawText(
            GEngine->GetMediumFont(),
            TEXT("Continue"),
            W * 0.5f - 145,
            H * 0.5f + 41,
            1.f, 1.f,
            FLinearColor::White
        );

        Canvas->DrawText(
            GEngine->GetMediumFont(),
            TEXT("Privacy"),
            W * 0.5f + 48,
            H * 0.5f + 41,
            1.f, 1.f,
            FLinearColor::White
        );
    }

    if (bHasSelectedVenue && !bShowWelcome)
    {
        const float PanelX = W - 360.f;
        const float PanelY = 110.f;
        const float PanelW = 336.f;
        const float PanelH = 300.f;

        DrawRect(PanelStrong, PanelX, PanelY, PanelW, PanelH);
        DrawRect(SelectedVenue.Accent, PanelX, PanelY, 5.f, PanelH);

        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Eyebrow, PanelX + 24.f, PanelY + 22.f, 1.f, 1.f, TextSecondary);
        Canvas->DrawText(GEngine->GetLargeFont(), SelectedVenue.Name, PanelX + 24.f, PanelY + 50.f, 0.9f, 0.9f, TextPrimary);
        Canvas->DrawText(GEngine->GetMediumFont(), SelectedVenue.Headline, PanelX + 24.f, PanelY + 96.f, 1.f, 1.f, TextPrimary);
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Time, PanelX + 24.f, PanelY + 142.f, 1.f, 1.f, TextSecondary);
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Distance, PanelX + 24.f, PanelY + 172.f, 1.f, 1.f, TextSecondary);
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.Presence, PanelX + 24.f, PanelY + 202.f, 1.f, 1.f, TextSecondary);
        Canvas->DrawText(GEngine->GetSmallFont(), SelectedVenue.AgeRule, PanelX + 24.f, PanelY + 246.f, 1.f, 1.f, TextSecondary);

        DrawRect(ButtonSecondary, PanelX + 24.f, PanelY + 262.f, 108.f, 30.f);

        Canvas->DrawText(
            GEngine->GetSmallFont(),
            TEXT("BACK"),
            PanelX + 56.f,
            PanelY + 269.f,
            1.f, 1.f,
            FLinearColor::White
        );
    }

    DrawRect(PanelStrong, 0, H - 54, W, 54);

    Canvas->DrawText(
        GEngine->GetSmallFont(),
        TEXT("WASD Move | Mouse Look | Click Explore | Esc Close"),
        32,
        H - 35,
        1.f, 1.f,
        TextSecondary
    );
}
