#include "ThunderCombatTestSubsystem.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void CombatTap(APlayerController* PC, FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0));
}
}

void UThunderCombatTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedEnemyBase* InEnemy)
{
    Enemy = InEnemy;
    Error.Reset();
    Phase = 0;
    PhaseTime = 0;
    RouteIndex = 0;
    RecoveryBaseline = Player->RecoveryCount;
    ReceivedBaseline = Enemy->GetReceivedStrikes();
    SuccessfulBaseline = Player->GetCombat()->GetSuccessfulHits();
    SerialBaseline = Player->GetCombat()->GetAttackSerial();
    DamageBaseline = Player->GetVitality()->GetSuccessfulImpacts();
    bSpamSent = false;
    bKickSaved = false;
    bRecoverySaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController())) CombatTap(PC, EKeys::LeftMouseButton);
}

bool UThunderCombatTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderCombatTestSubsystem::WalkFacingRoute(AWindseedCharacter* Player, float DeltaSeconds, bool bStopBeforeLast)
{
    const FVector CombatFacingPoints[] = {FVector(-200, -170, 82), FVector(-200, 0, 82), FVector(-140, 0, 82)};
    const int32 Count = bStopBeforeLast ? 2 : 3;
    if (RouteIndex < Count && WalkToward(Player, Enemy->GetActorLocation() + CombatFacingPoints[RouteIndex], DeltaSeconds)) ++RouteIndex;
    return RouteIndex >= Count;
}

bool UThunderCombatTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Enemy.IsValid()) { Error = TEXT("Combat target is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 25) { Error = FString::Printf(TEXT("Combat phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) { Error = TEXT("Combat controller is missing"); return false; }
    auto* Combat = Player->GetCombat();
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (Phase == 0 && PhaseTime > .3f)
    {
        if (Combat->GetAttackSerial() != SerialBaseline + 1 || Enemy->GetReceivedStrikes() != ReceivedBaseline || Enemy->GetMeleeHealth() != 4)
        { Error = TEXT("Out-of-range attack damaged the target"); return false; }
        Advance(1);
    }
    else if (Phase == 1)
    {
        const FVector CombatSidePoints[] = {FVector(600, -600, 82), FVector(600, -1650, 82), FVector(300, -1650, 82)};
        if (RouteIndex < 3 && WalkToward(Player, CombatSidePoints[RouteIndex], DeltaSeconds)) ++RouteIndex;
        if (RouteIndex == 3) { CombatTap(PC, EKeys::LeftMouseButton); Advance(2); }
    }
    else if (Phase == 2 && PhaseTime > .3f)
    {
        if (Enemy->GetReceivedStrikes() != ReceivedBaseline || Enemy->GetMeleeHealth() != 4 || Combat->GetComboStage() != 1)
        { Error = TEXT("Side-facing attack hit or chain timeout did not reset"); return false; }
        RouteIndex = 0;
        Advance(3);
    }
    else if (Phase == 3)
    {
        if (WalkFacingRoute(Player, DeltaSeconds))
        {
            ComboSerial = Combat->GetAttackSerial();
            CombatTap(PC, EKeys::LeftMouseButton);
            Advance(4);
        }
    }
    else if (Phase == 4)
    {
        if (PhaseTime > .06f && !bSpamSent) { CombatTap(PC, EKeys::LeftMouseButton); bSpamSent = true; }
        if (PhaseTime > .3f)
        {
            if (Combat->GetAttackSerial() != ComboSerial + 1 || Combat->GetComboStage() != 1 || Enemy->GetMeleeHealth() != 3 || Enemy->GetReceivedStrikes() != ReceivedBaseline + 1)
            { Error = TEXT("First punch or cooldown/deduplication failed"); return false; }
            CombatTap(PC, EKeys::Gamepad_RightTrigger);
            Advance(5);
        }
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (Combat->GetComboStage() != 2 || Combat->GetAttackSerial() != ComboSerial + 2 || Enemy->GetMeleeHealth() != 2 || Enemy->GetReceivedStrikes() != ReceivedBaseline + 2)
        { Error = TEXT("RT second punch did not hit once in the chain"); return false; }
        CombatTap(PC, EKeys::LeftMouseButton);
        Advance(6);
    }
    else if (Phase == 6)
    {
        if (PhaseTime > .08f && !bKickSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderKick.png"), false, false);
            bKickSaved = true;
        }
        if (PhaseTime > .3f)
        {
            if (!Enemy->IsDefeated() || Combat->GetComboStage() != 3 || Combat->GetAttackSerial() != ComboSerial + 3 || Enemy->GetReceivedStrikes() != ReceivedBaseline + 3 || Combat->GetSuccessfulHits() != SuccessfulBaseline + 3 || Player->GetVitality()->GetHealth() != 8)
            { Error = TEXT("Third kick did not defeat once with three unique strikes"); return false; }
            UE_LOG(LogTemp, Display, TEXT("COMBAT TEST: range/arc rejection, chain timeout, LMB/RT punch-punch-kick, cooldown and one hit per swing passed."));
            CombatTap(PC, EKeys::R);
            Advance(7);
        }
    }
    else if (Phase == 7 && PhaseTime > .3f)
    {
        if (Combat->IsAttacking() || Combat->GetComboStage() != 0 || Enemy->IsDefeated() || Enemy->GetMeleeHealth() != 4)
        { Error = TEXT("R did not reset the chain and revive the target"); return false; }
        RouteIndex = 0;
        Advance(8);
    }
    else if (Phase == 8)
    {
        if (WalkFacingRoute(Player, DeltaSeconds)) { CombatTap(PC, EKeys::LeftMouseButton); Advance(9); }
    }
    else if (Phase == 9 && PhaseTime > .08f)
    {
        if (!Combat->IsAttacking() || Enemy->GetReceivedStrikes() != ReceivedBaseline + 4) { Error = TEXT("Interruptible punch did not begin and hit"); return false; }
        CombatTap(PC, EKeys::R);
        Advance(10);
    }
    else if (Phase == 10 && PhaseTime > .05f)
    {
        if (Combat->IsAttacking() || Combat->GetComboStage() != 0 || Enemy->GetMeleeHealth() != 4 || Player->RecoveryCount != RecoveryBaseline + 2)
        { Error = TEXT("R did not cancel the active punch"); return false; }
        RouteIndex = 0;
        Advance(11);
    }
    else if (Phase == 11)
    {
        if (WalkFacingRoute(Player, DeltaSeconds, true)) Advance(12);
    }
    else if (Phase == 12)
    {
        Player->AddMovementInput((Enemy->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D());
        const double Distance = FVector::Dist2D(Player->GetActorLocation(), Enemy->GetActorLocation());
        if (Distance < 150) { CombatTap(PC, EKeys::LeftMouseButton); Advance(13); }
    }
    else if (Phase == 13)
    {
        if (Player->GetVitality()->GetSuccessfulImpacts() == DamageBaseline + 1)
        {
            if (Combat->IsAttacking() || Combat->GetComboStage() != 0 || Enemy->GetReceivedStrikes() != ReceivedBaseline + 5)
            { Error = TEXT("Real contact damage did not cancel the moving punch"); return false; }
            CombatTap(PC, EKeys::R);
            Advance(14);
        }
        else Player->AddMovementInput((Enemy->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D());
    }
    else if (Phase == 14 && PhaseTime > .3f)
    {
        if (Combat->IsAttacking() || Combat->GetComboStage() != 0 || Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 3 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("Recovery after real combat damage retained attack state"); return false; }
        Advance(15);
    }
    else if (Phase == 15)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, -40, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bRecoverySaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCombatRecovery.png"), false, false);
            bRecoverySaved = true;
            UE_LOG(LogTemp, Display, TEXT("COMBAT TEST PASS: range, arc, chain, three unique strikes, R cancellation, real damage cancellation and recovery passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
