#include "WindseedStage.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AWindseedStage::AWindseedStage()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GardenRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeAsset(TEXT("/Engine/BasicShapes/Cone.Cone"));
    Cube = CubeAsset.Object; Sphere = SphereAsset.Object; Cylinder = CylinderAsset.Object; Cone = ConeAsset.Object;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PaletteAsset(TEXT("/Game/Windseed/M_Palette.M_Palette"));
    Palette = PaletteAsset.Object;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyAsset(TEXT("/Game/Windseed/M_Sky.M_Sky"));
    SkyMaterial = SkyAsset.Object;
}

UStaticMeshComponent* AWindseedStage::Shape(UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color, bool bSolid)
{
    auto* Part = NewObject<UStaticMeshComponent>(this);
    Part->SetupAttachment(RootComponent);
    Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Part->SetCollisionResponseToAllChannels(ECR_Block);
    if (Palette)
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), Color);
        Part->SetMaterial(0, Material);
    }
    Part->RegisterComponent();
    return Part;
}

void AWindseedStage::Island(FVector Center, FVector2D Size)
{
    Shape(Cube, Center-FVector(0,0,70), FVector(Size.X/100,Size.Y/100,1.4f), FLinearColor(.12f,.35f,.37f), true);
    Shape(Cube, Center-FVector(0,0,6), FVector(Size.X/100,Size.Y/100,.12f), FLinearColor(.34f,.72f,.45f), true);
    auto* Rock = Shape(Cone, Center-FVector(0,0,250), FVector(Size.X/110,Size.Y/110,3.8f), FLinearColor(.09f,.23f,.28f));
    Rock->SetRelativeRotation(FRotator(180,0,0));
    for (int32 I=0; I<4; ++I)
    {
        const float X = I<2 ? -Size.X*.36f : Size.X*.36f;
        const float Y = I%2 ? -Size.Y*.36f : Size.Y*.36f;
        Plant(Center+FVector(X,Y,0), .65f);
    }
}

void AWindseedStage::Plant(FVector Position, float Scale)
{
    Shape(Cylinder, Position+FVector(0,0,36*Scale), FVector(.13f,.13f,.72f)*Scale, FLinearColor(.13f,.32f,.18f));
    Shape(Sphere, Position+FVector(0,0,90*Scale), FVector(.75f,.65f,.8f)*Scale, FLinearColor(.13f,.52f,.29f));
    Shape(Sphere, Position+FVector(20,-10,115)*Scale, FVector(.6f,.55f,.65f)*Scale, FLinearColor(.3f,.68f,.36f));
}

void AWindseedStage::Sign(FVector Position, const FString& Text, float Yaw)
{
    Shape(Cylinder, Position+FVector(0,0,50), FVector(.1f,.1f,1), FLinearColor(.28f,.18f,.12f));
    auto* Label = NewObject<UTextRenderComponent>(this);
    Label->SetupAttachment(RootComponent);
    Label->SetRelativeLocation(Position+FVector(0,0,125));
    Label->SetRelativeRotation(FRotator(0,Yaw,0));
    Label->SetText(FText::FromString(Text));
    Label->SetWorldSize(28);
    Label->SetTextRenderColor(FColor(255,235,176));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->RegisterComponent();
}

void AWindseedStage::BeginPlay()
{
    Super::BeginPlay();
    auto* SkyDome = Shape(Sphere, FVector(0,0,0), FVector(400,400,400), FLinearColor::White);
    if (SkyMaterial) SkyDome->SetMaterial(0, SkyMaterial);
    SkyDome->SetCastShadow(false);
    Island(FVector(0,0,0), FVector2D(1800,1800));
    Island(FVector(0,1100,100), FVector2D(580,580));
    Island(FVector(400,1450,200), FVector2D(580,580));
    Island(FVector(1050,100,100), FVector2D(580,580));
    Island(FVector(1500,-100,200), FVector2D(580,580));
    Island(FVector(1950,150,300), FVector2D(580,580));
    Island(GoalPosition, FVector2D(650,650));
    SeedPositions = { FVector(-100,0,85), FVector(0,1100,185), FVector(400,1450,285),
        FVector(1050,100,185), FVector(1500,-100,285), FVector(1950,150,385) };
    for (const FVector& Position : SeedPositions)
    {
        auto* Seed = Shape(Sphere, Position, FVector(.34f,.34f,.65f), FLinearColor(1,.68f,.12f));
        Seeds.Add(Seed);
        Collected.Add(false);
    }
    Shape(Cylinder, GoalPosition+FVector(0,0,12), FVector(2.6f,2.6f,.24f), FLinearColor(.85f,.75f,.5f), true);
    Shape(Cylinder, GoalPosition+FVector(0,0,130), FVector(.65f,.65f,2.2f), FLinearColor(.15f,.3f,.4f), true);
    Beacon = Shape(Sphere, GoalPosition+FVector(0,0,270), FVector(1.15f,1.15f,1.3f), FLinearColor(.2f,.37f,.45f));
    for (int32 I=0; I<6; ++I)
    {
        const float Angle = I*PI/3;
        Shape(Sphere, GoalPosition+FVector(FMath::Cos(Angle)*100,FMath::Sin(Angle)*100,230), FVector(.65f,.32f,.18f), FLinearColor(.95f,.67f,.23f));
    }
    Sign(FVector(-500,100,0), TEXT("WINDSEED GARDEN\n6 seeds awaken the beacon"));
    Sign(FVector(-80,650,0), TEXT("OVERLOOK  >"), -90);
    Sign(FVector(700,-200,0), TEXT("BEACON TRAIL  >"));
    Sign(GoalPosition+FVector(0,-180,0), TEXT("THE WIND BEACON"));
    for (int32 I=0; I<12; ++I)
    {
        const float A = I*PI/6;
        Shape(Sphere, FVector(FMath::Cos(A)*3600,FMath::Sin(A)*3600,-450), FVector(12,7,3), FLinearColor(.7f,.86f,.87f));
    }
    auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,1500), FRotator(-48,-35,0));
    auto* SunComponent = Sun->FindComponentByClass<UDirectionalLightComponent>();
    SunComponent->SetIntensity(3.0f);
    SunComponent->SetAtmosphereSunLight(true);
    auto* Atmosphere = GetWorld()->SpawnActor<AActor>();
    auto* AtmosphereComponent = NewObject<USkyAtmosphereComponent>(Atmosphere);
    Atmosphere->SetRootComponent(AtmosphereComponent);
    AtmosphereComponent->RegisterComponent();
    auto* Sky = GetWorld()->SpawnActor<ASkyLight>();
    Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->bLowerHemisphereIsBlack = false;
    Sky->GetLightComponent()->SetIntensity(1.2f);
    Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);
    auto* Exposure = GetWorld()->SpawnActor<APostProcessVolume>();
    Exposure->bUnbound = true;
    Exposure->Settings.bOverride_AutoExposureMethod = true;
    Exposure->Settings.AutoExposureMethod = AEM_Manual;
    Exposure->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    Exposure->Settings.AutoExposureApplyPhysicalCameraExposure = false;
    UE_LOG(LogTemp, Display, TEXT("WINDSEED: Stage ready, six seeds, seven islands."));
}

void AWindseedStage::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!Player) return;
    const float Time = GetWorld()->GetTimeSeconds();
    for (int32 I=0; I<Seeds.Num(); ++I)
    {
        if (Collected[I]) continue;
        Seeds[I]->SetRelativeLocation(SeedPositions[I]+FVector(0,0,FMath::Sin(Time*2.8f+I)*12));
        Seeds[I]->AddLocalRotation(FRotator(0,DeltaSeconds*90,0));
        if (FVector::DistSquared(Player->GetActorLocation(),SeedPositions[I]) < FMath::Square(110.f))
        {
            Collected[I] = true;
            Seeds[I]->SetVisibility(false);
            ++CollectedCount;
            Hint = CollectedCount==SeedCount ? TEXT("All six found! Reach the flower beacon.") : TEXT("Windseed found! Explore the overlook and the beacon trail.");
            UE_LOG(LogTemp, Display, TEXT("WINDSEED: Collected %d/%d (seed %d)"),CollectedCount,SeedCount,I+1);
        }
    }
    if (CollectedCount==SeedCount && !bFinished)
    {
        if (auto* Material = Cast<UMaterialInstanceDynamic>(Beacon->GetMaterial(0)))
            Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1,.8f,.15f));
    }
    if (FVector::Dist2D(Player->GetActorLocation(), GoalPosition)<150 && FMath::Abs(Player->GetActorLocation().Z-GoalPosition.Z-100)<150)
    {
        if (CollectedCount==SeedCount && !bFinished)
        {
            bFinished = true;
            FinishedAt = Time;
            Hint = TEXT("The garden is awake. Thank you, little seed!");
            UE_LOG(LogTemp, Display, TEXT("WINDSEED: GOAL COMPLETE"));
        }
        else if (!bFinished) Hint = FString::Printf(TEXT("The beacon needs %d more windseeds."),SeedCount-CollectedCount);
    }
    if (bFinished)
    {
        Beacon->SetRelativeScale3D(FVector(1.15f,1.15f,1.3f)*(1+.08f*FMath::Sin(Time*4)));
        Beacon->AddLocalRotation(FRotator(0,DeltaSeconds*45,0));
    }
}

void AWindseedStage::ResetTrail()
{
    CollectedCount=0; bFinished=false; FinishedAt=0;
    Hint=TEXT("Find all six windseeds. Follow the floating gardens.");
    for (int32 I=0;I<Seeds.Num();++I) { Collected[I]=false; Seeds[I]->SetVisibility(true); }
    Beacon->SetRelativeScale3D(FVector(1.15f,1.15f,1.3f));
    if (auto* Material=Cast<UMaterialInstanceDynamic>(Beacon->GetMaterial(0))) Material->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.2f,.37f,.45f));
}
