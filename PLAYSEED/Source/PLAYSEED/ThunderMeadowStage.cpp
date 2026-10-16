#include "ThunderMeadowStage.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderMovingPlatform.h"
#include "ThunderTiltingBridge.h"
#include "ThunderShortcutGate.h"
#include "ThunderWarpPair.h"
#include "ThunderDamageThistle.h"
#include "PlayseedCarryable.h"
#include "PlayseedClimbTree.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedCoin.h"
#include "PlayseedBreakableBox.h"
#include "ThunderRollingBoulder.h"
#include "ThunderWaterBomb.h"
#include "PlayseedChasingEnemy.h"
#include "PlayseedBurstEnemy.h"
#include "PlayseedGaleleafEnemy.h"
#include "ThunderGuide.h"
#include "ThunderLauncher.h"
#include "ThunderFlightBloom.h"
#include "ThunderFlightSwitch.h"
#include "ThunderStormkeeper.h"
#include "ThunderCloudflagRace.h"
#include "ThunderSkyCache.h"
#include "ThunderScarletTrail.h"
#include "ThunderSkyChimes.h"
#include "ThunderBoundSentinel.h"
#include "ThunderBindingPost.h"
#include "ThunderPrisonGate.h"
#include "ThunderHundredLantern.h"
#include "ThunderMountainRest.h"
#include "ThunderLifeseed.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
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
    // 南の木を橋の出口から離し、乗降とカメラの見通しを確保します。
    for (const FVector& Position : {FVector(-2500, -2000, 0), FVector(1450, -2500, 0), FVector(-2600, 2400, 0), FVector(400, 2400, 0), FVector(2900, -1000, 0)})
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

void AThunderMeadowStage::BuildMovingPlatform()
{
    // World が Actor を所有し、Stage は再開時に初期化するための参照だけを保持します。
    MovingPlatform = GetWorld()->SpawnActor<AThunderMovingPlatform>(FVector(600, 800, 90), FRotator::ZeroRotator);
    AddTerrace(FVector(1250, 800, 220), FVector2D(500, 500), FLinearColor(.18f, .42f, .26f));
    AddSign(FVector(300, 450, 0), TEXT("THE DRIFTLEAF LIFT\nJump aboard, ride, then jump to the ledge"));
}

void AThunderMeadowStage::ResetMechanisms()
{
    if (MovingPlatform) MovingPlatform->ResetMotion();
    if (TiltingBridge) TiltingBridge->ResetTilt();
    if (ShortcutGate) ShortcutGate->ResetGate();
    if (WarpPair) WarpPair->ResetWarps();
    if (CarryProp) CarryProp->ResetToSpawn();
    if (PracticeEnemy) PracticeEnemy->ResetEnemy();
    for (APlayseedCoin* Coin : TrainingCoins) if (Coin) Coin->ResetCoin();
    if (PracticeBox) PracticeBox->ResetBox();
    for (APlayseedCoin* Coin : TrailCoins) if (Coin) Coin->ResetCoin();
    if (RollingBoulder) RollingBoulder->ResetMotion();
    if (WaterBomb) WaterBomb->ResetCycle();
    if (ChasingEnemy) ChasingEnemy->ResetEnemy();
    if (BurstEnemy) BurstEnemy->ResetEnemy();
    if (GaleleafEnemy) GaleleafEnemy->ResetEnemy();
    if (Guide) Guide->ResetDialogue();
    if (FlightBloom) FlightBloom->ResetBloom();
    if (FlightSwitch) FlightSwitch->RefreshFromProgress();
    if (Stormkeeper) Stormkeeper->ResetToSpawn();
    if (CloudflagRace) CloudflagRace->ResetRace();
    if (SkyCache) SkyCache->ResetBox();
    if (ScarletTrail) ScarletTrail->ResetTrail();
    if (SkyChimes) SkyChimes->ResetChimes();
    if (BoundSentinel) BoundSentinel->ResetSentinel();
    if (BindingPost) BindingPost->ResetPost();
    if (PrisonGate) PrisonGate->ResetGate();
    if (HundredLantern) HundredLantern->ResetBonus();
    if (MountainRest) MountainRest->ResetRest();
}

void AThunderMeadowStage::BuildWarpCaves()
{
    const FVector MountainCenter(2200, 2100, 0);
    FVector Floors[2];
    float ExitYaws[2];
    const int32 TrailIndices[2] = {2, 22};
    for (int32 Index = 0; Index < 2; ++Index)
    {
        const FVector Trail = MountainFootPath[TrailIndices[Index]];
        const FVector Outward = (Trail - MountainCenter).GetSafeNormal2D();
        Floors[Index] = Trail + Outward * 350;
        const FRotator Rotation(0, Outward.Rotation().Yaw, 0);
        ExitYaws[Index] = (-Outward).Rotation().Yaw;
        auto At = [&](FVector Local) { return Floors[Index] + Rotation.RotateVector(Local); };
        const FLinearColor Rock(.16f, .23f, .29f);
        // 山道の外側へ床を重ね、三方の壁と天井で中が空いたくぼみを作ります。
        AddShape(Cube, At(FVector(0, 0, -20)), FVector(520, 460, 40), Rock, true, Rotation);
        for (float Side : {-1.f, 1.f}) AddShape(Cube, At(FVector(0, Side * 250, 140)), FVector(580, 40, 280), Rock, true, Rotation);
        AddShape(Cube, At(FVector(280, 0, 140)), FVector(40, 540, 280), Rock, true, Rotation);
        AddShape(Cube, At(FVector(0, 0, 300)), FVector(580, 540, 40), Rock, true, Rotation);
        AddShape(Cylinder, At(FVector(0, 0, 3)), FVector(130, 130, 6), FLinearColor(.4f, .15f, .75f));
        AddSign(Trail - Outward * 65, TEXT("THE ECHO HOLLOW\nStep into the purple light"));
    }
    WarpPair = GetWorld()->SpawnActor<AThunderWarpPair>();
    if (WarpPair) WarpPair->Configure(Floors[0], Floors[1], ExitYaws[0], ExitYaws[1]);
}

void AThunderMeadowStage::BuildDamageThistle()
{
    DamageThistle = GetWorld()->SpawnActor<AThunderDamageThistle>(FVector(0, -1200, 60), FRotator::ZeroRotator);
    AddSign(FVector(0, -1500, 0), TEXT("SPARKING THISTLE\nKeep clear, or jump over"));
}

void AThunderMeadowStage::BuildCarryProp()
{
    CarryProp = GetWorld()->SpawnActor<APlayseedCarryable>(FVector(-450, -1100, 60), FRotator::ZeroRotator);
    AddSign(FVector(-650, -1350, 0), TEXT("THE TIDESTONE\nE / X: lift nearby, then throw"));
}

void AThunderMeadowStage::BuildClimbTree()
{
    ClimbTree = GetWorld()->SpawnActor<APlayseedClimbTree>(FVector(-1800, -500, 0), FRotator::ZeroRotator);
    AddSign(FVector(-1550, -650, 0), TEXT("THE HUSHBLOOM TREE\nF / Y: hold or release\nW / S: climb   SPACE: leap away"));
}

void AThunderMeadowStage::BuildPracticeEnemy()
{
    PracticeEnemy = GetWorld()->SpawnActor<APlayseedEnemyBase>(FVector(300, -1500, 0), FRotator::ZeroRotator);
    if (PracticeEnemy) PracticeEnemy->ConfigureMeleeDurability(4);
    AddSign(FVector(550, -1750, 0), TEXT("THE TUFTLING\nJump, or try punch - punch - kick"));
}

void AThunderMeadowStage::BuildTrainingCoins()
{
    const EPlayseedCoinKind TrainingKinds[] = {EPlayseedCoinKind::Gold, EPlayseedCoinKind::Scarlet, EPlayseedCoinKind::Azure};
    for (int32 Index = 0; Index < 3; ++Index)
    {
        auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(FVector(-350 + Index * 300, -300, 62), FRotator::ZeroRotator);
        if (!Coin) continue;
        Coin->ConfigureCoin(*FString::Printf(TEXT("TrainingCoin_%d"), Index), TrainingKinds[Index]);
        TrainingCoins.Add(Coin);
    }
    AddSign(FVector(-50, -60, 0), TEXT("LIGHTCOINS\nGold 1 - Scarlet 2 - Azure 5"));
}

void AThunderMeadowStage::BuildPracticeBox()
{
    PracticeBox = GetWorld()->SpawnActor<APlayseedBreakableBox>(FVector(800, -600, 0), FRotator::ZeroRotator);
    if (PracticeBox) PracticeBox->ConfigureBox(TEXT("MeadowCopperCache"));
    AddSign(FVector(1050, -800, 0), TEXT("THE COPPER CACHE\nStrike, or dive onto the top"));
}

void AThunderMeadowStage::BuildCoinTrail()
{
    // 草原の四つの花輪と山道へ、固有IDを持つ金コインを100個配置します。
    // 地形と同じ足元の点を残し、検証でも実際の配置へ歩いて向かえます。
    const FVector RosetteCenters[] = {FVector(-2400, 2500, 0), FVector(-1400, 2650, 0), FVector(-450, 2650, 0), FVector(550, 2400, 0)};
    auto SpawnTrailCoin = [this](FVector FootPoint)
    {
        const int32 CoinIndex = TrailCoins.Num();
        auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(FootPoint + FVector(0, 0, 62), FRotator::ZeroRotator);
        if (!Coin) return;
        Coin->ConfigureCoin(*FString::Printf(TEXT("TrailGold_%03d"), CoinIndex), EPlayseedCoinKind::Gold);
        TrailCoins.Add(Coin);
    };
    for (const FVector& Center : RosetteCenters)
    {
        for (int32 Index = 0; Index < 20; ++Index)
        {
            const float Angle = Index * 2 * PI / 20;
            const FVector FootPoint = Center + FVector(FMath::Cos(Angle) * 260, FMath::Sin(Angle) * 260, 0);
            SpawnTrailCoin(FootPoint);
            MeadowCoinFootPath.Add(FootPoint);
        }
    }
    for (int32 Index = 0; Index < 20 && Index < MountainFootPath.Num(); ++Index) SpawnTrailCoin(MountainFootPath[Index]);
    AddSign(FVector(-2700, 2150, 0), TEXT("THE GLOWING ROSETTES\nFollow four circles, then the summit trail"));
}

void AThunderMeadowStage::BuildRollingBoulder()
{
    TArray<FVector> DownhillPath;
    for (int32 Index = 19; Index >= 3; --Index) DownhillPath.Add(MountainFootPath[Index]);
    RollingBoulder = GetWorld()->SpawnActor<AThunderRollingBoulder>();
    if (RollingBoulder) RollingBoulder->ConfigurePath(DownhillPath);
    AddSign(MountainFootPath[0] + FVector(-300, -100, 0), TEXT("THE STORMSTONE RUN\nWatch the rolling stones - use the edges"));
}

void AThunderMeadowStage::BuildWaterBomb()
{
    WaterBomb = GetWorld()->SpawnActor<AThunderWaterBomb>(FVector(-400, -2600, 0), FRotator::ZeroRotator);
    AddSign(FVector(-100, -2850, 0), TEXT("THE RAINFALL PEARL\nLeave the amber circle before it falls"));
}

void AThunderMeadowStage::BuildChasingEnemy()
{
    ChasingEnemy = GetWorld()->SpawnActor<APlayseedChasingEnemy>(FVector(2700, -1200, 0), FRotator(0, 180, 0));
    AddSign(FVector(3000, -1450, 0), TEXT("THE ROAMING TUFTLING\nKeep moving, or strike before it reaches you"));
}

void AThunderMeadowStage::BuildBurstEnemy()
{
    BurstEnemy = GetWorld()->SpawnActor<APlayseedBurstEnemy>(FVector(2300, -2300, 0), FRotator(0, -90, 0));
    AddSign(FVector(2600, -2550, 0), TEXT("THE SCARLET SEEDBURST\nTwo seconds to leave - or strike first"));
}

void AThunderMeadowStage::BuildGaleleafEnemy()
{
    GaleleafEnemy = GetWorld()->SpawnActor<APlayseedGaleleafEnemy>(FVector(2700, 0, 0), FRotator(0, 180, 0));
    AddSign(FVector(3000, 250, 0), TEXT("THE GALELEAF CARRIER\nFirst hit frees the leaf - another earns azure five"));
}

void AThunderMeadowStage::BuildGuideAndLauncher()
{
    Guide = GetWorld()->SpawnActor<AThunderGuide>(FVector(-2800, 1800, 0), FRotator::ZeroRotator);
    Launcher = GetWorld()->SpawnActor<AThunderLauncher>(FVector(-2400, 1800, 0), FRotator::ZeroRotator);
    AddSign(FVector(-3050, 1500, 0), TEXT("AERI'S SKYBLOOM\nG / D-pad Down: talk or interact nearby"));
}

void AThunderMeadowStage::BuildFlightBloom()
{
    FlightBloom = GetWorld()->SpawnActor<AThunderFlightBloom>(FVector(-2600, 2800, 0), FRotator::ZeroRotator);
    AddSign(FVector(-2850, 2850, 0), TEXT("LEAFWINGS\nSPACE in the air: glide / fall\nMouse or right stick: steer - W/S: speed"));
}

void AThunderMeadowStage::BuildWingTower()
{
    const FVector Center(-2600, -2500, 0);
    AddShape(Cylinder, Center + FVector(0, 0, 470), FVector(280, 280, 940), FLinearColor(.12f, .27f, .35f), true);
    for (int32 Index = 0; Index < 8; ++Index)
    {
        const float Angle = PI + Index * PI / 4;
        const FVector Top = Center + FVector(FMath::Cos(Angle) * 500, FMath::Sin(Angle) * 500, 100 + Index * 120);
        AddTerrace(Top, FVector2D(400, 400), FLinearColor(.25f, .6f, .47f));
        WingTowerFootPath.Add(Top);
    }
    FlightSwitch = GetWorld()->SpawnActor<AThunderFlightSwitch>(WingTowerFootPath.Last(), FRotator::ZeroRotator);
    AddSign(FVector(-3300, -2700, 0), TEXT("THE SUNSPROUT SPIRE\nJump around the steps - dive onto the sun switch\nAwaken leafwings throughout the meadow"));
}

void AThunderMeadowStage::BuildShortcutGate()
{
    const FVector Position(1600, 300, 0);
    ShortcutGate = GetWorld()->SpawnActor<AThunderShortcutGate>(Position, FRotator::ZeroRotator);
    // 柵の左右を壁へつなぎ、スイッチを踏むと中央の通路を歩けるようにします。
    for (const float Side : {-1.f, 1.f})
        AddShape(Cube, Position + FVector(0, Side * 400, 170), FVector(60, 480, 340), FLinearColor(.2f, .28f, .26f), true);
    AddSign(Position + FVector(-450, -550, 0), TEXT("THE MEADOW SHORTCUT\nStand on the purple switch"));
}

void AThunderMeadowStage::BuildTiltingBridge()
{
    TiltingBridge = GetWorld()->SpawnActor<AThunderTiltingBridge>(FVector(0, -2100, 160), FRotator::ZeroRotator);
    AddTerrace(FVector(-850, -2100, 180), FVector2D(500), FLinearColor(.2f, .4f, .24f));
    AddTerrace(FVector(850, -2100, 180), FVector2D(500), FLinearColor(.2f, .4f, .24f));
    // 中央の支点は装飾です。歩行の衝突は回転する床一枚に集約します。
    AddShape(Cylinder, FVector(0, -2100, 65), FVector(120, 180, 190), FLinearColor(.27f, .2f, .15f));
    AddSign(FVector(-850, -2350, 180), TEXT("THE BALANCE BOUGH\nYour weight tips the crossing"));
}

void AThunderMeadowStage::BuildLighting()
{
    auto* SkyDome = AddShape(Sphere, FVector::ZeroVector, FVector(40000), FLinearColor::White);
    SkyDome->SetMaterial(0, SkyMaterial);
    if(AReferenceBattlefieldTerrain::IsReferenceCourse())
        if(auto* ReferenceSky=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Sky.M_Sky")))SkyDome->SetMaterial(0,ReferenceSky);
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
    if(AReferenceBattlefieldTerrain::IsReferenceCourse())
    {
        Exposure->Settings.bOverride_ToneCurveAmount=true;Exposure->Settings.ToneCurveAmount=0;
        Exposure->Settings.bOverride_ExpandGamut=true;Exposure->Settings.ExpandGamut=0;
        Exposure->Settings.bOverride_BlueCorrection=true;Exposure->Settings.BlueCorrection=0;
        Exposure->Settings.bOverride_AutoExposureBias=true;Exposure->Settings.AutoExposureBias=0;
    }
}

void AThunderMeadowStage::BuildStormkeeper()
{
    // 主ミッション1だけに番人を配置します。別の目的で山頂を訪れる時は通行を妨げません。
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber() != 1) return;
    Stormkeeper = GetWorld()->SpawnActor<AThunderStormkeeper>(FVector(2200, 2100, 2100), FRotator(0, 180, 0));
    AddSign(FVector(2250, 2450, 2000), TEXT("THE STORMKEEPER\nCircle behind - E / X to lift and throw\nThree landings on the crown - cliffs do not count"));
}

void AThunderMeadowStage::BeginPlay()
{
    Super::BeginPlay();
    // 新地形は検証用の明示オプションから段階的に統合します。
    if(AReferenceBattlefieldTerrain::IsReferenceCourse())
    {
        GetWorld()->SpawnActor<AReferenceBattlefieldTerrain>();
        BuildReferenceChain();
        BuildLighting();
        return;
    }
    BuildMeadow();
    BuildWoodenBridge();
    BuildMovingPlatform();
    BuildTiltingBridge();
    BuildShortcutGate();
    BuildMountain();
    BuildWarpCaves();
    BuildDamageThistle();
    BuildCarryProp();
    BuildClimbTree();
    BuildPracticeEnemy();
    BuildTrainingCoins();
    BuildPracticeBox();
    BuildCoinTrail();
    BuildRollingBoulder();
    BuildWaterBomb();
    BuildChasingEnemy();
    BuildBurstEnemy();
    BuildGaleleafEnemy();
    BuildGuideAndLauncher();
    BuildFlightBloom();
    BuildWingTower();
    BuildStormkeeper();
    BuildCloudflagRace();
    BuildSkyIsland();
    BuildSkyCache();
    BuildScarletTrail();
    BuildSkyChimes();
    BuildBoundSentinel();
    HundredLantern = GetWorld()->SpawnActor<AThunderHundredLantern>();
    // 山道の外側140cmは歩ける幅の内側で、岩球の中央線から離れます。
    const FVector RestFoot = MountainFootPath[11];
    const FVector RestSpot = RestFoot + (RestFoot - FVector(2200,2100,0)).GetSafeNormal2D() * 140;
    MountainRest = GetWorld()->SpawnActor<AThunderMountainRest>(RestSpot, FRotator::ZeroRotator);
    AddSign(RestSpot + FVector(0,0,30), TEXT("DEWREST BLOOM\nGround contact restores 2 vitality\nWait 5 seconds, leave and return"));
    Lifeseed = GetWorld()->SpawnActor<AThunderLifeseed>(FVector(-2200,800,62),FRotator::ZeroRotator);
    AddSign(FVector(-2350,1000,0),TEXT("A LIFESEED\nTouch its amber heart: +1 life\nOnce per visit - R keeps it collected"));
    BuildLighting();
}

void AThunderMeadowStage::BuildCloudflagRace()
{
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber() != 2) return;
    CloudflagRace = GetWorld()->SpawnActor<AThunderCloudflagRace>(FVector(-1050, -650, 0), FRotator::ZeroRotator);
    CloudflagRace->Configure(MountainFootPath);
    AddSign(FVector(-1250, -800, 0), TEXT("BRISK'S CLOUDFLAG RACE\nG / DOWN to start - caves allowed\nNo Skybloom launches - R to retry"));
    AddShape(Cylinder, FVector(2240, 2140, 2160), FVector(14, 14, 320), FLinearColor(.85f, .55f, .18f));
    AddShape(Cube, FVector(2330, 2140, 2260), FVector(180, 10, 100), FLinearColor(.05f, .65f, .55f));
    AddSign(FVector(2380, 2350, 2000), TEXT("THE CLOUDFLAG\nReach the crown, then wait for Brisk"));
}

void AThunderMeadowStage::BuildReferenceChain()
{
    // 参照の(X,Y,Z)をUEの(X,-Z,Y)へ変換した配置です。地形と同じcm単位で置きます。
    const FVector PostFloor(260,-1920,735);
    const FTransform GateTransform(FRotator(0,56,0),FVector(1456,-446,968));
    PrisonGate=GetWorld()->SpawnActorDeferred<AThunderPrisonGate>(AThunderPrisonGate::StaticClass(),GateTransform);
    PrisonGate->ConfigureReferenceReward(FVector(1550,-300,1200));
    UGameplayStatics::FinishSpawningActor(PrisonGate,GateTransform);
    BoundSentinel=GetWorld()->SpawnActor<AThunderBoundSentinel>(PostFloor+FVector(400,0,228),FRotator::ZeroRotator);
    BoundSentinel->Configure(PostFloor);PrisonGate->Configure(BoundSentinel);
    BindingPost=GetWorld()->SpawnActor<AThunderBindingPost>(PostFloor,FRotator::ZeroRotator);
    BindingPost->Configure(BoundSentinel,PrisonGate);
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

void AThunderMeadowStage::BuildSkyCache()
{
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber() != 3) return;
    SkyCache = GetWorld()->SpawnActor<AThunderSkyCache>(SkyIslandTop + FVector(200, 150, 0), FRotator::ZeroRotator);
    SkyCache->ConfigureBox(TEXT("SkyLanternCache"));
    AddSign(SkyIslandTop + FVector(200, 330, 0), TEXT("THE SKYSEED CACHE\nStrike the copper box - claim its lantern"));
}

void AThunderMeadowStage::BuildScarletTrail()
{
    // どの目的で入場しても探索できます。各地点はPLAYSEED独自の配置です。
    const FVector Court(1200, -1300, 0);
    // 山道の赤は道幅の内側の縁へ置き、既存の中央の金コイン周回と重ねません。
    const FVector MountainRed = MountainFootPath[3] + (MountainFootPath[3] - FVector(2200, 2100, 0)).GetSafeNormal2D() * 140 + FVector(0, 0, 62);
    ScarletTrail = GetWorld()->SpawnActor<AThunderScarletTrail>(Court, FRotator::ZeroRotator);
    ScarletTrail->Configure({FVector(-1800, -500, 712), FVector(-700, 1800, 362), FVector(-1850, -1200, 312),
        FVector(1250, 800, 282), FVector(0, -2100, 242), MountainRed,
        SkyIslandTop + FVector(-300, -150, 62), Court + FVector(-350, -100, 62)});
    AddShape(Cylinder, Court + FVector(0, 200, 3), FVector(220, 220, 6), FLinearColor(.7f, .15f, .25f));
    AddSign(Court + FVector(0, 250, 0), TEXT("THE BINDING COURT\nFind eight scarlet trail coins\nA lantern returns to this circle"));
}

void AThunderMeadowStage::BuildSkyChimes()
{
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber() != 5) return;
    SkyChimes = GetWorld()->SpawnActor<AThunderSkyChimes>();
    SkyChimes->Configure({FVector(-1200, 2000, 2400), FVector(-400, 2000, 2400), FVector(400, 2000, 2400), FVector(1200, 2000, 2400), FVector(2000, 2000, 2400)}, SkyIslandTop + FVector(-250, 250, 100));
    AddSign(FVector(-2500, 1550, 0), TEXT("THE FIVE SKY CHIMES\nAwaken leafwings and launch above the crown\nCollect the five CENTRE coins - edges do not count"));
}

void AThunderMeadowStage::BuildBoundSentinel()
{
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber() != 6) return;
    BoundSentinel = GetWorld()->SpawnActor<AThunderBoundSentinel>();
    if (BoundSentinel) BoundSentinel->Configure(FVector(2700, -2800, 0));
    PrisonGate = GetWorld()->SpawnActor<AThunderPrisonGate>(FVector(3250, -2800, 200), FRotator::ZeroRotator);
    BindingPost = GetWorld()->SpawnActor<AThunderBindingPost>(FVector(2700, -2800, 0), FRotator::ZeroRotator);
    if (PrisonGate) PrisonGate->Configure(BoundSentinel);
    if (BindingPost) BindingPost->Configure(BoundSentinel, PrisonGate);
    AddShape(Cube, FVector(3460, -2800, 200), FVector(30, 430, 400), FLinearColor(.17f, .24f, .24f), true);
    for (float Side : {-1.f, 1.f}) AddShape(Cube, FVector(3350, -2800 + Side * 210, 200), FVector(220, 25, 400), FLinearColor(.17f, .24f, .24f), true);
    AddSign(FVector(2450, -3100, 0), TEXT("THE COPPER BINDING POST\nJump above its top, then CTRL / RB dive\nThree real pounds free the sentinel"));
    AddSign(FVector(3150, -2450, 0), TEXT("MOSSBOUND SENTINEL\nThe copper tether limits its reach\nLeave the GOLD target before it lunges"));
}
