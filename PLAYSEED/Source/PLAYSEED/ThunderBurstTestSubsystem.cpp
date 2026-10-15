#include "ThunderBurstTestSubsystem.h"
#include "PlayseedBurstEnemy.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void BurstTap(APlayerController* PC, FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0));
}
}

void UThunderBurstTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedBurstEnemy* InEnemy)
{
    Enemy = InEnemy;
    Phase = Cycle = Waypoint = 0;
    PhaseTime = 0;
    Error.Reset();
    ExplosionBaseline = Enemy->GetExplosionCount();
    HitsBaseline = Enemy->GetBlastHits();
    RecoveryBaseline = Player->RecoveryCount;
    bRecoverySaved = false;
}

bool UThunderBurstTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderBurstTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector BurstApproachPoints[] = {FVector(-600, -900, 82), FVector(1900, -900, 82), FVector(1900, -2850, 82), Enemy->GetActorLocation() + FVector(0, -550, 82)};
    if (Waypoint < 4 && WalkToward(Player, BurstApproachPoints[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint >= 4;
}

bool UThunderBurstTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Enemy.IsValid() || !PC) { Error = TEXT("Burst enemy or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 30) { Error = FString::Printf(TEXT("Burst cycle %d phase %d timed out at %s"), Cycle, Phase, *Player->GetActorLocation().ToString()); return false; }
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (Approach(Player, DeltaSeconds))
        {
            if (Enemy->IsFuseActive() || Enemy->IsDefeated()) { Error = TEXT("Burst fuse began outside its range"); return false; }
            Next(1);
        }
    }
    else if (Phase == 1)
    {
        const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
        if (Cycle == 3 && Enemy->IsFuseActive() && Enemy->GetFuseAge() > .12f)
        {
            BurstTap(PC, EKeys::R);
            Cycle = 4;
            Next(6);
        }
        else if (Cycle == 2 && Offset.Size2D() < 165 && Player->GetCharacterMovement()->IsMovingOnGround())
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            BurstTap(PC, EKeys::LeftMouseButton);
            Next(7);
        }
        else if (WalkToward(Player, Enemy->GetActorLocation() + FVector(0, Cycle == 2 ? 0 : -220, 82), DeltaSeconds) && Cycle < 2)
        {
            if (!Enemy->IsFuseActive()) { Error = TEXT("Burst fuse did not start on approach"); return false; }
            if (Cycle == 0)
            {
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBurstFuse.png"), false, false);
                Next(2);
            }
            else Next(4);
        }
    }
    else if (Phase == 2)
    {
        if (Enemy->GetExplosionCount() == ExplosionBaseline + 1)
        {
            if (!Enemy->HasExploded() || !Enemy->IsDefeated() || Enemy->GetBlastHits() != HitsBaseline + 1 || Player->GetVitality()->GetHealth() != 6 || Player->GetVitality()->GetLastImpulse().Size2D() < 400)
            { Error = TEXT("Fuse blast did not damage and knock back once"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBurstImpact.png"), false, false);
            Next(3);
        }
    }
    else if (Phase == 3 && PhaseTime > .6f)
    {
        if (Enemy->GetExplosionCount() != ExplosionBaseline + 1 || Enemy->GetBlastHits() != HitsBaseline + 1) { Error = TEXT("Burst explosion repeated"); return false; }
        BurstTap(PC, EKeys::R);
        Cycle = 1;
        Next(6);
    }
    else if (Phase == 4)
    {
        if (WalkToward(Player, Enemy->GetActorLocation() + FVector(-600, 0, 82), DeltaSeconds)) Next(5);
    }
    else if (Phase == 5)
    {
        if (Enemy->GetExplosionCount() == ExplosionBaseline + 2)
        {
            if (Enemy->GetBlastHits() != HitsBaseline + 1 || Player->GetVitality()->GetHealth() != 8) { Error = TEXT("Ordinary retreat during fuse did not avoid the blast"); return false; }
            BurstTap(PC, EKeys::R);
            Cycle = 2;
            Next(6);
        }
    }
    else if (Phase == 6 && PhaseTime > .2f)
    {
        if (Enemy->IsDefeated() || Enemy->IsFuseActive() || Enemy->HasExploded() || Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + Cycle)
        { Error = TEXT("R did not revive the burst enemy and clear fuse/flash"); return false; }
        if (Cycle == 4) Next(9);
        else { Waypoint = 0; Next(0); }
    }
    else if (Phase == 7 && PhaseTime > .3f)
    {
        if (!Enemy->IsDefeated() || Enemy->IsFuseActive() || Enemy->HasExploded() || Enemy->GetExplosionCount() != ExplosionBaseline + 2 || Player->GetVitality()->GetHealth() != 8)
        { Error = TEXT("Melee did not cancel the fuse before the blast"); return false; }
        Next(8);
    }
    else if (Phase == 8 && PhaseTime > 2.2f)
    {
        if (Enemy->GetExplosionCount() != ExplosionBaseline + 2) { Error = TEXT("Defeated fuse exploded later"); return false; }
        BurstTap(PC, EKeys::R);
        Cycle = 3;
        Next(6);
    }
    else if (Phase == 9 && PhaseTime > 2.3f)
    {
        if (Enemy->GetExplosionCount() != ExplosionBaseline + 2 || Enemy->GetBlastHits() != HitsBaseline + 1 || Enemy->IsFuseActive() || Enemy->IsDefeated() || Player->GetVitality()->GetHealth() != 8)
        { Error = TEXT("R during the fuse left a delayed blast"); return false; }
        if (!bRecoverySaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBurstRecovery.png"), false, false);
            bRecoverySaved = true;
            UE_LOG(LogTemp, Display, TEXT("BURST TEST PASS: proximity two-second fuse, one two-damage blast, normal retreat avoids radius, melee cancels fuse, R during fuse prevents delayed blast, four resets and recovery passed."));
        }
        return PhaseTime > 3;
    }
    return false;
}
