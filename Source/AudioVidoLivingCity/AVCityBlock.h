#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AVVenueData.h"
#include "AVCityBlock.generated.h"

UCLASS()
class AUDIOVIDOLIVINGCITY_API AAVCityBlock : public AActor
{
    GENERATED_BODY()
public:
    AAVCityBlock();
    virtual void OnConstruction(const FTransform& Transform) override;
    const TArray<FAVVenueData>& GetVenues() const { return Venues; }
private:
    UPROPERTY() USceneComponent* Root;
    UPROPERTY() TArray<FAVVenueData> Venues;
    UStaticMeshComponent* AddMesh(const FString& Name, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, USceneComponent* Parent = nullptr, FName Tag = NAME_None);
    class UTextRenderComponent* AddLabel(const FString& Text, const FVector& Loc, const FRotator& Rot, float Size, const FLinearColor& Color, USceneComponent* Parent = nullptr);
    void AddVenue(const FAVVenueData& Data, const FVector& Scale);
    void AddStreetFurniture();
};
