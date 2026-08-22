#pragma once

#include "CoreMinimal.h"
#include "AVVenueData.generated.h"

UENUM()
enum class EAVVenueType : uint8 { Cinema, MusicBar, ConcertHall, SocialClub };

USTRUCT()
struct FAVVenueData
{
    GENERATED_BODY()
    UPROPERTY() FName Id;
    UPROPERTY() EAVVenueType Type = EAVVenueType::Cinema;
    UPROPERTY() FString Name;
    UPROPERTY() FString Eyebrow;
    UPROPERTY() FString Headline;
    UPROPERTY() FString Time;
    UPROPERTY() FString Distance;
    UPROPERTY() FString Presence;
    UPROPERTY() FString AgeRule;
    UPROPERTY() FLinearColor Accent = FLinearColor::White;
    UPROPERTY() FVector WorldLocation = FVector::ZeroVector;
};

inline TArray<FAVVenueData> MakeAudioVidoDemoData()
{
    return {
        {TEXT("cinema"), EAVVenueType::Cinema, TEXT("LUMIERE CINEMA"), TEXT("NOW SHOWING"), TEXT("The Last Horizon"), TEXT("21:20  |  23:45"), TEXT("350 m"), TEXT("42 interested nearby"), TEXT("12+"), FLinearColor(1.0f,.54f,.25f), FVector(-720, 420, 170)},
        {TEXT("bar"), EAVVenueType::MusicBar, TEXT("VELVET ROOM"), TEXT("LIVE TONIGHT"), TEXT("NORA K.  /  Deep House"), TEXT("22:00 — 02:00"), TEXT("480 m"), TEXT("28 music lovers nearby"), TEXT("18+"), FLinearColor(.75f,.22f,.82f), FVector(700, 430, 170)},
        {TEXT("concert"), EAVVenueType::ConcertHall, TEXT("THE FORUM"), TEXT("DOORS OPEN 20:30"), TEXT("ARCADE SUN — Live"), TEXT("Starts in 48 min"), TEXT("620 m"), TEXT("126 attending tonight"), TEXT("16+"), FLinearColor(.16f,.64f,1.0f), FVector(-700, -500, 190)},
        {TEXT("social"), EAVVenueType::SocialClub, TEXT("COMMON GROUND"), TEXT("SOCIAL CLUB"), TEXT("Film & Sound Circle"), TEXT("Planning table 21:00"), TEXT("210 m"), TEXT("16 people in this area"), TEXT("Community rules apply"), FLinearColor(.25f,.85f,.66f), FVector(690, -480, 150)}
    };
}
