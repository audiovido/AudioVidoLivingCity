// AVCityBlock.cpp

#include "AVCityBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/UObjectGlobals.h"

AAVCityBlock::AAVCityBlock()
{
    PrimaryActorTick.bCanEverTick = false;
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("CityRoot"));
    Root->SetMobility(EComponentMobility::Static);
    SetRootComponent(Root);
}

UStaticMeshComponent* AAVCityBlock::AddMesh(const FString& Name, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, USceneComponent* Parent, FName Tag)
{
    UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, *Name);
    Mesh->SetupAttachment(Parent ? Parent : Root);
    Mesh->SetRelativeLocation(Loc);
    Mesh->SetRelativeScale3D(Scale);
    Mesh->SetMobility(EComponentMobility::Static);
    Mesh->SetCollisionProfileName(Tag.IsNone() ? TEXT("BlockAll") : TEXT("BlockAllDynamic"));
    static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    static UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    Mesh->SetStaticMesh(Cube);
    if (BaseMat)
    {
        UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(BaseMat, this);
        Mid->SetVectorParameterValue(TEXT("Color"), Color);
        Mesh->SetMaterial(0, Mid);
    }
    if (!Tag.IsNone()) Mesh->ComponentTags.Add(Tag);
    Mesh->RegisterComponent();
    return Mesh;
}

UTextRenderComponent* AAVCityBlock::AddLabel(const FString& Text, const FVector& Loc, const FRotator& Rot, float Size, const FLinearColor& Color, USceneComponent* Parent)
{
    UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
    Label->SetupAttachment(Parent ? Parent : Root);
    Label->SetRelativeLocation(Loc);
    Label->SetRelativeRotation(Rot);
    Label->SetText(FText::FromString(Text));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(Size);
    Label->SetTextRenderColor(Color.ToFColor(true));
    Label->RegisterComponent();
    return Label;
}

void AAVCityBlock::AddVenue(const FAVVenueData& Data, const FVector& Scale)
{
    const FName Tag = Data.Id;
    AddMesh(Data.Id.ToString() + TEXT("_Shell"), Data.WorldLocation, Scale, FLinearColor(.018f,.025f,.035f), Root, Tag);

    // All facade pieces use city/world-relative coordinates. Parenting them to the
    // already-scaled shell multiplied both their offsets and their size, pushing the
    // visible venue geometry far outside the camera frustum.
    const float Side = Data.WorldLocation.Y > 0.f ? -1.f : 1.f;
    const float FrontY = Data.WorldLocation.Y + Side * (Scale.Y * 50.f + 4.f);
    const FRotator Facing = Data.WorldLocation.Y > 0.f ? FRotator(0,0,0) : FRotator(0,180,0);
    const auto AtFront = [&](float X, float Depth, float Z)
    {
        return FVector(Data.WorldLocation.X + X, FrontY + Side * Depth, Data.WorldLocation.Z + Z);
    };

    AddMesh(Data.Id.ToString()+TEXT("_Entry"), AtFront(0,0,-Scale.Z*12.f), FVector(Scale.X*.58f,.06f,Scale.Z*.62f), Data.Accent*.32f, Root, Tag);
    AddMesh(Data.Id.ToString()+TEXT("_Canopy"), AtFront(0,12.f,Scale.Z*9.f), FVector(Scale.X*.7f,.24f,.05f), Data.Accent, Root, Tag);
    AddLabel(Data.Name, AtFront(0,17.f,Scale.Z*16.f), Facing, 28.f, Data.Accent, Root);
    AddLabel(Data.Headline, AtFront(0,18.f,Scale.Z*4.f), Facing, 15.f, FLinearColor::White, Root);

    if (Data.Type == EAVVenueType::Cinema)
    {
        for(int32 i=-1;i<=1;i++) AddMesh(FString::Printf(TEXT("Poster_%d"),i), AtFront(i*105.f,8.f,-5.f), FVector(.38f,.04f,.68f), i==0?FLinearColor(.85f,.22f,.12f):FLinearColor(.12f,.25f,.42f), Root, Tag);
        AddLabel(TEXT("21:20   23:45"), AtFront(0,20.f,-70.f), Facing, 13.f, FLinearColor(1,.8f,.55f), Root);
    }
    else if (Data.Type == EAVVenueType::MusicBar)
    {
        for(int32 i=-2;i<=2;i++) AddMesh(FString::Printf(TEXT("Beat_%d"),i), AtFront(i*42.f,9.f,-8.f), FVector(.11f,.04f,.18f+FMath::Abs(i)*.05f), i%2?Data.Accent:FLinearColor(.12f,.6f,.85f), Root, Tag);
    }
    else if (Data.Type == EAVVenueType::ConcertHall)
    {
        AddMesh(TEXT("StagePortal"), AtFront(0,11.f,-8.f), FVector(1.7f,.05f,.74f), FLinearColor(.04f,.11f,.2f), Root, Tag);
        for(int32 i=-2;i<=2;i++) AddMesh(FString::Printf(TEXT("StageLight_%d"),i), AtFront(i*65.f,17.f,20.f), FVector(.12f,.04f,.12f), i%2?Data.Accent:FLinearColor(.9f,.2f,.55f), Root);
    }
    else
    {
        for(int32 i=-2;i<=2;i++) AddMesh(FString::Printf(TEXT("SocialWindow_%d"),i), AtFront(i*75.f,8.f,5.f), FVector(.28f,.04f,.48f), FLinearColor(.12f,.26f,.22f), Root, Tag);
        AddLabel(TEXT("FILM  +  SOUND  CIRCLE"), AtFront(0,20.f,-58.f), Facing, 12.f, FLinearColor(.65f,1.f,.84f), Root);
    }
    UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
    Light->SetupAttachment(Root); Light->SetRelativeLocation(AtFront(0,0,-25.f)); Light->SetLightColor(Data.Accent.ToFColor(true)); Light->SetIntensity(2400.f); Light->SetAttenuationRadius(520.f); Light->RegisterComponent();
}

void AAVCityBlock::AddStreetFurniture()
{
    AddMesh(TEXT("Road"), FVector(0,0,-32), FVector(17,4.8f,.12f), FLinearColor(.015f,.019f,.024f));
    AddMesh(TEXT("CrossStreet"), FVector(0,0,-30), FVector(3.7f,17,.11f), FLinearColor(.013f,.017f,.022f));
    for(int32 i=-6;i<=6;i++)
    {
        AddMesh(FString::Printf(TEXT("Lane_%d"),i), FVector(i*230.f,0,-18), FVector(.55f,.035f,.015f), FLinearColor(.55f,.48f,.31f));
        if(i%2==0)
        {
            for(int32 side : {-1,1})
            {
                UStaticMeshComponent* Pole=AddMesh(FString::Printf(TEXT("Lamp_%d_%d"),i,side), FVector(i*220.f,side*250.f,100), FVector(.04f,.04f,1.4f), FLinearColor(.06f,.07f,.08f));
                UPointLightComponent* L=NewObject<UPointLightComponent>(this); L->SetupAttachment(Pole); L->SetRelativeLocation(FVector(0,0,55)); L->SetLightColor(FColor(255,188,118)); L->SetIntensity(950); L->SetAttenuationRadius(430); L->RegisterComponent();
            }
        }
    }
    for(int32 i=0;i<28;i++)
    {
        const float A = (i/28.f)*PI*2.f;
        FVector P(FMath::Cos(A)*FMath::RandRange(250.f,950.f),FMath::Sin(A)*FMath::RandRange(180.f,370.f),18.f);
        AddMesh(FString::Printf(TEXT("Crowd_%d"),i), P, FVector(.07f,.07f,.32f), FLinearColor(.035f,.045f,.055f));
    }
    USkyLightComponent* Sky=NewObject<USkyLightComponent>(this); Sky->SetupAttachment(Root); Sky->SetIntensity(.35f); Sky->RegisterComponent();
    UExponentialHeightFogComponent* Fog=NewObject<UExponentialHeightFogComponent>(this); Fog->SetupAttachment(Root); Fog->SetFogDensity(.012f); Fog->SetFogHeightFalloff(.22f); Fog->SetFogInscatteringColor(FLinearColor(.025f,.035f,.06f)); Fog->RegisterComponent();
}

void AAVCityBlock::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (GetComponents().Num() > 1) return;
    Venues = MakeAudioVidoDemoData();
    AddStreetFurniture();
    for (const auto& Venue : Venues)
    {
        AddVenue(Venue, FVector(5.f, 5.f, 5.f));
    }
}