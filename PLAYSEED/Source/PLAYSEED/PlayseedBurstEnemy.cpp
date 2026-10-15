#include "PlayseedBurstEnemy.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedBurstEnemy::APlayseedBurstEnemy()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BurstSphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Fuse = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScarletFuse"));
    Fuse->SetupAttachment(RootComponent);
    Fuse->SetStaticMesh(BurstSphere.Object);
    Fuse->SetRelativeLocation(FVector(0, 0, 135));
    Fuse->SetRelativeScale3D(FVector(.16f));
    Fuse->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Blast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeedburstFlash"));
    Blast->SetupAttachment(RootComponent);
    Blast->SetStaticMesh(BurstSphere.Object);
    Blast->SetRelativeLocation(FVector(0, 0, 40));
    Blast->SetRelativeScale3D(FVector(6));
    Blast->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayseedBurstEnemy::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto Paint = [this, Palette](UStaticMeshComponent* Part, FLinearColor Color)
        {
            auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
            Material->SetVectorParameterValue(TEXT("Tint"), Color);
            Part->SetMaterial(0, Material);
        };
        Paint(Fuse, FLinearColor(1, .08f, .04f));
        Paint(Blast, FLinearColor(1, .65f, .05f));
    }
    Fuse->SetVisibility(false);
    Blast->SetVisibility(false);
}

void APlayseedBurstEnemy::OnDefeated(AWindseedCharacter*)
{
    // 攻撃や踏みつけで先に倒された場合も、残りの導火線を止めます。
    bFuseActive = false;
    Fuse->SetVisibility(false);
}

void APlayseedBurstEnemy::Explode(AWindseedCharacter* Player)
{
    bExploded = true;
    FlashAge = 0;
    ++ExplosionCount;
    Blast->SetVisibility(true);
    DefeatEnemy(Player, false);
    const auto* Capsule = Player->GetCapsuleComponent();
    const float AxisHalf = Capsule->GetScaledCapsuleHalfHeight() - Capsule->GetScaledCapsuleRadius();
    const FVector Center = Player->GetActorLocation();
    const FVector BlastCenter = GetActorLocation() + FVector(0, 0, 40);
    const FVector Closest = FMath::ClosestPointOnSegment(BlastCenter, Center - FVector(0, 0, AxisHalf), Center + FVector(0, 0, AxisHalf));
    // 爆発済みの状態を先に確定し、被弾からの死亡・再開は最後に通知します。
    if (FVector::DistSquared(BlastCenter, Closest) <= FMath::Square(300.f + Capsule->GetScaledCapsuleRadius()))
        if (Player->GetVitality()->TryTakeImpact(2, Center - BlastCenter)) ++BlastHits;
}

void APlayseedBurstEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bExploded)
    {
        FlashAge += DeltaSeconds;
        Blast->SetVisibility(FlashAge < .25f);
        return;
    }
    if (IsDefeated()) return;
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    if (!bFuseActive && FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) < 450 && FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z - 82) < 160)
    { bFuseActive = true; FuseAge = 0; Fuse->SetVisibility(true); }
    if (bFuseActive)
    {
        FuseAge += DeltaSeconds;
        Fuse->SetRelativeScale3D(FVector(.16f + .06f * FMath::Sin(FuseAge * 22)));
        if (FuseAge >= 2) Explode(Player);
    }
}

void APlayseedBurstEnemy::ResetEnemy()
{
    bFuseActive = bExploded = false;
    FuseAge = FlashAge = 0;
    Fuse->SetVisibility(false);
    Blast->SetVisibility(false);
    Super::ResetEnemy();
}
