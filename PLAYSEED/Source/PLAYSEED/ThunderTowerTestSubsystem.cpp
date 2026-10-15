#include "ThunderTowerTestSubsystem.h"
#include "ThunderTraversalTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderFlightSwitch.h"
#include "ThunderFlightBloom.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedActionComponent.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void TowerKey(APlayerController* PC, FKey Key, bool bPressed) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0)); }
void TowerTap(APlayerController* PC, FKey Key) { TowerKey(PC, Key, true); TowerKey(PC, Key, false); }
}
void UThunderTowerTestSubsystem::SetPhase(int32 Number)
{
    Phase = Number; PhaseTime = 0;
    UE_LOG(LogTemp, Display, TEXT("TOWER TEST: phase %d"), Number);
}
void UThunderTowerTestSubsystem::BeginBloomRoute()
{
    Route = {FVector(-3000, -600, 82), FVector(-3000, 2800, 82), Stage->GetFlightBloom()->GetActorLocation() + FVector(0, 0, 82)};
    Waypoint = 0;
}
void UThunderTowerTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bRestoredProcess)
{
    Stage = InStage; bReadProcess = bRestoredProcess; Error.Reset(); SetPhase(0); BeginBloomRoute();
    RecoveryBefore = Player->RecoveryCount; GrantsBefore = Player->GetFlight()->GetPowerGrants();
    ActivationBefore = Stage->GetFlightSwitch()->GetActivationCount(); PoundsBefore = Player->GetActions()->GetCompletedPounds();
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != 0 || Progress->IsFlightUnlocked() != bReadProcess || Stage->GetFlightSwitch()->IsActivated() != bReadProcess || Stage->GetFlightBloom()->IsAvailable() != bReadProcess)
        Error = TEXT("Tower test needs healthy isolated progress with zero rewards and the expected wing unlock");
}
bool UThunderTowerTestSubsystem::WalkBloomRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint >= Route.Num()) return true;
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    if (Offset.Size2D() < 20 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); ++Waypoint; }
    else
    {
        Player->AddMovementInput(Offset.GetSafeNormal2D());
        auto* PC = Cast<APlayerController>(Player->GetController());
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    }
    return Waypoint >= Route.Num();
}
bool UThunderTowerTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Stage.IsValid() || !PC) { Error = TEXT("Tower stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 90) { Error = FString::Printf(TEXT("Tower phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || Progress->GetEarnedRewardCount() != 0 || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Tower route changed health/rewards or failed to save"); return false; }
    auto* Power = Player->GetFlight(); auto* Move = Player->GetCharacterMovement();
    if (Phase == 0)
    {
        if (WalkBloomRoute(Player, DeltaSeconds))
        {
            if (bReadProcess)
            {
                if (!Power->HasPower() || Power->GetPowerGrants() != GrantsBefore + 1) { Error = TEXT("Restored unlock did not permit an actual flower pickup"); return false; }
                Move->StopMovementImmediately(); TowerKey(PC, EKeys::SpaceBar, true); SetPhase(20);
            }
            else
            {
                if (Power->HasPower() || Power->GetPowerGrants() != GrantsBefore || Stage->GetFlightBloom()->IsAvailable() || !Stage->GetFlightBloom()->IsHidden()) { Error = TEXT("Locked flower granted power before the tower switch"); return false; }
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWingBloomLocked.png"), false, false); SetPhase(1);
            }
        }
    }
    else if (Phase == 1 && PhaseTime > .5f) { TowerTap(PC, EKeys::R); SetPhase(2); }
    else if (Phase == 2 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Progress->IsFlightUnlocked()) { Error = TEXT("Locked-state R changed wing progress"); return false; }
        RecoveryBefore = Player->RecoveryCount;
        GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginTowerRoute(Player, Stage->GetWingTowerFootPath()); SetPhase(3);
    }
    else if (Phase == 3)
    {
        auto* Traverse = GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>(); const bool bDone = Traverse->UpdateRoute(Player, DeltaSeconds);
        if (Traverse->HasFailed()) { Error = Traverse->GetError(); return false; }
        if (bDone)
        {
            if (Progress->IsFlightUnlocked()) { Error = TEXT("Ordinary tower jumps unlocked without a pound"); return false; }
            PC->SetControlRotation(FRotator(-30, -45, 0)); FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWingSwitchClosed.png"), false, false);
            TowerTap(PC, EKeys::LeftMouseButton); SetPhase(4);
        }
    }
    else if (Phase == 4 && PhaseTime > .4f)
    {
        if (Progress->IsFlightUnlocked()) { Error = TEXT("Ordinary punch activated the pound-only switch"); return false; }
        TowerKey(PC, EKeys::SpaceBar, true); SetPhase(5);
    }
    else if (Phase == 5 && PhaseTime > .18f) { TowerKey(PC, EKeys::SpaceBar, false); SetPhase(6); }
    else if (Phase == 6 && PhaseTime > .5f && Move->IsMovingOnGround())
    {
        if (Progress->IsFlightUnlocked() || Player->GetActions()->GetCompletedPounds() != PoundsBefore) { Error = TEXT("Ordinary landing activated the pound-only switch"); return false; }
        TowerKey(PC, EKeys::SpaceBar, true); SetPhase(7);
    }
    else if (Phase == 7 && PhaseTime > .18f) { TowerKey(PC, EKeys::SpaceBar, false); TowerTap(PC, EKeys::LeftControl); SetPhase(8); }
    else if (Phase == 8 && PhaseTime > .4f && Move->IsMovingOnGround())
    {
        if (!Progress->IsFlightUnlocked() || !Stage->GetFlightSwitch()->IsActivated() || Stage->GetFlightSwitch()->GetActivationCount() != ActivationBefore + 1 || !Stage->GetFlightBloom()->IsAvailable()) { Error = TEXT("Actual top-surface ground pound did not save unlock and expose the flower"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWingSwitchOpen.png"), false, false); SetPhase(26);
    }
    else if (Phase == 26 && PhaseTime > .5f) { TowerKey(PC, EKeys::SpaceBar, true); SetPhase(9); }
    else if (Phase == 9 && PhaseTime > .18f) { TowerKey(PC, EKeys::SpaceBar, false); TowerTap(PC, EKeys::LeftControl); SetPhase(10); }
    else if (Phase == 10 && PhaseTime > .4f && Move->IsMovingOnGround())
    {
        if (Stage->GetFlightSwitch()->GetActivationCount() != ActivationBefore + 1 || !Progress->IsFlightUnlocked()) { Error = TEXT("Repeated pound duplicated or lost unlock"); return false; }
        TowerTap(PC, EKeys::R); SetPhase(11);
    }
    else if (Phase == 11 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || !Progress->IsFlightUnlocked() || !Stage->GetFlightBloom()->IsAvailable() || !Stage->GetFlightSwitch()->IsActivated()) { Error = TEXT("R did not preserve switch and flower unlock"); return false; }
        RecoveryBefore = Player->RecoveryCount; BeginBloomRoute(); SetPhase(12);
    }
    else if (Phase == 12)
    {
        WalkBloomRoute(Player, DeltaSeconds);
        if (Power->GetPowerGrants() > GrantsBefore)
        {
            if (Power->GetPowerGrants() != GrantsBefore + 1 || Power->GetRemaining() < 40) { Error = TEXT("Switch-unlocked flower did not grant one fresh power"); return false; }
            Move->StopMovementImmediately(); TowerKey(PC, EKeys::SpaceBar, true); SetPhase(20);
        }
    }
    else if (Phase == 20 && PhaseTime > .18f) { TowerKey(PC, EKeys::SpaceBar, false); SetPhase(21); }
    else if (Phase == 21 && PhaseTime > .15f) { TowerTap(PC, EKeys::SpaceBar); SetPhase(22); }
    else if (Phase == 22 && PhaseTime > .3f)
    {
        if (!Power->IsFlying() || !Power->HasPower()) { Error = TEXT("Unlocked pickup did not permit actual powered flight"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWingsUnlocked.png"), false, false); SetPhase(23);
    }
    else if (Phase == 23 && PhaseTime > .8f) { TowerTap(PC, EKeys::SpaceBar); SetPhase(24); }
    else if (Phase == 24 && PhaseTime > .2f && Move->IsMovingOnGround())
    {
        if (Power->IsFlying() || !Power->HasPower()) { Error = TEXT("Space did not return powered flight to normal gravity and landing"); return false; }
        TowerTap(PC, EKeys::R); SetPhase(25);
    }
    else if (Phase == 25 && PhaseTime > .4f)
    {
        if (Power->HasPower() || Power->IsFlying() || Player->RecoveryCount != RecoveryBefore + 1 || !Progress->IsFlightUnlocked() || !Stage->GetFlightSwitch()->IsActivated() || !Stage->GetFlightBloom()->IsAvailable()) { Error = TEXT("Final R did not clear temporary power and retain persistent switch unlock"); return false; }
        UE_LOG(LogTemp, Display, TEXT("TOWER TEST PASS: %s; actual flower pickup, powered flight, Space cancel, gravity landing and R persistence passed with zero rewards."), bReadProcess ? TEXT("loaded actual saved wing unlock after process restart") : TEXT("locked pickup rejected, ordinary jump/punch/landing excluded, tower climbed with normal jumps, top ground pound saved unlock, duplicate pound ignored"));
        return true;
    }
    return false;
}
