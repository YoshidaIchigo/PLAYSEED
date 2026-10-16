#include "ThunderRollingBoulder.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderRollingBoulder::AThunderRollingBoulder()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BoulderRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StoneMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Stone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StormStone"));
    Stone->SetupAttachment(RootComponent);
    Stone->SetStaticMesh(StoneMesh.Object);
    Stone->SetRelativeScale3D(FVector(1.4f));
    Stone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Vein = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmberVein"));
    Vein->SetupAttachment(Stone);
    Vein->SetStaticMesh(StoneMesh.Object);
    Vein->SetRelativeScale3D(FVector(1.02f, .16f, 1.02f));
    Vein->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AThunderRollingBoulder::BeginPlay()
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
        Paint(Stone, FLinearColor(.12f, .18f, .26f));
        Paint(Vein, FLinearColor(.9f, .48f, .08f));
    }
}

void AThunderRollingBoulder::ConfigureReferencePath(const TArray<FVector>& FootPath,bool bContinuousLoop)
{
    Radius=100;Speed=600;bLoop=bContinuousLoop;bReference=true;
    Stone->SetRelativeScale3D(FVector(2));
    // 金の帯を外し、自作の黒い鉄球と小さい反射の形にします。
    Vein->SetRelativeScale3D(FVector(.18f,.22f,.1f));Vein->SetRelativeLocation(FVector(-15,-30,34));
    if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto Paint=[&](UStaticMeshComponent* Part,FLinearColor Color){auto* Tint=UMaterialInstanceDynamic::Create(Material,this);Tint->SetVectorParameterValue(TEXT("Tint"),Color);Part->SetMaterial(0,Tint);};
        // Tintは線形色です。sRGB表示でも灰色にならない暗い値を使います。
        Paint(Stone,FLinearColor(.004f,.006f,.009f));Paint(Vein,FLinearColor(.65f,.7f,.8f));
    }
    ConfigurePath(FootPath);
}

void AThunderRollingBoulder::ConfigurePath(const TArray<FVector>& FootPath)
{
    Path.Reset();
    for (const FVector& FootPoint : FootPath) Path.Add(FootPoint + FVector(0, 0, Radius));
    ResetMotion();
}

void AThunderRollingBoulder::ResetMotion()
{
    ++MotionRevision;
    Speed=bReference?600:450;
    NextPoint = 1;
    TravelledDistance = 0;
    WaitRemaining = 0;
    bWaiting = false;
    SetActorHiddenInGame(false);
    Stone->SetRelativeRotation(FRotator::ZeroRotator);
    if (!Path.IsEmpty()) SetActorLocation(Path[0]);
}

bool AThunderRollingBoulder::IsSpawnClear(AWindseedCharacter* Player) const
{
    if (!Player) return true;
    const auto* Capsule = Player->GetCapsuleComponent();
    const float AxisHalf = Capsule->GetScaledCapsuleHalfHeight() - Capsule->GetScaledCapsuleRadius();
    const FVector Center = Player->GetActorLocation();
    const FVector Closest = FMath::ClosestPointOnSegment(Path[0], Center - FVector(0, 0, AxisHalf), Center + FVector(0, 0, AxisHalf));
    return FVector::DistSquared(Closest, Path[0]) > FMath::Square(240.f);
}

void AThunderRollingBoulder::CheckContact(FVector Start, FVector End, AWindseedCharacter* Player)
{
    if (!Player) return;
    const auto* Capsule = Player->GetCapsuleComponent();
    const float PlayerRadius = Capsule->GetScaledCapsuleRadius();
    const float AxisHalf = Capsule->GetScaledCapsuleHalfHeight() - PlayerRadius;
    const FVector Center = Player->GetActorLocation();
    FVector BallPoint, PlayerPoint;
    // 移動前後の線分とカプセルの軸を照合します。長いフレームで通り越しても接触を見落としません。
    FMath::SegmentDistToSegmentSafe(Start, End, Center - FVector(0, 0, AxisHalf), Center + FVector(0, 0, AxisHalf), BallPoint, PlayerPoint);
    if (FVector::DistSquared(BallPoint, PlayerPoint) <= FMath::Square(Radius + PlayerRadius))
        if (Player->GetVitality()->TryTakeImpact(2, Center - BallPoint)) ++ConfirmedHits;
}

void AThunderRollingBoulder::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Path.Num() < 2) return;
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (bWaiting)
    {
        WaitRemaining = FMath::Max(0.f, WaitRemaining - DeltaSeconds);
        // 出現点に人がいる間は待ち、突然その身体へ岩を重ねません。
        if (WaitRemaining == 0 && IsSpawnClear(Player)) { ResetMotion(); ++RespawnCount; }
        return;
    }
    if(bReference&&!bLoop)
    {
        // 下りの傾きで加速し、山道の平らな区間で突然速さを戻さないようにします。
        const FVector Down=Path[NextPoint]-GetActorLocation();
        Speed=FMath::Clamp(Speed+980*FMath::Max(0.f,-Down.GetSafeNormal().Z)*DeltaSeconds,600.f,2100.f);
    }
    float Remaining = Speed * DeltaSeconds;
    const int32 TickRevision = MotionRevision;
    while (Remaining > 0 && !bWaiting)
    {
        const FVector Start = GetActorLocation();
        const FVector Offset = Path[NextPoint] - Start;
        const float Distance = Offset.Size();
        const float Step = FMath::Min(Remaining, Distance);
        const FVector End = Distance > KINDA_SMALL_NUMBER ? Start + Offset / Distance * Step : Path[NextPoint];
        CheckContact(Start, End, Player);
        // 致命傷からGameModeがこの岩も初期化した場合、古いTickの位置を上書きしません。
        if (MotionRevision != TickRevision) return;
        SetActorLocation(End);
        // 回転は描画だけです。経路の進行とダメージ判定から分離します。
        Stone->AddWorldRotation(FQuat(FVector::CrossProduct(FVector::UpVector, Offset.GetSafeNormal()).GetSafeNormal(), Step / Radius));
        TravelledDistance += Step;
        Remaining -= Step;
        if (Step >= Distance - KINDA_SMALL_NUMBER)
        {
            if (++NextPoint == Path.Num())
            {
                ++CompletedCycles;
                if(bLoop){NextPoint=1;continue;}
                bWaiting = true;
                WaitRemaining = 1.5f;
                SetActorHiddenInGame(true);
            }
        }
    }
}
