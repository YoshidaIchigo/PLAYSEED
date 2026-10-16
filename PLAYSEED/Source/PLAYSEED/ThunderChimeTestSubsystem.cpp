#include "ThunderChimeTestSubsystem.h"
#include "ThunderSkyChimes.h"
#include "ThunderMeadowStage.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderTowerTestSubsystem.h"
#include "ThunderFlightBloom.h"
#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void ChimeKey(APlayerController* PC, FKey Key, bool bDown) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bDown ? IE_Pressed : IE_Released, bDown ? 1 : 0)); }
void ChimeTap(APlayerController* PC, FKey Key) { ChimeKey(PC, Key, true); ChimeKey(PC, Key, false); }
void ChimeAxis(APlayerController* PC, FKey Key, float Value) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Axis, Value)); }
}
void UThunderChimeTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; UE_LOG(LogTemp, Display, TEXT("CHIME TEST: phase %d"), Number); }
void UThunderChimeTestSubsystem::SetRoute(TArray<FVector> Points, int32 NextPhase) { Route = MoveTemp(Points); Waypoint = 0; SetPhase(NextPhase); }
void UThunderChimeTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); Centre = 0; bEdgeCrossed = false; RecoveryBefore = Player->RecoveryCount;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != (bReadProcess ? 1 : 0) || Progress->HasEarnedReward(5) != bReadProcess || Stage->GetSkyChimes()->GetCoins().Num() != 5) { Error = TEXT("Chime test requires five centres and isolated expected actual reward 5"); return; }
    TSet<FName> Ids; for (APlayseedCoin* Coin : Stage->GetSkyChimes()->GetCoins()) if (Coin) Ids.Add(Coin->GetCoinId());
    if (Ids.Num() != 5) { Error = TEXT("Chime centre IDs are not unique"); return; }
    if (bReadProcess) { SetPhase(30); return; }
    if (Progress->IsLauncherUnlocked() && Progress->IsFlightUnlocked())
    {
        // 操舵の再検証では、前の実プレイで保存した解放を読み直せます。能力や報酬は付与しません。
        GrantsBefore = Player->GetFlight()->GetPowerGrants(); LaunchBefore = Player->GetLaunch()->GetLaunchCount();
        UE_LOG(LogTemp, Display, TEXT("CHIME TEST: using actual saved guide/tower unlocks with zero rewards; still requires real flower and launch."));
        SetRoute({FVector(-3000, -600, 82), FVector(-3000, 2800, 82), Stage->GetFlightBloom()->GetActorLocation() + FVector(0, 0, 82)}, 2); return;
    }
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, Progress->IsLauncherUnlocked()); SetPhase(0);
}
void UThunderChimeTestSubsystem::SteerTo(AWindseedCharacter* Player, FVector Goal)
{
    // 既存の飛行検証と同じ、現在速度を見た先読みをマウス入力へ変換します。
    // 飛行部品の方向・速度・座標そのものは設定しません。
    const FRotator Desired = (Goal - Player->GetActorLocation() - Player->GetVelocity() * .2f).Rotation();
    const auto* Power = Player->GetFlight(); auto* PC = Cast<APlayerController>(Player->GetController());
    ChimeAxis(PC, EKeys::MouseX, FMath::Clamp(FRotator::NormalizeAxis(Desired.Yaw - Power->GetFlightYaw()) * 6, -80.f, 80.f));
    ChimeAxis(PC, EKeys::MouseY, -FMath::Clamp((FMath::Clamp(Desired.Pitch, -60.f, 70.f) - Power->GetFlightPitch()) * 6, -80.f, 80.f));
}
void UThunderChimeTestSubsystem::ClearAxes(AWindseedCharacter* Player)
{
    auto* PC = Cast<APlayerController>(Player->GetController()); ChimeAxis(PC, EKeys::MouseX, 0); ChimeAxis(PC, EKeys::MouseY, 0); ChimeKey(PC, EKeys::S, false); ChimeKey(PC, EKeys::W, false);
}
bool UThunderChimeTestSubsystem::WalkRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint >= Route.Num()) return true;
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    if (Offset.Size2D() < 20 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); ++Waypoint; }
    else { Player->AddMovementInput(Offset.GetSafeNormal2D()); auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); }
    return Waypoint == Route.Num();
}
bool UThunderChimeTestSubsystem::FlyRoute(AWindseedCharacter* Player)
{
    if (Waypoint >= Route.Num()) return true;
    SteerTo(Player, Route[Waypoint]); if (FVector::Dist(Player->GetActorLocation(), Route[Waypoint]) < 150) ++Waypoint;
    return Waypoint == Route.Num();
}
bool UThunderChimeTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Chimes = Stage.IsValid() ? Stage->GetSkyChimes() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>(); auto* Power = Player->GetFlight(); auto* Move = Player->GetCharacterMovement();
    if (!Chimes || !PC) { Error = TEXT("Chime stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 90) { Error = FString::Printf(TEXT("Chime phase%d waypoint%d centre%d at%s power%.1f count%d"), Phase, Waypoint, Centre, *Player->GetActorLocation().ToString(), Power->GetRemaining(), Chimes->GetCollectedCount()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Chime route took damage or save failed"); return false; }
    if (Phase != 0 && Phase != 1 && Phase != 15 && Phase != 31 && Player->RecoveryCount != RecoveryBefore) { Error = TEXT("Unexpected recovery on real chime flight route"); return false; }
    if (Phase < 12 && Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Chime reward saved before actual lantern pickup"); return false; }
    if (Phase >= 6 && Phase <= 10 && !Power->IsFlying()) { Error = FString::Printf(TEXT("Chime flight ended before all actual centres and island approach: phase%d count%d at%s remaining%.1f"), Phase, Chimes->GetCollectedCount(), *Player->GetActorLocation().ToString(), Power->GetRemaining()); return false; }
    if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>(); const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { GetGameInstance()->GetSubsystem<UThunderTowerTestSubsystem>()->BeginTest(Player, Stage.Get(), false); SetPhase(1); }
    }
    else if (Phase == 1)
    {
        auto* Tower = GetGameInstance()->GetSubsystem<UThunderTowerTestSubsystem>(); const bool bDone = Tower->UpdateTest(Player, DeltaSeconds);
        if (!Tower->GetError().IsEmpty()) { Error = Tower->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; GrantsBefore = Power->GetPowerGrants(); LaunchBefore = Player->GetLaunch()->GetLaunchCount(); SetRoute({FVector(-3000, -600, 82), FVector(-3000, 2800, 82), Stage->GetFlightBloom()->GetActorLocation() + FVector(0, 0, 82)}, 2); }
    }
    else if (Phase == 2)
    {
        WalkRoute(Player, DeltaSeconds);
        if (Power->GetPowerGrants() == GrantsBefore + 1 && Power->HasPower()) SetRoute({FVector(-3000, 2200, 82), FVector(-2600, 2000, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-140, 0, 82)}, 3);
    }
    else if (Phase == 3 && WalkRoute(Player, DeltaSeconds)) { ChimeTap(PC, EKeys::G); SetPhase(4); }
    else if (Phase == 4 && PhaseTime > .4f)
    {
        if (!Player->GetLaunch()->IsAiming() || Power->GetRemaining() < 30) { Error = TEXT("Powered actual pad approach did not enter aim with sufficient real time"); return false; }
        ChimeTap(PC, EKeys::LeftMouseButton); SetPhase(5);
    }
    else if (Phase == 5 && PhaseTime > .4f)
    {
        if (!Power->IsFlying() || Player->GetLaunch()->GetLaunchCount() != LaunchBefore + 1) { Error = TEXT("Actual powered launch did not begin flight after pending launch"); return false; }
        ChimeKey(PC, EKeys::S, true); SetRoute({FVector(-2200, 800, 2400), FVector(-2600, 2160, 2400)}, 6);
    }
    else if (Phase == 6 && FlyRoute(Player)) { EdgePrevious = Player->GetActorLocation(); SetPhase(7); }
    else if (Phase == 7)
    {
        SteerTo(Player, FVector(-700, 2160, 2400)); const FVector Position = Player->GetActorLocation(); const FVector Ring = Chimes->GetCoins()[0]->GetActorLocation();
        if (!bEdgeCrossed && EdgePrevious.X < Ring.X && Position.X >= Ring.X)
        {
            const float Alpha = (Ring.X - EdgePrevious.X) / FMath::Max(Position.X - EdgePrevious.X, .001f); const FVector Crossing = FMath::Lerp(EdgePrevious, Position, Alpha);
            const float Radius = FVector::Dist(Crossing, Ring);
            if (Radius < 110 || Radius > 235 || Chimes->GetCollectedCount() != 0 || Chimes->GetCoins()[0]->IsCollected() || Chimes->GetReward()) { Error = FString::Printf(TEXT("Real edge crossing radius%.1f count%d was not inside the ring away from centre"), Radius, Chimes->GetCollectedCount()); return false; }
            bEdgeCrossed = true; UE_LOG(LogTemp, Display, TEXT("CHIME EDGE TEST PASS: actual flight crossed ring plane at radius%.1f without central coin or progress."), Radius);
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChimeEdge.png"), false, false);
        }
        EdgePrevious = Position;
        if (bEdgeCrossed && Position.X > -900) SetRoute({FVector(-2700, 1200, 2400), FVector(-2600, 2000, 2400)}, 8);
    }
    else if (Phase == 8 && FlyRoute(Player)) SetPhase(9);
    else if (Phase == 9)
    {
        while (Centre < 5 && Chimes->GetCoins()[Centre]->IsCollected()) ++Centre;
        if (Centre < 5) SteerTo(Player, Chimes->GetCoins()[Centre]->GetActorLocation());
        else
        {
            if (!bEdgeCrossed || Chimes->GetCollectedCount() != 5 || Chimes->GetRewardGenerationCount() != 1 || !Chimes->GetReward() || Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Five real unique centre coins did not produce one uncollected reward5"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChimeFive.png"), false, false); SetRoute({FVector(900, 2000, 2400)}, 10);
        }
    }
    else if (Phase == 10 && FlyRoute(Player)) SetPhase(11);
    else if (Phase == 11)
    {
        if (Power->IsFlying()) SteerTo(Player, Stage->GetSkyIslandTop() + FVector(0, 0, 60));
        if (Move->IsMovingOnGround())
        {
            ClearAxes(Player); const FVector Top = Stage->GetSkyIslandTop();
            if (FMath::Abs(Player->GetActorLocation().Z - 82 - Top.Z) > 8 || FVector::Dist2D(Player->GetActorLocation(), Top) > 500 || Chimes->GetCollectedCount() != 5 || Chimes->GetRewardGenerationCount() != 1 || Progress->GetEarnedRewardCount() != 0) { Error = TEXT("Actual return flight did not land on island with one uncollected chime reward"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChimeLanding.png"), false, false); SetRoute({Chimes->GetRewardRestLocation() - FVector(0, 0, 18)}, 12);
        }
    }
    else if (Phase == 12 && WalkRoute(Player, DeltaSeconds)) SetPhase(13);
    else if (Phase == 13 && PhaseTime > .4f)
    {
        if (!Progress->HasEarnedReward(5) || Progress->GetEarnedRewardCount() != 1 || !Chimes->GetReward()->IsCollected() || Chimes->GetReward()->GetSuccessfulCollections() != 1 || Chimes->GetRewardGenerationCount() != 1) { Error = TEXT("Actual island lantern pickup did not save reward5 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderChimeReward.png"), false, false); SetPhase(14);
    }
    else if (Phase == 14 && PhaseTime > .5f) { ChimeTap(PC, EKeys::R); SetPhase(15); }
    else if (Phase == 15 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Chimes->GetCollectedCount() != 0 || Chimes->GetReward() || Power->HasPower() || !Progress->HasEarnedReward(5) || Progress->GetEarnedRewardCount() != 1) { Error = TEXT("R did not reset five centres and power while retaining reward5"); return false; }
        for (APlayseedCoin* Coin : Chimes->GetCoins()) if (!Coin || Coin->IsCollected() || Coin->IsHidden()) { Error = TEXT("R did not restore all five actual centre coins"); return false; }
        UE_LOG(LogTemp, Display, TEXT("CHIME TEST PASS: actual guide+tower unlocks, bloom and cannon, edge flight excluded, five unique real central pickups, one reward, real island return landing, actual lantern5, R persistence.")); return true;
    }
    else if (Phase == 30 && PhaseTime > 1)
    {
        if (Chimes->GetCollectedCount() != 0 || Chimes->GetReward() || !Progress->HasEarnedReward(5) || Progress->GetEarnedRewardCount() != 1 || !Progress->IsFlightUnlocked() || !Progress->IsLauncherUnlocked()) { Error = TEXT("Restart did not restore actual chime reward5 and real unlocks with fresh visit"); return false; }
        for (APlayseedCoin* Coin : Chimes->GetCoins()) if (!Coin || Coin->IsCollected() || Coin->IsHidden()) { Error = TEXT("Restart did not restore five centre actors"); return false; }
        ChimeTap(PC, EKeys::R); SetPhase(31);
    }
    else if (Phase == 31 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Chimes->GetCollectedCount() != 0 || !Progress->HasEarnedReward(5) || Progress->GetEarnedRewardCount() != 1) { Error = TEXT("Restart-loaded chime reward lost on R"); return false; }
        UE_LOG(LogTemp, Display, TEXT("CHIME READ TEST PASS: separate process restored actually earned reward5 and unlocks, fresh five centres, R retained reward; no fixtures.")); return true;
    }
    return false;
}
