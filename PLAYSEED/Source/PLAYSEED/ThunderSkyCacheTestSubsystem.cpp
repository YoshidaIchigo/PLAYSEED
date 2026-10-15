#include "ThunderSkyCacheTestSubsystem.h"
#include "ThunderSkyCache.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderMeadowStage.h"
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

namespace { void SkyCacheTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); } }
void UThunderSkyCacheTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; Waypoint = 0; UE_LOG(LogTemp, Display, TEXT("SKY CACHE TEST: phase %d"), Number); }
void UThunderSkyCacheTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); RecoveryBefore = Player->RecoveryCount; LaunchBefore = Player->GetLaunch()->GetLaunchCount(); HighestFeet = 0;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != (bReadProcess ? 1 : 0) || Progress->HasEarnedReward(3) != bReadProcess) { Error = TEXT("Sky cache requires isolated expected actual reward 3 state"); return; }
    if (bReadProcess) { SetPhase(20); return; }
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, false); SetPhase(0);
}
bool UThunderSkyCacheTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 10 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 90, .2f, 1.f));
    auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); return false;
}
bool UThunderSkyCacheTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Box = Stage.IsValid() ? Stage->GetSkyCache() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>(); auto* Move = Player->GetCharacterMovement();
    if (!Box || !PC) { Error = TEXT("Sky cache stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 60) { Error = FString::Printf(TEXT("Sky cache phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Sky route took damage or save failed"); return false; }
    if (Phase != 0 && Phase != 11 && Phase != 21 && Player->RecoveryCount != RecoveryBefore) { Error = TEXT("Unexpected recovery on actual ground-to-island route"); return false; }
    if (Phase < 8 && Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Sky reward awarded before actual box pickup"); return false; }
    if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>(); const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; Route = {FVector(-2800, -600, 82), FVector(-3000, 1500, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-140, 0, 82)}; SetPhase(1); }
    }
    else if (Phase == 1 || Phase == 5)
    {
        if (Waypoint < Route.Num() && Walk(Player, Route[Waypoint], DeltaSeconds)) ++Waypoint;
        if (Waypoint == Route.Num()) { SkyCacheTap(PC, Phase == 1 ? EKeys::G : EKeys::LeftMouseButton); SetPhase(Phase == 1 ? 2 : 6); }
    }
    else if (Phase == 2 && PhaseTime > .4f)
    {
        if (!Player->GetLaunch()->IsAiming() || !FMath::IsNearlyEqual(Player->GetLaunch()->GetAimPitch(), 70.f, .1f)) { Error = TEXT("Actual nearby G did not enter default Skybloom aim"); return false; }
        LaunchOrigin = Player->GetActorLocation(); SkyCacheTap(PC, EKeys::LeftMouseButton); SetPhase(3);
    }
    else if (Phase == 3)
    {
        HighestFeet = FMath::Max(HighestFeet, static_cast<float>(Player->GetActorLocation().Z - 82));
        if (PhaseTime > 1 && Move->IsMovingOnGround())
        {
            const FVector Island = Stage->GetSkyIslandTop(); const FVector Position = Player->GetActorLocation();
            if (Player->GetLaunch()->GetLaunchCount() != LaunchBefore + 1 || FVector::Dist2D(Position, LaunchOrigin) < 2000 || HighestFeet < 1800 || FMath::Abs(Position.Z - 82 - Island.Z) > 5 || FMath::Abs(Position.X - Island.X) > 500 || FMath::Abs(Position.Y - Island.Y) > 500 || Box->IsBroken() || Box->GetReward()) { Error = TEXT("Actual ballistic flight did not land on the island before opening its box"); return false; }
            UE_LOG(LogTemp, Display, TEXT("SKY CACHE TEST: actual ground launch landed on island at %s, peak feet %.1f."), *Position.ToString(), HighestFeet);
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSkyCacheLanding.png"), false, false); SetPhase(4);
        }
    }
    else if (Phase == 4 && PhaseTime > .5f)
    {
        const FVector Floor = Box->GetActorLocation(); Route = {Floor + FVector(-300, -150, 82), Floor + FVector(-300, 0, 82), Floor + FVector(-145, 0, 82)}; SetPhase(5);
    }
    else if (Phase == 6 && PhaseTime > .4f)
    {
        if (!Box->IsBroken() || Box->GetRewardGenerationCount() != 1 || !Box->GetReward() || Progress->GetEarnedRewardCount() != 0 || !Box->GetDropCoins().IsEmpty()) { Error = TEXT("Actual island strike did not create one uncollected reward 3 without coins"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSkyCacheOpen.png"), false, false); SkyCacheTap(PC, EKeys::LeftMouseButton); SetPhase(7);
    }
    else if (Phase == 7 && PhaseTime > .4f)
    {
        if (Box->GetRewardGenerationCount() != 1 || Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Repeated strike duplicated sky lantern"); return false; } SetPhase(8);
    }
    else if (Phase == 8)
    {
        if (Walk(Player, Box->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds)) SetPhase(9);
    }
    else if (Phase == 9 && PhaseTime > .4f)
    {
        if (!Progress->HasEarnedReward(3) || Progress->GetEarnedRewardCount() != 1 || !Box->GetReward()->IsCollected() || Box->GetReward()->GetSuccessfulCollections() != 1) { Error = TEXT("Actual sky lantern contact did not save reward 3 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSkyCacheReward.png"), false, false); SetPhase(10);
    }
    else if (Phase == 10 && PhaseTime > .5f) { SkyCacheTap(PC, EKeys::R); SetPhase(11); }
    else if (Phase == 11 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Box->IsBroken() || Box->GetReward() || !Progress->HasEarnedReward(3) || Progress->GetEarnedRewardCount() != 1 || Player->GetLaunch()->IsAiming()) { Error = TEXT("R did not restore box/ground player and preserve reward 3"); return false; }
        UE_LOG(LogTemp, Display, TEXT("SKY CACHE TEST PASS: actual guide, ground walk, real launch arc, island floor landing, normal strike, one lantern3, repeated strike rejection, actual pickup and R persistence.")); return true;
    }
    else if (Phase == 20 && PhaseTime > 1)
    {
        if (Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(3) || Box->IsBroken() || Box->GetReward()) { Error = TEXT("Restart did not load actual sky reward 3 with fresh box"); return false; }
        SkyCacheTap(PC, EKeys::R); SetPhase(21);
    }
    else if (Phase == 21 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(3)) { Error = TEXT("Restarted sky reward lost on R"); return false; }
        UE_LOG(LogTemp, Display, TEXT("SKY CACHE READ TEST PASS: separate process restored actually earned reward 3 and R retained it; no fixtures.")); return true;
    }
    return false;
}
