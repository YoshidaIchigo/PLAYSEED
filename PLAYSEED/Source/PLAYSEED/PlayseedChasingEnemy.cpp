#include "PlayseedChasingEnemy.h"
#include "PlayseedCoin.h"
#include "WindseedCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void APlayseedChasingEnemy::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
}

void APlayseedChasingEnemy::Tick(float DeltaSeconds)
{
    // 基底Actorが踏みつけ・横接触・撃破後の表示を担当します。
    Super::Tick(DeltaSeconds);
    if (IsDefeated()) return;
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const float Distance = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
    if (State == EPlayseedChaseState::Idle)
    {
        if (Distance < GetDetectionRadius() && FMath::Abs(Player->GetActorLocation().Z - HomeLocation.Z - 82) < 180)
        {
            FCollisionQueryParams Sight;
            Sight.AddIgnoredActor(this);
            Sight.AddIgnoredActor(Player);
            if (!GetWorld()->LineTraceTestByChannel(GetActorLocation() + FVector(0, 0, 80), Player->GetActorLocation(), ECC_Visibility, Sight))
            { State = EPlayseedChaseState::Chasing; ++ChaseStarts; }
        }
    }
    else if (State == EPlayseedChaseState::Chasing)
    {
        // 遠くまで引っ張られたら追跡をやめ、元の場所へ帰ります。
        if (Distance > 1000 || FVector::Dist2D(GetActorLocation(), HomeLocation) > 900)
            State = EPlayseedChaseState::Returning;
        else MoveOnGround(Player->GetActorLocation(), GetChasingSpeed(), DeltaSeconds);
    }
    else
    {
        if (FVector::Dist2D(GetActorLocation(), HomeLocation) < 20)
        { State = EPlayseedChaseState::Idle; ++CompletedReturns; }
        else MoveOnGround(HomeLocation, 220, DeltaSeconds);
    }
}

void APlayseedChasingEnemy::MoveOnGround(FVector Goal, float Speed, float DeltaSeconds)
{
    const FVector Start = GetActorLocation();
    const FVector Direction = (Goal - Start).GetSafeNormal2D();
    const float RemainingDistance = FVector::Dist2D(Start, Goal);
    FVector Candidate = Start + Direction * FMath::Min(Speed * DeltaSeconds, RemainingDistance);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(this, 0));
    FHitResult Wall, Floor;
    // 簡単な地上AIです。壁を抜けず、高い段差や崖へそのまま歩き込みません。
    if (GetWorld()->SweepSingleByChannel(Wall, Start + FVector(0, 0, GetGroundProbeHeight()), Candidate + FVector(0, 0, GetGroundProbeHeight()), FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(70), Params)) return;
    if (!GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0, 0, 150), Candidate - FVector(0, 0, 150), ECC_Visibility, Params) ||
        Floor.ImpactNormal.Z < .7f || FMath::Abs(Floor.ImpactPoint.Z - Start.Z) > 35) return;
    Candidate.Z = Floor.ImpactPoint.Z;
    SetActorLocation(Candidate);
    if (!Direction.IsNearlyZero()) SetActorRotation(FRotator(0, Direction.Rotation().Yaw, 0));
}

void APlayseedChasingEnemy::OnDefeated(AWindseedCharacter*)
{
    if (bDropIssued) return;
    bDropIssued = true;
    ++DropGenerationCount;
    auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(GetActorLocation() + FVector(0, 0, 62), FRotator::ZeroRotator);
    if (!Coin) return;
    Coin->ConfigureCoin(*FString::Printf(TEXT("%s_GoldDrop"), *GetFName().ToString()), EPlayseedCoinKind::Gold);
    DropCoin = Coin;
}

void APlayseedChasingEnemy::ResetEnemy()
{
    if (DropCoin.IsValid()) DropCoin->Destroy();
    DropCoin.Reset();
    bDropIssued = false;
    State = EPlayseedChaseState::Idle;
    Super::ResetEnemy();
}
