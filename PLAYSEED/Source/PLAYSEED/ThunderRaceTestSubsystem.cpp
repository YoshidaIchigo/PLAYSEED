#include "ThunderRaceTestSubsystem.h"
#include "ThunderCloudflagRace.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderWarpPair.h"
#include "ThunderRollingBoulder.h"
#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedLantern.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace { void RaceTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); } }
void UThunderRaceTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; UE_LOG(LogTemp, Display, TEXT("RACE TEST: phase %d"), Number); }
void UThunderRaceTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); RecoveryBefore = Player->RecoveryCount; LaunchBefore = Player->GetLaunch()->GetLaunchCount(); bWinningAttempt = false;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != (bReadProcess ? 1 : 0) || Progress->HasEarnedReward(2) != bReadProcess) { Error = TEXT("Race test requires isolated expected actual reward 2 state"); return; }
    if (bReadProcess) { SetPhase(30); return; }
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, false); SetPhase(0);
}
bool UThunderRaceTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation(); auto* Move = Player->GetCharacterMovement();
    if (Offset.Size2D() < 30 && FMath::Abs(Offset.Z) < 65 && Move->IsMovingOnGround()) { Move->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 90, .25f, 1.f));
    const auto* Boulder = Stage->GetRollingBoulder();
    if (Boulder && !Boulder->IsWaiting() && FVector::Dist2D(Player->GetActorLocation(), Boulder->GetActorLocation()) < 450 && FMath::Abs(Player->GetActorLocation().Z - 82 - (Boulder->GetActorLocation().Z - 70)) < 160 && Move->IsMovingOnGround()) Player->Jump();
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); return false;
}
void UThunderRaceTestSubsystem::BeginWarpRoute()
{
    Route = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
    for (int32 Index = 0; Index <= 2; ++Index) Route.Add(Stage->GetMountainFootPath()[Index] + FVector(0, 0, 82));
    Route.Add(Stage->GetWarpPair()->GetCaveFloor(0) + FVector(0, 0, 82)); Waypoint = 0;
}
bool UThunderRaceTestSubsystem::WalkRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint < Route.Num() && Walk(Player, Route[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint == Route.Num();
}
bool UThunderRaceTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Race = Stage.IsValid() ? Stage->GetCloudflagRace() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Race || !PC) { Error = TEXT("Race stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 90) { Error = FString::Printf(TEXT("Race phase %d waypoint %d timed out at %s, state %d time %.1f"), Phase, Waypoint, *Player->GetActorLocation().ToString(), static_cast<int32>(Race->GetRaceState()), Race->GetElapsed()); return false; }
    if (!Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Race progress save failed"); return false; }
    if (Phase != 0 && Phase != 8 && Phase != 15 && Phase != 24 && Phase != 31 && Player->RecoveryCount != RecoveryBefore) { Error = TEXT("Unexpected recovery during actual race"); return false; }
    if (Phase < 22 && Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Race reward awarded before real winning pickup"); return false; }
    if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>(); const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; SetPhase(1); }
    }
    else if (Phase == 1 || Phase == 9 || Phase == 16)
    {
        if (Walk(Player, Race->GetStartFloor() + FVector(-150, 0, 82), DeltaSeconds)) { RaceTap(PC, EKeys::G); SetPhase(Phase == 1 ? 2 : Phase == 9 ? 10 : 17); }
    }
    else if (Phase == 2 && PhaseTime > .4f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::Running) { Error = TEXT("Nearby G did not start real race"); return false; }
        if (Race->HasRivalFinished()) { BeginWarpRoute(); SetPhase(3); }
    }
    else if (Phase == 3 || Phase == 18)
    {
        if (Stage->GetWarpPair()->GetWarpCount() == 1)
        {
            Player->GetCharacterMovement()->StopMovementImmediately(); Route.Reset();
            for (int32 Index = 22; Index < Stage->GetMountainFootPath().Num(); ++Index) Route.Add(Stage->GetMountainFootPath()[Index] + FVector(0, 0, 82));
            Waypoint = 0; SetPhase(4);
        }
        else WalkRoute(Player, DeltaSeconds);
    }
    else if (Phase == 4)
    {
        if (WalkRoute(Player, DeltaSeconds)) SetPhase(bWinningAttempt ? 19 : 5);
    }
    else if (Phase == 5 && PhaseTime > .4f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::Lost || !Race->HasRivalFinished() || Race->GetReward() || !Race->UsedWarp()) { Error = TEXT("Actually arriving after rival did not lose without reward"); return false; }
        RaceTap(PC, EKeys::G); FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRaceLost.png"), false, false); SetPhase(6);
    }
    else if (Phase == 6 && PhaseTime > .4f)
    {
        if (Race->GetReward() || Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Lost race granted a lantern through dialogue"); return false; }
        RaceTap(PC, EKeys::R); SetPhase(8);
    }
    else if (Phase == 8 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Race->GetRaceState() != ECloudflagRaceState::Idle || Stage->GetWarpPair()->GetWarpCount() != 0) { Error = TEXT("R did not reset lost race and cave"); return false; }
        RecoveryBefore = Player->RecoveryCount; SetPhase(9);
    }
    else if (Phase == 10 && PhaseTime > .4f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::Running) { Error = TEXT("Retry dialogue did not restart race"); return false; }
        Route = {FVector(-3000, -650, 82), FVector(-3000, 1800, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-160, 0, 82)}; Waypoint = 0; SetPhase(11);
    }
    else if (Phase == 11)
    {
        if (WalkRoute(Player, DeltaSeconds)) { RaceTap(PC, EKeys::G); SetPhase(12); }
    }
    else if (Phase == 12 && PhaseTime > .4f)
    {
        if (!Player->GetLaunch()->IsAiming() || Race->GetRaceState() != ECloudflagRaceState::Running) { Error = TEXT("Actual aim alone did not remain eligible"); return false; }
        RaceTap(PC, EKeys::LeftMouseButton); SetPhase(13);
    }
    else if (Phase == 13 && PhaseTime > .5f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::Disqualified || Player->GetLaunch()->GetLaunchCount() != LaunchBefore + 1 || Race->GetReward()) { Error = TEXT("Actual cannon launch did not disqualify without a reward"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRaceDisqualified.png"), false, false); SetPhase(14);
    }
    else if (Phase == 14 && PhaseTime > .5f) { RaceTap(PC, EKeys::R); SetPhase(15); }
    else if (Phase == 15 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Race->GetRaceState() != ECloudflagRaceState::Idle || Player->GetLaunch()->IsAiming()) { Error = TEXT("R did not clear disqualified flight and reset race"); return false; }
        RecoveryBefore = Player->RecoveryCount; bWinningAttempt = true; SetPhase(16);
    }
    else if (Phase == 17 && PhaseTime > .4f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::Running) { Error = TEXT("Final fair race did not begin"); return false; }
        BeginWarpRoute(); SetPhase(18);
    }
    else if (Phase == 19 && PhaseTime > .4f)
    {
        if (Race->GetRaceState() != ECloudflagRaceState::AwaitingRival || !Race->HasPlayerReachedFlag() || Race->HasRivalFinished() || Race->GetReward() || !Race->UsedWarp()) { Error = TEXT("Actual early flag arrival with allowed warp did not wait for rival"); return false; }
        RaceTap(PC, EKeys::G); FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRaceWaiting.png"), false, false); SetPhase(20);
    }
    else if (Phase == 20)
    {
        if (Race->GetReward()) { Error = TEXT("Dialogue granted reward before rival arrived"); return false; }
        if (Race->GetRaceState() == ECloudflagRaceState::Won)
        {
            if (!Race->HasRivalFinished() || Race->GetPlayerTime() >= Race->GetRivalTime()) { Error = TEXT("Race winner timing invalid"); return false; }
            RaceTap(PC, EKeys::Gamepad_DPad_Down); SetPhase(21);
        }
    }
    else if (Phase == 21 && PhaseTime > .4f)
    {
        if (!Race->GetReward() || Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Winning dialogue did not produce an uncollected lantern"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRaceWon.png"), false, false); SetPhase(22);
    }
    else if (Phase == 22)
    {
        if (Walk(Player, Race->GetFlagFloor() + FVector(-180, 0, 82), DeltaSeconds)) SetPhase(23);
    }
    else if (Phase == 23 && PhaseTime > .4f)
    {
        if (!Progress->HasEarnedReward(2) || Progress->GetEarnedRewardCount() != 1 || !Race->GetReward()->IsCollected() || Race->GetReward()->GetSuccessfulCollections() != 1) { Error = TEXT("Real race lantern pickup did not save reward 2 once"); return false; }
        RaceTap(PC, EKeys::G); SetPhase(25);
    }
    else if (Phase == 25 && PhaseTime > .4f) { if (Race->GetReward()->GetSuccessfulCollections() != 1 || Progress->GetEarnedRewardCount() != 1) { Error = TEXT("Repeat winner dialogue duplicated reward"); return false; } RaceTap(PC, EKeys::R); SetPhase(24); }
    else if (Phase == 24 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Race->GetRaceState() != ECloudflagRaceState::Idle || Race->GetReward() || !Progress->HasEarnedReward(2) || Progress->GetEarnedRewardCount() != 1) { Error = TEXT("R did not reset race and retain earned reward 2"); return false; }
        UE_LOG(LogTemp, Display, TEXT("RACE TEST PASS: real guide unlock, dialogue starts, actual late loss, retry, aim eligible, actual cannon disqualification, allowed cave warp, early flag wait, rival arrival, winner dialogue, actual reward 2, duplicate and R persistence.")); return true;
    }
    else if (Phase == 30 && PhaseTime > 1)
    {
        if (Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(2) || Race->GetRaceState() != ECloudflagRaceState::Idle || Race->GetReward()) { Error = TEXT("Restart did not restore actual race reward 2"); return false; }
        RaceTap(PC, EKeys::R); SetPhase(31);
    }
    else if (Phase == 31 && PhaseTime > .4f)
    {
        if (Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(2) || Player->RecoveryCount != RecoveryBefore + 1) { Error = TEXT("Restart-loaded race reward lost on R"); return false; }
        UE_LOG(LogTemp, Display, TEXT("RACE READ TEST PASS: separate process loaded actually earned reward 2, R retained it; no fixtures.")); return true;
    }
    return false;
}
