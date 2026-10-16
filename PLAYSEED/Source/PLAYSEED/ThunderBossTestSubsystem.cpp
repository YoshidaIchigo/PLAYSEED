#include "ThunderBossTestSubsystem.h"
#include "ThunderStormkeeper.h"
#include "PlayseedLantern.h"
#include "ThunderMeadowStage.h"
#include "ThunderTraversalTestSubsystem.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace { void BossTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); } }
void UThunderBossTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; UE_LOG(LogTemp, Display, TEXT("BOSS TEST: phase %d"), Number); }
void UThunderBossTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); RecoveryBefore = Player->RecoveryCount; bInvalidThrowDone = false; ThrowsBefore = RouteIndex = 0;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || !RewardBaseline.Begin(Progress, 1, bReadProcess) || Progress->HasEarnedReward(1) != bReadProcess) { Error = TEXT("Boss test needs actual isolated progress with expected reward 1 state"); return; }
    if (bReadProcess) { SetPhase(20); return; }
    TArray<FVector> Path = Stage->GetMountainFootPath(); Path.Pop(); // 中央の敵へ接触せず、山頂入口まで実際に歩きます。
    GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginMountainRoute(Player, Path); SetPhase(0);
}
bool UThunderBossTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, float Tolerance)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < Tolerance && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 90, .22f, 1.f));
    auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-25, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); return false;
}
bool UThunderBossTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Boss = Stage.IsValid() ? Stage->GetStormkeeper() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>(); auto* Move = Player->GetCharacterMovement();
    if (!Boss || !PC) { Error = TEXT("Boss stage/controller missing"); return false; }
    if (!RewardBaseline.HasPrior(Progress)) { Error = TEXT("A previously earned campaign reward was lost"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 100) { Error = FString::Printf(TEXT("Boss phase %d timed out at %s, enemy %s yaw %.1f, valid %d held %d recovering %d"), Phase, *Player->GetActorLocation().ToString(), *Boss->GetActorLocation().ToString(), Boss->GetActorRotation().Yaw, Boss->GetValidThrows(), Boss->IsHeld(), Boss->IsRecovering()); return false; }
    if (!Progress->GetProgressProblem().IsEmpty() || (Phase < 10 && Progress->GetEarnedRewardCount() != RewardBaseline.BeforeCount())) { Error = TEXT("Boss reward was saved before its real collection or save failed"); return false; }
    if (Player->RecoveryCount != RecoveryBefore && Phase != 13 && Phase != 21) { Error = TEXT("Unexpected recovery during real boss traversal"); return false; }
    const FVector Center = Boss->GetArenaFloor();
    if (Phase == 0)
    {
        auto* Traverse = GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>(); const bool bDone = Traverse->UpdateRoute(Player, DeltaSeconds);
        if (Traverse->HasFailed()) { Error = Traverse->GetError(); return false; }
        if (bDone) SetPhase(1);
    }
    else if (Phase == 1)
    {
        if (Walk(Player, Boss->GetActorLocation() + Boss->GetActorForwardVector() * 145, DeltaSeconds, 4)) { BossTap(PC, EKeys::E); SetPhase(2); }
    }
    else if (Phase == 2 && PhaseTime > 1.4f && Move->IsMovingOnGround())
    {
        if (Boss->GetFrontCounters() != 1 || Boss->IsHeld() || Player->GetVitality()->GetHealth() != 6) { Error = FString::Printf(TEXT("Front pickup: counters %d held %d HP %d enemy %s hero %s invulnerable %d"), Boss->GetFrontCounters(), Boss->IsHeld(), Player->GetVitality()->GetHealth(), *Boss->GetActorLocation().ToString(), *Player->GetActorLocation().ToString(), Player->GetVitality()->IsInvulnerable()); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBossCounter.png"), false, false); SetPhase(3);
    }
    else if (Phase == 3)
    {
        // 旋回が遅い敵の外側を通常移動で回り、背後に到達してから近づきます。
        const FVector Relative = Player->GetActorLocation() - Boss->GetActorLocation(); const float Dot = FVector::DotProduct(Relative.GetSafeNormal2D(), Boss->GetActorForwardVector());
        if (Dot < -.65f) SetPhase(4);
        else Walk(Player, Boss->GetActorLocation() + Relative.GetSafeNormal2D().RotateAngleAxis(45, FVector::UpVector) * 260, DeltaSeconds);
    }
    else if (Phase == 4)
    {
        if (Walk(Player, Boss->GetActorLocation() - Boss->GetActorForwardVector() * 155, DeltaSeconds, 10)) { BossTap(PC, EKeys::E); SetPhase(5); }
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (!Boss->IsHeldBy(Player)) { Error = TEXT("Actual rear E did not pick up the keeper"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBossHeld.png"), false, false);
        CarryRoute = bInvalidThrowDone ? TArray<FVector>{Center + FVector(-300, -250, 82), Center + FVector(-250, 0, 82), Center + FVector(-150, 0, 82)} : TArray<FVector>{Center + FVector(350, -100, 82), Center + FVector(480, -100, 82)};
        RouteIndex = 0; SetPhase(6);
    }
    else if (Phase == 6)
    {
        if (Walk(Player, CarryRoute[RouteIndex], DeltaSeconds, 10)) ++RouteIndex;
        if (RouteIndex == CarryRoute.Num()) { ThrowsBefore = Boss->GetValidThrows(); BossTap(PC, EKeys::E); SetPhase(7); }
    }
    else if (Phase == 7 && PhaseTime > 2.5f && !Boss->IsRecovering() && !Boss->IsInThrownFlight())
    {
        if (!bInvalidThrowDone)
        {
            if (Boss->GetValidThrows() != 0 || Boss->GetOffArenaReturns() != 1 || Boss->GetReward()) { Error = TEXT("Off-arena physics throw counted as a summit hit"); return false; }
            bInvalidThrowDone = true; UE_LOG(LogTemp, Display, TEXT("BOSS TEST: actual off-arena throw returned without credit.")); SetPhase(3);
        }
        else
        {
            if (Boss->GetValidThrows() != ThrowsBefore + 1) { Error = TEXT("Actual on-summit physical throw did not count once"); return false; }
            if (Boss->GetValidThrows() == 3) { if (!Boss->IsDefeated() || !Boss->GetReward()) { Error = TEXT("Three real throws did not produce reward 1"); return false; } SetPhase(10); }
            else SetPhase(3);
        }
    }
    else if (Phase == 10)
    {
        if (Walk(Player, Center + FVector(0, 0, 82), DeltaSeconds)) SetPhase(11);
    }
    else if (Phase == 11 && PhaseTime > .5f)
    {
        if (!Progress->HasEarnedReward(1) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Boss->GetReward()->IsCollected() || Boss->GetReward()->GetSuccessfulCollections() != 1 || Player->GetVitality()->GetHealth() != 6) { Error = TEXT("Real lantern pickup did not save reward 1 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBossReward.png"), false, false); SetPhase(12);
    }
    else if (Phase == 12)
    {
        if (Walk(Player, Center + FVector(-250, 0, 82), DeltaSeconds)) SetPhase(14);
    }
    else if (Phase == 14)
    {
        if (Walk(Player, Center + FVector(0, 0, 82), DeltaSeconds)) { if (Boss->GetReward()->GetSuccessfulCollections() != 1 || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount()) { Error = TEXT("Repeated overlap duplicated lantern"); return false; } BossTap(PC, EKeys::R); SetPhase(13); }
    }
    else if (Phase == 13 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Boss->IsDefeated() || Boss->GetValidThrows() != 0 || Boss->GetReward() || Player->GetVitality()->GetHealth() != 8 || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Progress->HasEarnedReward(1)) { Error = TEXT("R did not reset battle and retain earned reward"); return false; }
        RecoveryBefore = Player->RecoveryCount; UE_LOG(LogTemp, Display, TEXT("BOSS TEST PASS: normal mountain walk, front counter, rear lifts, real off-arena throw excluded, three physical summit throws, actual lantern pickup, duplicate and R persistence.")); return true;
    }
    else if (Phase == 20 && PhaseTime > 1)
    {
        if (Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Progress->HasEarnedReward(1) || Boss->IsDefeated() || Boss->GetReward()) { Error = TEXT("Restart did not load actually earned reward 1 with fresh battle"); return false; }
        BossTap(PC, EKeys::R); SetPhase(21);
    }
    else if (Phase == 21 && PhaseTime > .4f)
    {
        if (Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Progress->HasEarnedReward(1) || Player->RecoveryCount != RecoveryBefore + 1) { Error = TEXT("Restart-loaded reward was lost on R"); return false; }
        RecoveryBefore = Player->RecoveryCount; UE_LOG(LogTemp, Display, TEXT("BOSS READ TEST PASS: a separate process loaded actual boss reward 1 and retained it on R; no reward fixtures.")); return true;
    }
    return false;
}
