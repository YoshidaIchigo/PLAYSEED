#include "ThunderMeadowStage.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderMeadowStage::AThunderMeadowStage()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MeadowRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PaletteAsset(TEXT("/Game/Windseed/M_Palette.M_Palette"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyAsset(TEXT("/Game/Windseed/M_Sky.M_Sky"));
    Cube = CubeAsset.Object;
    Sphere = SphereAsset.Object;
    Cylinder = CylinderAsset.Object;
    Palette = PaletteAsset.Object;
    SkyMaterial = SkyAsset.Object;
}

UStaticMeshComponent* AThunderMeadowStage::AddShape(UStaticMesh* Mesh, FVector Position, FVector Dimensions, FLinearColor Color, bool bSolid)
{
    auto* Part = NewObject<UStaticMeshComponent>(this);
    Part->SetupAttachment(RootComponent);
    Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Position);
    // Engine 基本形状は一辺/直径100cm。呼出し側は読める寸法cmで指定します。
    Part->SetRelativeScale3D(Dimensions / 100.f);
    Part->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Part->SetCollisionResponseToAllChannels(ECR_Block);
    if (Palette)
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), Color);
        Part->SetMaterial(0, Material);
    }
    // NewObject の Outer は this。登録により Actor の構成部品として描画・衝突へ参加します。
    Part->RegisterComponent();
    return Part;
}

void AThunderMeadowStage::AddTerrace(FVector TopCenter, FVector2D Size, FLinearColor GrassColor)
{
    const float Height = TopCenter.Z + 160;
    AddShape(Cube, TopCenter - FVector(0, 0, Height * .5f), FVector(Size.X, Size.Y, Height), FLinearColor(.17f, .24f, .24f), true);
    AddShape(Cube, TopCenter - FVector(0, 0, 6), FVector(Size.X, Size.Y, 12), GrassColor, true);
}

void AThunderMeadowStage::AddTree(FVector Position)
{
    // ここでは装飾です。登れる木の入力・衝突は後続の専用機能として追加します。
    AddShape(Cylinder, Position + FVector(0, 0, 110), FVector(32, 32, 220), FLinearColor(.28f, .18f, .1f));
    AddShape(Sphere, Position + FVector(0, 0, 270), FVector(250, 230, 230), FLinearColor(.06f, .3f, .16f));
    AddShape(Sphere, Position + FVector(45, 0, 355), FVector(180, 185, 170), FLinearColor(.15f, .42f, .2f));
}

void AThunderMeadowStage::AddSign(FVector Position, const FString& Text)
{
    AddShape(Cylinder, Position + FVector(0, 0, 60), FVector(12, 12, 120), FLinearColor(.3f, .2f, .12f));
    auto* Label = NewObject<UTextRenderComponent>(this);
    Label->SetupAttachment(RootComponent);
    Label->SetRelativeLocation(Position + FVector(0, 0, 150));
    Label->SetRelativeRotation(FRotator(0, 180, 0));
    Label->SetText(FText::FromString(Text));
    Label->SetWorldSize(28);
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetTextRenderColor(FColor(255, 235, 180));
    Label->RegisterComponent();
    Signs.Add(Label);
}

void AThunderMeadowStage::BuildMeadow()
{
    AddTerrace(FVector::ZeroVector, FVector2D(7000, 6500), FLinearColor(.06f, .34f, .14f));
    AddTerrace(FVector(-1350, 450, 100), FVector2D(1100, 1100), FLinearColor(.1f, .4f, .19f));
    AddTerrace(FVector(-1750, 1400, 200), FVector2D(1000, 1000), FLinearColor(.16f, .46f, .23f));
    AddTerrace(FVector(-700, 1800, 300), FVector2D(850, 850), FLinearColor(.25f, .44f, .3f));
    for (const FVector& Position : {FVector(-2500, -2000, 0), FVector(700, -2000, 0), FVector(-2600, 2400, 0), FVector(400, 2400, 0), FVector(2900, -1000, 0)})
        AddTree(Position);
    AddSign(FVector(-300, -200, 0), TEXT("THUNDER MEADOW\nNorth-west: the terrace trail"));
    AddSign(FVector(-700, 1920, 300), TEXT("THE MOSS OVERLOOK"));
}

void AThunderMeadowStage::BuildLighting()
{
    auto* SkyDome = AddShape(Sphere, FVector::ZeroVector, FVector(40000), FLinearColor::White);
    SkyDome->SetMaterial(0, SkyMaterial);
    SkyDome->SetCastShadow(false);
    // 光源は World が所有し、レベル退出時に地形と一緒に破棄されます。
    auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0, 0, 1500), FRotator(-50, -30, 0));
    Sun->GetLightComponent()->SetIntensity(3);
    auto* Fill = GetWorld()->SpawnActor<ASkyLight>();
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->bLowerHemisphereIsBlack = false;
    Fill->GetLightComponent()->SetIntensity(1.2f);
    Fill->GetLightComponent()->SetRealTimeCaptureEnabled(true);
    auto* Exposure = GetWorld()->SpawnActor<APostProcessVolume>();
    Exposure->bUnbound = true;
    Exposure->Settings.bOverride_AutoExposureMethod = true;
    Exposure->Settings.AutoExposureMethod = AEM_Manual;
    Exposure->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    Exposure->Settings.AutoExposureApplyPhysicalCameraExposure = false;
}

void AThunderMeadowStage::BeginPlay()
{
    Super::BeginPlay();
    BuildMeadow();
    BuildLighting();
}

void AThunderMeadowStage::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (auto* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
    {
        // 文字の裏面を見せると鏡文字になるため、案内だけをカメラへ向けます。
        // 地形や看板の柱は回さず、文字は垂直に保ちます。
        for (UTextRenderComponent* Sign : Signs)
        {
            const float Yaw = (Camera->GetCameraLocation() - Sign->GetComponentLocation()).Rotation().Yaw;
            Sign->SetWorldRotation(FRotator(0, Yaw, 0));
        }
    }
}
