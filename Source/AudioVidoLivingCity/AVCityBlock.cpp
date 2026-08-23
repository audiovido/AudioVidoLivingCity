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

    // Distinct massing per venue type. WorldLocation stays authoritative.
    FVector ShellScale = Scale;
    FLinearColor ShellColor(.035f, .045f, .060f);
    FLinearColor Secondary(.075f, .090f, .115f);

    if (Data.Type == EAVVenueType::Cinema)
    {
        ShellScale = FVector(4.2f, 2.8f, 3.6f);
        ShellColor = FLinearColor(.045f, .050f, .065f);
        Secondary = FLinearColor(.12f, .075f, .055f);
    }
    else if (Data.Type == EAVVenueType::MusicBar)
    {
        ShellScale = FVector(3.0f, 2.7f, 3.8f);
        ShellColor = FLinearColor(.050f, .030f, .060f);
        Secondary = FLinearColor(.12f, .045f, .13f);
    }
    else if (Data.Type == EAVVenueType::ConcertHall)
    {
        ShellScale = FVector(4.5f, 3.0f, 4.0f);
        ShellColor = FLinearColor(.030f, .045f, .065f);
        Secondary = FLinearColor(.045f, .12f, .18f);
    }
    else
    {
        ShellScale = FVector(3.8f, 2.6f, 3.5f);
        ShellColor = FLinearColor(.035f, .060f, .055f);
        Secondary = FLinearColor(.055f, .15f, .12f);
    }

    AddMesh(
        Data.Id.ToString() + TEXT("_Shell"),
        Data.WorldLocation,
        ShellScale,
        ShellColor,
        Root,
        Tag
    );

    // Fronts face toward the center of the district.
    const float Side = Data.WorldLocation.Y > 0.f ? -1.f : 1.f;
    const float FrontY =
        Data.WorldLocation.Y
        + Side * (ShellScale.Y * 50.f + 4.f);

    const FRotator Facing =
        Data.WorldLocation.Y > 0.f
        ? FRotator(0, 0, 0)
        : FRotator(0, 180, 0);

    const auto AtFront = [&](float X, float Depth, float Z)
    {
        return FVector(
            Data.WorldLocation.X + X,
            FrontY + Side * Depth,
            Data.WorldLocation.Z + Z
        );
    };

    // Small public forecourt. Intentionally untagged.
    AddMesh(
        Data.Id.ToString() + TEXT("_Plaza"),
        FVector(
            Data.WorldLocation.X,
            FrontY + Side * 105.f,
            -19.f
        ),
        FVector(ShellScale.X * .82f, 1.15f, .035f),
        FLinearColor(.105f, .115f, .125f),
        Root
    );

    // General facade frame and entry hierarchy.
    AddMesh(
        Data.Id.ToString() + TEXT("_FacadeBand"),
        AtFront(0.f, 5.f, 25.f),
        FVector(ShellScale.X * .90f, .055f, .16f),
        Secondary,
        Root,
        Tag
    );

    AddMesh(
        Data.Id.ToString() + TEXT("_Entry"),
        AtFront(0.f, 2.f, -ShellScale.Z * 11.f),
        FVector(ShellScale.X * .48f, .07f, ShellScale.Z * .56f),
        Data.Accent * .42f,
        Root,
        Tag
    );

    AddMesh(
        Data.Id.ToString() + TEXT("_Canopy"),
        AtFront(0.f, 18.f, ShellScale.Z * 9.f),
        FVector(ShellScale.X * .72f, .28f, .07f),
        Data.Accent,
        Root,
        Tag
    );

    AddLabel(
        Data.Name,
        AtFront(0.f, 22.f, ShellScale.Z * 16.f),
        Facing,
        28.f,
        Data.Accent,
        Root
    );

    AddLabel(
        Data.Headline,
        AtFront(0.f, 23.f, ShellScale.Z * 5.f),
        Facing,
        15.f,
        FLinearColor(.90f, .93f, .98f),
        Root
    );

    // --------------------------------------------------------
    // CINEMA: broad marquee + poster rhythm
    // --------------------------------------------------------
    if (Data.Type == EAVVenueType::Cinema)
    {
        AddMesh(
            Data.Id.ToString() + TEXT("_Marquee"),
            AtFront(0.f, 24.f, 72.f),
            FVector(1.75f, .32f, .15f),
            Data.Accent * .80f,
            Root,
            Tag
        );

        for (int32 i = -1; i <= 1; ++i)
        {
            AddMesh(
                FString::Printf(TEXT("%s_Poster_%d"), *Data.Id.ToString(), i),
                AtFront(i * 105.f, 10.f, -8.f),
                FVector(.38f, .05f, .68f),
                i == 0
                    ? FLinearColor(.85f, .22f, .12f)
                    : FLinearColor(.12f, .25f, .42f),
                Root,
                Tag
            );
        }

        AddLabel(
            TEXT("21:20   23:45"),
            AtFront(0.f, 25.f, -70.f),
            Facing,
            13.f,
            FLinearColor(1.f, .80f, .55f),
            Root
        );
    }

    // --------------------------------------------------------
    // MUSIC BAR: vertical club identity + rhythmic fins
    // --------------------------------------------------------
    else if (Data.Type == EAVVenueType::MusicBar)
    {
        AddMesh(
            Data.Id.ToString() + TEXT("_VerticalSign"),
            AtFront(-112.f, 10.f, 70.f),
            FVector(.22f, .07f, 1.25f),
            Data.Accent,
            Root,
            Tag
        );

        for (int32 i = -2; i <= 2; ++i)
        {
            AddMesh(
                FString::Printf(TEXT("%s_Beat_%d"), *Data.Id.ToString(), i),
                AtFront(i * 42.f, 11.f, -6.f),
                FVector(
                    .11f,
                    .05f,
                    .20f + FMath::Abs(i) * .08f
                ),
                i % 2
                    ? Data.Accent
                    : FLinearColor(.12f, .60f, .85f),
                Root,
                Tag
            );
        }
    }

    // --------------------------------------------------------
    // CONCERT HALL: symmetric civic portal + crown
    // --------------------------------------------------------
    else if (Data.Type == EAVVenueType::ConcertHall)
    {
        AddMesh(
            Data.Id.ToString() + TEXT("_Crown"),
            AtFront(0.f, -10.f, 150.f),
            FVector(1.75f, .55f, .20f),
            Secondary,
            Root,
            Tag
        );

        AddMesh(
            Data.Id.ToString() + TEXT("_StagePortal"),
            AtFront(0.f, 13.f, -5.f),
            FVector(1.65f, .07f, .82f),
            FLinearColor(.045f, .11f, .20f),
            Root,
            Tag
        );

        AddMesh(
            Data.Id.ToString() + TEXT("_PylonL"),
            AtFront(-175.f, 8.f, 18.f),
            FVector(.30f, .08f, 1.15f),
            Secondary,
            Root,
            Tag
        );

        AddMesh(
            Data.Id.ToString() + TEXT("_PylonR"),
            AtFront(175.f, 8.f, 18.f),
            FVector(.30f, .08f, 1.15f),
            Secondary,
            Root,
            Tag
        );

        for (int32 i = -2; i <= 2; ++i)
        {
            AddMesh(
                FString::Printf(TEXT("%s_StageLight_%d"), *Data.Id.ToString(), i),
                AtFront(i * 65.f, 19.f, 28.f),
                FVector(.12f, .05f, .12f),
                i % 2
                    ? Data.Accent
                    : FLinearColor(.90f, .20f, .55f),
                Root
            );
        }
    }

    // --------------------------------------------------------
    // COMMON GROUND: glazing rhythm + low welcoming wings
    // --------------------------------------------------------
    else
    {
        AddMesh(
            Data.Id.ToString() + TEXT("_WingL"),
            AtFront(-150.f, -15.f, -35.f),
            FVector(.65f, .42f, .65f),
            FLinearColor(.055f, .10f, .085f),
            Root,
            Tag
        );

        AddMesh(
            Data.Id.ToString() + TEXT("_WingR"),
            AtFront(150.f, -15.f, -35.f),
            FVector(.65f, .42f, .65f),
            FLinearColor(.055f, .10f, .085f),
            Root,
            Tag
        );

        for (int32 i = -2; i <= 2; ++i)
        {
            AddMesh(
                FString::Printf(TEXT("%s_SocialWindow_%d"), *Data.Id.ToString(), i),
                AtFront(i * 68.f, 10.f, 8.f),
                FVector(.25f, .05f, .48f),
                FLinearColor(.10f, .28f, .23f),
                Root,
                Tag
            );
        }

        AddLabel(
            TEXT("FILM  +  SOUND  CIRCLE"),
            AtFront(0.f, 24.f, -58.f),
            Facing,
            12.f,
            FLinearColor(.65f, 1.f, .84f),
            Root
        );
    }

    // Venue frontage accent light.
    UPointLightComponent* Light =
        NewObject<UPointLightComponent>(
            this,
            *FString::Printf(
                TEXT("%s_FrontLight"),
                *Data.Id.ToString()
            )
        );

    Light->SetupAttachment(Root);
    Light->SetRelativeLocation(
        AtFront(0.f, 95.f, 35.f)
    );
    Light->SetLightColor(Data.Accent.ToFColor(true));
    Light->SetIntensity(1750.f);
    Light->SetAttenuationRadius(480.f);
    Light->RegisterComponent();
}

void AAVCityBlock::AddStreetFurniture()
{
    const FLinearColor RoadColor(.020f, .025f, .032f);
    const FLinearColor WalkColor(.095f, .105f, .115f);
    const FLinearColor CurbColor(.15f, .16f, .17f);

    AddMesh(
        TEXT("Road"),
        FVector(0, 0, -32),
        FVector(17.f, 4.8f, .12f),
        RoadColor
    );

    AddMesh(
        TEXT("CrossStreet"),
        FVector(0, 0, -30),
        FVector(3.7f, 17.f, .11f),
        FLinearColor(.017f, .022f, .029f)
    );

    // Pedestrian strips around the two crossing roads.
    AddMesh(
        TEXT("SidewalkNorth"),
        FVector(0, 315.f, -20.f),
        FVector(17.f, .62f, .055f),
        WalkColor
    );

    AddMesh(
        TEXT("SidewalkSouth"),
        FVector(0, -315.f, -20.f),
        FVector(17.f, .62f, .055f),
        WalkColor
    );

    AddMesh(
        TEXT("SidewalkEast"),
        FVector(250.f, 0, -19.f),
        FVector(.55f, 17.f, .052f),
        WalkColor
    );

    AddMesh(
        TEXT("SidewalkWest"),
        FVector(-250.f, 0, -19.f),
        FVector(.55f, 17.f, .052f),
        WalkColor
    );

    AddMesh(
        TEXT("CurbNorth"),
        FVector(0, 260.f, -16.f),
        FVector(17.f, .045f, .075f),
        CurbColor
    );

    AddMesh(
        TEXT("CurbSouth"),
        FVector(0, -260.f, -16.f),
        FVector(17.f, .045f, .075f),
        CurbColor
    );

    // Lane markers + street lights.
    for (int32 i = -6; i <= 6; ++i)
    {
        AddMesh(
            FString::Printf(TEXT("Lane_%d"), i),
            FVector(i * 230.f, 0, -18.f),
            FVector(.55f, .035f, .015f),
            FLinearColor(.62f, .54f, .32f)
        );

        if (i % 2 == 0)
        {
            for (int32 side : {-1, 1})
            {
                UStaticMeshComponent* Pole =
                    AddMesh(
                        FString::Printf(
                            TEXT("Lamp_%d_%d"),
                            i,
                            side
                        ),
                        FVector(
                            i * 220.f,
                            side * 250.f,
                            100.f
                        ),
                        FVector(.04f, .04f, 1.4f),
                        FLinearColor(.075f, .080f, .090f)
                    );

                UPointLightComponent* L =
                    NewObject<UPointLightComponent>(this);

                L->SetupAttachment(Pole);
                L->SetRelativeLocation(
                    FVector(0, 0, 55.f)
                );
                L->SetLightColor(
                    FColor(255, 188, 118)
                );
                L->SetIntensity(900.f);
                L->SetAttenuationRadius(420.f);
                L->RegisterComponent();
            }
        }
    }

    // Lightweight crowd markers.
    for (int32 i = 0; i < 28; ++i)
    {
        const float A =
            (i / 28.f) * PI * 2.f;

        FVector P(
            FMath::Cos(A) *
                FMath::RandRange(250.f, 950.f),
            FMath::Sin(A) *
                FMath::RandRange(180.f, 370.f),
            18.f
        );

        AddMesh(
            FString::Printf(TEXT("Crowd_%d"), i),
            P,
            FVector(.07f, .07f, .32f),
            FLinearColor(.055f, .065f, .075f)
        );
    }

    USkyLightComponent* Sky =
        NewObject<USkyLightComponent>(this);

    Sky->SetupAttachment(Root);
    Sky->SetIntensity(.42f);
    Sky->RegisterComponent();

    UExponentialHeightFogComponent* Fog =
        NewObject<UExponentialHeightFogComponent>(this);

    Fog->SetupAttachment(Root);
    Fog->SetFogDensity(.010f);
    Fog->SetFogHeightFalloff(.22f);
    Fog->SetFogInscatteringColor(
        FLinearColor(.030f, .040f, .065f)
    );
    Fog->RegisterComponent();
}

void AAVCityBlock::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (GetComponents().Num() > 1) return;
    Venues = MakeAudioVidoDemoData();
    AddStreetFurniture();
    for (const auto& Venue : Venues)
    {
        AddVenue(Venue, FVector(3.6f, 3.2f, 3.8f));
    }
}