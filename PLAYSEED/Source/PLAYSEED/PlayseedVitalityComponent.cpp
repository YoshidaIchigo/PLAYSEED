#include "PlayseedVitalityComponent.h"
#include "PlayseedGameModeBase.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "WindseedCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayseedVitalityComponent::UPlayseedVitalityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayseedVitalityComponent::BeginPlay()
{
    Super::BeginPlay();
    Restore();
}

void UPlayseedVitalityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    InvulnerabilityRemaining = FMath::Max(0.f, InvulnerabilityRemaining - DeltaTime);
}

bool UPlayseedVitalityComponent::TryTakeImpact(int32 Points, FVector AwayDirection)
{
    if (Points <= 0 || bDefeating) return false;
    if (IsInvulnerable()) { ++BlockedImpacts; return false; }
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return false;
    Player->GetActions()->CancelGroundPound();
    Player->GetCarry()->DropHeld();
    Player->GetClimb()->CancelClimb();
    Player->GetCombat()->CancelMelee();
    ++SuccessfulImpacts;
    Health = FMath::Max(0, Health - Points);
    InvulnerabilityRemaining = 1.2f;
    if (Health == 0) { Defeat(); return true; }
    FVector Direction = AwayDirection.GetSafeNormal2D();
    if (Direction.IsNearlyZero()) Direction = -Player->GetActorForwardVector();
    LastImpulse = Direction * 440 + FVector(0, 0, 320);
    // LaunchCharacterを使い、物理移動として押し戻します。座標を飛ばすノックバックではありません。
    Player->StopJumping();
    Player->LaunchCharacter(LastImpulse, true, true);
    return true;
}

void UPlayseedVitalityComponent::Defeat()
{
    if (bDefeating) return;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    bDefeating = true;
    Health = 0;
    ++DefeatCount;
    // 再開で何を初期化するかはステージのGameModeへ任せます。ここに収集物や地形を持ち込みません。
    if (auto* Mode = Cast<APlayseedGameModeBase>(GetWorld()->GetAuthGameMode())) Mode->HandlePlayerDefeat(Player);
    else Player->ReturnToTrail();
    bDefeating = false;
}

void UPlayseedVitalityComponent::Restore()
{
    Health = GetMaximumHealth();
    InvulnerabilityRemaining = 0;
    LastImpulse = FVector::ZeroVector;
}
