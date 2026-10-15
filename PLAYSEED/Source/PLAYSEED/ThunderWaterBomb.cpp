#include "ThunderWaterBomb.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderWaterBomb::AThunderWaterBomb()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RainfallRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RainfallWarning"));
    Marker->SetupAttachment(RootComponent);
    Marker->SetStaticMesh(Cylinder.Object);
    Marker->SetRelativeLocation(FVector(0, 0, 3));
    Marker->SetRelativeScale3D(FVector(4, 4, .04f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Drop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RainfallPearl"));
    Drop->SetupAttachment(RootComponent);
    Drop->SetStaticMesh(Sphere.Object);
    Drop->SetRelativeScale3D(FVector(1.2f));
    Drop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AThunderWaterBomb::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        MarkerTint = UMaterialInstanceDynamic::Create(Palette, this);
        Marker->SetMaterial(0, MarkerTint);
        auto* DropTint = UMaterialInstanceDynamic::Create(Palette, this);
        DropTint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.08f, .5f, .95f));
        Drop->SetMaterial(0, DropTint);
    }
    ResetCycle();
}

void AThunderWaterBomb::BeginWarning()
{
    Phase = EThunderWaterPhase::Warning;
    PhaseAge = 0;
    BombHeight = 700;
    FallSpeed = 0;
    ++WarningSerial;
    Drop->SetRelativeLocation(FVector(0, 0, BombHeight + 60));
    Drop->SetVisibility(false);
    Marker->SetVisibility(true);
    Marker->SetRelativeScale3D(FVector(4, 4, .04f));
    if (MarkerTint) MarkerTint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1, .48f, .08f));
}

void AThunderWaterBomb::ResetCycle()
{
    // 診断の累積回数は残し、遊びに必要な落下速度と周期だけを初期化します。
    BeginWarning();
}

void AThunderWaterBomb::Impact()
{
    // 外部へ被弾を通知する前に着弾済みにします。通知中の死亡・再開が最後に残る設計です。
    Phase = EThunderWaterPhase::Recovering;
    PhaseAge = 0;
    ++ImpactCount;
    Drop->SetVisibility(false);
    Marker->SetVisibility(true);
    Marker->SetRelativeScale3D(FVector(4, 4, .04f));
    if (MarkerTint) MarkerTint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.2f, .85f, 1));
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const auto* Capsule = Player->GetCapsuleComponent();
    const float AxisHalf = Capsule->GetScaledCapsuleHalfHeight() - Capsule->GetScaledCapsuleRadius();
    const FVector Center = Player->GetActorLocation();
    const FVector Blast = GetActorLocation() + FVector(0, 0, 20);
    const FVector Closest = FMath::ClosestPointOnSegment(Blast, Center - FVector(0, 0, AxisHalf), Center + FVector(0, 0, AxisHalf));
    if (FVector::DistSquared(Blast, Closest) <= FMath::Square(200.f + Capsule->GetScaledCapsuleRadius()))
        if (Player->GetVitality()->TryTakeImpact(2, Center - GetActorLocation())) ++ConfirmedHits;
}

void AThunderWaterBomb::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    PhaseAge += DeltaSeconds;
    if (Phase == EThunderWaterPhase::Warning)
    {
        const float Pulse = 4 + FMath::Sin(PhaseAge * 10) * .12f;
        Marker->SetRelativeScale3D(FVector(Pulse, Pulse, .04f));
        if (PhaseAge >= 1.6f)
        {
            Phase = EThunderWaterPhase::Falling;
            PhaseAge = 0;
            Drop->SetVisibility(true);
        }
    }
    else if (Phase == EThunderWaterPhase::Falling)
    {
        FallSpeed += 980 * DeltaSeconds;
        BombHeight = FMath::Max(0.f, BombHeight - FallSpeed * DeltaSeconds);
        Drop->SetRelativeLocation(FVector(0, 0, BombHeight + 60));
        if (BombHeight == 0) Impact();
        // ImpactからGameModeの再開へ入った場合も、ここから古い状態を書き戻しません。
    }
    else
    {
        Marker->SetVisibility(PhaseAge < .3f);
        if (PhaseAge >= 2) BeginWarning();
    }
}
