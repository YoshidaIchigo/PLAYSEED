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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeAsset(TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PaletteAsset(TEXT("/Game/Windseed/M_Palette.M_Palette"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyAsset(TEXT("/Game/Windseed/M_Sky.M_Sky"));
    Cube = CubeAsset.Object;
    Sphere = SphereAsset.Object;
    Cylinder = CylinderAsset.Object;
    Cone = ConeAsset.Object;
    Palette = PaletteAsset.Object;
    SkyMaterial = SkyAsset.Object;
}

UStaticMeshComponent* AThunderMeadowStage::AddShape(UStaticMesh* Mesh, FVector Position, FVector Dimensions, FLinearColor Color, bool bSolid, FRotator Rotation)
{
    auto* Part = NewObject<UStaticMeshComponent>(this);
    Part->SetupAttachment(RootComponent);
    Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeRotation(Rotation);
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

void AThunderMeadowStage::AddRamp(FVector Start, FVector End, float Width)
{
    const FVector Delta = End - Start;
    const FRotator Rotation = Delta.Rotation();
    const FVector Up = Rotation.RotateVector(FVector::UpVector);
    // Start/End を歩く面の高さとし、厚み40cmの中心を法線方向へ20cm下げます。
    // 隣の区間と長さを60cm重ね、曲がり角に足が落ちる隙間を作らないようにします。
    AddShape(Cube, (Start + End) * .5f - Up * 20, FVector(Delta.Size() + 60, Width, 40),
        FLinearColor(.18f, .4f, .24f), true, Rotation);
}

void AThunderMeadowStage::BuildMountain()
{
    const FVector Center(2200, 2100, 0);
    const FLinearColor Rock(.14f, .2f, .25f);
    const FLinearColor Moss(.09f, .27f, .15f);
    // 山塊は上へ行くほど細くし、周回路の内側を支える段丘にします。
    AddShape(Cylinder, Center + FVector(0, 0, 300), FVector(2000, 2000, 600), Rock, true);
    AddShape(Cylinder, Center + FVector(0, 0, 594), FVector(2000, 2000, 12), Moss, true);
    AddShape(Cylinder, Center + FVector(0, 0, 900), FVector(1850, 1850, 600), Rock, true);
    AddShape(Cylinder, Center + FVector(0, 0, 1194), FVector(1850, 1850, 12), Moss, true);
    AddShape(Cylinder, Center + FVector(0, 0, 1540), FVector(1750, 1750, 680), Rock, true);
    AddShape(Cylinder, Center + FVector(0, 0, 1874), FVector(1750, 1750, 12), Moss, true);
    AddShape(Cylinder, Center + FVector(0, 0, 1940), FVector(1300, 1300, 120), Rock, true);
    AddShape(Cylinder, Center + FVector(0, 0, 1994), FVector(1320, 1320, 12), Moss, true);

    constexpr int32 SegmentCount = 24;
    constexpr float Radius = 1100;
    // 最後は連絡路へ直角に曲がるため、手前から傾斜を緩めて水平にします。
    // 登り坂のすぐ上に幅広い橋を置くと、その側面に頭が当たって歩けなくなるためです。
    const float LandingHeights[] = {1800, 1860, 1900, 1920, 1920};
    MountainFootPath.Reset();
    for (int32 Index = 0; Index <= SegmentCount; ++Index)
    {
        const float Angle = PI + Index * 2 * PI / SegmentCount;
        const float Height = Index < 20 ? Index * 90.f : LandingHeights[Index - 20];
        const FVector Point = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, Height);
        if (MountainFootPath.Num()) AddRamp(MountainFootPath.Last(), Point, 420);
        MountainFootPath.Add(Point);
    }
    const FVector SummitEntry(1600, 2100, 2000);
    AddRamp(MountainFootPath.Last(), SummitEntry, 420);
    MountainFootPath.Add(SummitEntry);
    MountainFootPath.Add(Center + FVector(0, 0, 2000));
    AddSign(Center + FVector(200, 300, 2000), TEXT("THE CLOUD CROWN"));
    AddSign(FVector(700, 2300, 0), TEXT("SUMMIT TRAIL\nFollow the rising path"));
}

void AThunderMeadowStage::BuildSkyIsland()
{
    // 地上まで埋める段丘とは違い、厚さ160cmの床だけを空中に置きます。
    AddShape(Cube, SkyIslandTop - FVector(0, 0, 80), FVector(1100, 1100, 160), FLinearColor(.2f, .3f, .34f), true);
    AddShape(Cube, SkyIslandTop - FVector(0, 0, 6), FVector(1100, 1100, 12), FLinearColor(.2f, .46f, .3f), true);
    // 下側の岩にも衝突を付け、底からすり抜けないようにします。上面は平らな床で整えます。
    AddShape(Cone, SkyIslandTop - FVector(0, 0, 200), FVector(1100, 1100, 300),
        FLinearColor(.12f, .24f, .28f), true, FRotator(180, 0, 0));
    AddTree(SkyIslandTop + FVector(200, 160, 0));
    AddSign(SkyIslandTop + FVector(-50, -240, 0), TEXT("THE LANTERN ISLAND"));
}

void AThunderMeadowStage::BuildWoodenBridge()
{
    AddTerrace(FVector(-2500, -1200, 100), FVector2D(600, 600), FLinearColor(.12f, .38f, .2f));
    AddTerrace(FVector(-1200, -1200, 100), FVector2D(600, 600), FLinearColor(.12f, .38f, .2f));
    BridgeFootPath.Reset();
    constexpr int32 PlankCount = 12;
    const FLinearColor Wood(.35f, .19f, .08f);
    for (int32 Index = 0; Index <= PlankCount; ++Index)
    {
        const float T = static_cast<float>(Index) / PlankCount;
        const FVector Point(-2200 + 700 * T, -1200, 100 + 150 * FMath::Sin(PI * T));
        if (BridgeFootPath.Num())
        {
            const FVector Previous = BridgeFootPath.Last();
            const FVector Delta = Point - Previous;
            const FRotator Rotation = Delta.Rotation();
            const FVector Up = Rotation.RotateVector(FVector::UpVector);
            // 厚さ24cmの板を5cmだけ重ね、歩く面に隙間を作らず、木の板の区切りも残します。
            AddShape(Cube, (Previous + Point) * .5f - Up * 12, FVector(Delta.Size() + 5, 260, 24), Wood, true, Rotation);
        }
        BridgeFootPath.Add(Point);
        if (Index == 4 || Index == 8)
        {
            // 支柱は中央通路を避けて両側へ置き、橋の下に通常の身長で通れる空間を残します。
            for (float Side : {-1.f, 1.f})
                AddShape(Cube, FVector(Point.X, Point.Y + Side * 150, Point.Z * .5f), FVector(30, 30, Point.Z), FLinearColor(.23f, .12f, .055f), true);
        }
    }
    AddSign(FVector(-2650, -1000, 100), TEXT("THE TIMBER CROSSING"));
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
    BuildWoodenBridge();
    BuildMountain();
    BuildSkyIsland();
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
