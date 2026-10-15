#include "ThunderChaseTestSubsystem.h"
#include "PlayseedChasingEnemy.h"
#include "PlayseedCoin.h"
#include "PlayseedVitalityComponent.h"
#include "ThunderCoinLedger.h"
#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void ChaseTap(APlayerController* PC, FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0));
}
}

void UThunderChaseTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedChasingEnemy* InEnemy)
{
    Enemy = InEnemy;
    Phase = Waypoint = Loop = 0;
    PhaseTime = 0;
    Error.Reset();
    ChaseBaseline = Enemy->GetChaseStarts();
    ReturnBaseline = Enemy->GetCompletedReturns();
    DropBaseline = Enemy->GetDropGenerationCount();
    RecoveryBaseline = Player->RecoveryCount;
    bDropSaved = false;
}

bool UThunderChaseTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderChaseTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    // 箱と近道の壁を避け、地上の南側から東へ向かいます。
    const FVector ChaseApproachPoints[] = {FVector(-600, -900, 82), FVector(1900, -900, 82), FVector(1900, -600, 82)};
    if (Waypoint < 3 && WalkToward(Player, ChaseApproachPoints[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint >= 3;
}

bool UThunderChaseTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode());
    if (!Enemy.IsValid() || !PC || !Mode) { Error = TEXT("Chase test actor or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 25) { Error = FString::Printf(TEXT("Chase phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8) { Error = TEXT("Chase, escape or first-strike route unexpectedly took damage"); return false; }
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (Approach(Player, DeltaSeconds))
        {
            if (!Enemy->IsIdle() || Enemy->GetChaseStarts() != ChaseBaseline) { Error = TEXT("Enemy discovered player outside its range"); return false; }
            Next(1);
        }
    }
    else if (Phase == 1)
    {
        WalkToward(Player, Enemy->GetHomeLocation() + FVector(-500, 0, 82), DeltaSeconds);
        if (Enemy->GetChaseStarts() == ChaseBaseline + 1 && Enemy->IsChasing() && FVector::Dist2D(Enemy->GetActorLocation(), Enemy->GetHomeLocation()) > 180)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChasingTuftling.png"), false, false);
            Next(2);
        }
    }
    else if (Phase == 2)
    {
        if (WalkToward(Player, FVector(1000, -1200, 82), DeltaSeconds)) Next(3);
    }
    else if (Phase == 3)
    {
        if (Enemy->GetCompletedReturns() == ReturnBaseline + 1 && Enemy->IsIdle() && FVector::Dist2D(Enemy->GetActorLocation(), Enemy->GetHomeLocation()) < 25)
        {
            UE_LOG(LogTemp, Display, TEXT("CHASE TEST: out-of-range idle, actual discovery and movement, ordinary retreat and completed return home passed."));
            Next(4);
        }
    }
    else if (Phase == 4)
    {
        const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
        if (Offset.Size2D() < 165 && Player->GetCharacterMovement()->IsMovingOnGround())
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            ChaseTap(PC, EKeys::LeftMouseButton);
            Next(5);
        }
        else
        {
            Player->AddMovementInput(Offset.GetSafeNormal2D());
            PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
        }
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (!Enemy->IsDefeated() || Enemy->GetDropGenerationCount() != DropBaseline + Loop + 1 || !Enemy->GetDropCoin())
        { Error = TEXT("A chasing enemy did not defeat and drop one gold coin"); return false; }
        ChaseTap(PC, EKeys::LeftMouseButton);
        Next(6);
    }
    else if (Phase == 6)
    {
        if (!Enemy->GetDropCoin() || Enemy->GetDropGenerationCount() != DropBaseline + Loop + 1)
        { Error = TEXT("Defeated enemy generated another drop"); return false; }
        if (WalkToward(Player, Enemy->GetDropCoin()->GetActorLocation(), DeltaSeconds))
        {
            if (!Enemy->GetDropCoin()->IsCollected() || Mode->GetCoinLedger()->GetTotal() != 1)
            { Error = TEXT("Walking into the enemy drop did not collect one point"); return false; }
            Next(7);
        }
    }
    else if (Phase == 7)
    {
        if (!bDropSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChaserDrop.png"), false, false);
            bDropSaved = true;
        }
        if (PhaseTime > .6f)
        {
            if (Loop == 0) { ChaseTap(PC, EKeys::R); Next(8); }
            else
            {
                if (Enemy->GetChaseStarts() < ChaseBaseline + 3 || Enemy->GetDropGenerationCount() != DropBaseline + 2)
                { Error = TEXT("Chasing and a fresh drop did not resume after R"); return false; }
                UE_LOG(LogTemp, Display, TEXT("CHASE TEST PASS: discovery, chase, escape, return home, melee defeat and exactly one collectible gold drop, R cleanup, second chase/defeat/fresh one point passed."));
                return true;
            }
        }
    }
    else if (Phase == 8 && PhaseTime > .2f)
    {
        if (Enemy->IsDefeated() || !Enemy->IsIdle() || Enemy->GetDropCoin() || Mode->GetCoinLedger()->GetTotal() != 0 || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not revive the chaser and remove its own drop and total"); return false; }
        Waypoint = 0;
        Loop = 1;
        Next(9);
    }
    else if (Phase == 9)
    {
        if (Approach(Player, DeltaSeconds)) Next(4);
    }
    return false;
}
