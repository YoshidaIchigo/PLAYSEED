#include "ThunderScarletTestSubsystem.h"
#include "ThunderScarletTrail.h"
#include "ThunderHundredLantern.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "ThunderMovingPlatform.h"
#include "ThunderRollingBoulder.h"
#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "PlayseedClimbTree.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void ScarletKey(APlayerController* PC, FKey Key, bool bDown) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bDown ? IE_Pressed : IE_Released, bDown ? 1 : 0)); }
void ScarletTap(APlayerController* PC, FKey Key) { ScarletKey(PC, Key, true); ScarletKey(PC, Key, false); }
}
void UThunderScarletTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; UE_LOG(LogTemp, Display, TEXT("SCARLET TEST: phase %d"), Number); }
void UThunderScarletTestSubsystem::SetRoute(TArray<FVector> Points, int32 NextPhase) { Route = MoveTemp(Points); Waypoint = 0; SetPhase(NextPhase); }
void UThunderScarletTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); RecoveryBefore = Player->RecoveryCount; LaunchBefore = Player->GetLaunch()->GetLaunchCount(); HighestFeet = 0;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    bCampaignRun = ThunderCampaignEnabled(); bReadRun = bReadProcess; bSimultaneousShot = false;
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || !RewardBaseline.Begin(Progress, 4, bReadProcess) || Progress->HasEarnedReward(4) != bReadProcess || Stage->GetScarletTrail()->GetCoins().Num() != 8) { Error = TEXT("Scarlet test needs eight target coins and isolated expected actual reward 4 state"); return; }
    TSet<FName> Ids; for (APlayseedCoin* Coin : Stage->GetScarletTrail()->GetCoins()) if (Coin) Ids.Add(Coin->GetCoinId());
    if (Ids.Num() != 8) { Error = TEXT("Scarlet target IDs are not unique"); return; }
    if (bReadProcess) { SetPhase(40); return; }
    if (bCampaignRun)
    {
        if (!Progress->IsLauncherUnlocked() || !Progress->IsFlightUnlocked() || Progress->HasEarnedReward(7)) { Error = TEXT("Final campaign needs the actual earlier unlocks and unearned4/7"); return; }
        TArray<FVector> Points = {FVector(-2800, -600, 82), FVector(-2800, 2500, 82)};
        for (const FVector& Foot : Stage->GetMeadowCoinFootPath()) Points.Add(Foot + FVector(0, 0, 82));
        SetRoute(MoveTemp(Points), 50); return;
    }
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, false); SetPhase(0);
}
bool UThunderScarletTestSubsystem::CheckCount(int32 Expected)
{
    if (Stage->GetScarletTrail()->GetCollectedCount() == Expected && (Expected == 8 || !Stage->GetScarletTrail()->GetReward())) return true;
    Error = FString::Printf(TEXT("Scarlet target count %d expected %d; early reward %d"), Stage->GetScarletTrail()->GetCollectedCount(), Expected, Stage->GetScarletTrail()->GetReward() != nullptr); return false;
}
bool UThunderScarletTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, bool bJump)
{
    const FVector Offset = Goal - Player->GetActorLocation(); auto* Move = Player->GetCharacterMovement();
    if (Offset.Size2D() < 25 && FMath::Abs(Offset.Z) < 65 && Move->IsMovingOnGround()) { Move->StopMovementImmediately(); Player->StopJumping(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 100, .25f, 1.f));
    const auto* Boulder = Stage->GetRollingBoulder();
    const bool bAvoidBoulder = Boulder && !Boulder->IsWaiting() && FVector::Dist2D(Player->GetActorLocation(), Boulder->GetActorLocation()) < 280 && FMath::Abs(Player->GetActorLocation().Z - 82 - (Boulder->GetActorLocation().Z - 70)) < 160;
    if (Move->IsMovingOnGround() && ((bJump && Offset.Z > 50 && Offset.Size2D() < 480) || bAvoidBoulder)) Player->Jump();
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); return false;
}
bool UThunderScarletTestSubsystem::WalkRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint < Route.Num() && Walk(Player, Route[Waypoint], DeltaSeconds, Phase != 16 && Phase != 17 && Phase != 44)) ++Waypoint;
    return Waypoint == Route.Num();
}
bool UThunderScarletTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Trail = Stage.IsValid() ? Stage->GetScarletTrail() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>(); auto* Move = Player->GetCharacterMovement();
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode()); auto* Ledger = Mode ? Mode->GetCoinLedger() : nullptr;
    if (!Trail || !PC || !Ledger) { Error = TEXT("Scarlet stage/controller/ledger missing"); return false; }
    if (!RewardBaseline.HasPrior(Progress)) { Error = TEXT("A previously earned campaign reward was lost"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 90) { Error = FString::Printf(TEXT("Scarlet phase %d waypoint %d timed out at %s targets %d"), Phase, Waypoint, *Player->GetActorLocation().ToString(), Trail->GetCollectedCount()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || !Progress->GetProgressProblem().IsEmpty()) { Error = FString::Printf(TEXT("Scarlet route HP%d phase%d waypoint%d at%s boulder%s; save%s"), Player->GetVitality()->GetHealth(), Phase, Waypoint, *Player->GetActorLocation().ToString(), *Stage->GetRollingBoulder()->GetActorLocation().ToString(), *Progress->GetProgressProblem()); return false; }
    if (Phase != 0 && Phase != 3 && Phase != 28 && Phase != 41 && Player->RecoveryCount != RecoveryBefore) { Error = TEXT("Unexpected recovery during continuous eight-coin route"); return false; }
    if ((Phase < 26 || Phase == 50 || Phase == 51) && Progress->GetEarnedRewardCount() != RewardBaseline.BeforeCount()) { Error = TEXT("Scarlet reward saved before its actual lantern contact"); return false; }
    if (Phase == 50 && WalkRoute(Player, DeltaSeconds))
    {
        if (Ledger->GetTotal() != 80 || Ledger->GetRedCount() != 0 || !CheckCount(0) || Stage->GetHundredLantern()->GetReward()) { if (Error.IsEmpty()) Error = TEXT("Campaign did not really collect80 meadow golds before scarlet route"); return false; }
        UE_LOG(LogTemp, Display, TEXT("CAMPAIGN:80 real meadow golds, starting continuous8scarlets with no R."));
        SetRoute({FVector(-3000, 2500, 82), FVector(-3000, -600, 82)}, 51);
    }
    else if (Phase == 51 && WalkRoute(Player, DeltaSeconds)) SetPhase(4);
    else if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>(); const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; SetPhase(1); }
    }
    else if (Phase == 1)
    {
        if (Walk(Player, Stage->GetTrainingCoins()[1]->GetActorLocation() + FVector(0, 0, 20), DeltaSeconds)) SetPhase(2);
    }
    else if (Phase == 2 && PhaseTime > .3f)
    {
        if (!CheckCount(0) || Ledger->GetRedCount() != 1) { if (Error.IsEmpty()) Error = TEXT("Actual training scarlet pickup did not remain outside the eight target IDs"); return false; }
        ScarletTap(PC, EKeys::R); SetPhase(3);
    }
    else if (Phase == 3 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Ledger->GetTotal() != 0 || !CheckCount(0)) { if (Error.IsEmpty()) Error = TEXT("R did not clear training pickup before continuous target collection"); return false; }
        RecoveryBefore = Player->RecoveryCount; SetPhase(4);
    }
    else if (Phase == 4)
    {
        if (Walk(Player, Stage->GetClimbTree()->GetActorLocation() + FVector(130, 0, 82), DeltaSeconds)) { ScarletTap(PC, EKeys::F); SetPhase(5); }
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (!Player->GetClimb()->IsClimbing()) { Error = TEXT("Actual F did not grab the scarlet tree"); return false; }
        ScarletKey(PC, EKeys::W, true); SetPhase(6);
    }
    else if (Phase == 6 && !Player->GetClimb()->IsClimbing() && Move->IsMovingOnGround())
    {
        ScarletKey(PC, EKeys::W, false); SetPhase(42);
    }
    else if (Phase == 42 && PhaseTime > .3f)
    {
        // 登り終わりは幹から94cm離れた縁です。頂上を普通に歩いて中央のコインへ触れます。
        if (!Walk(Player, Trail->GetCoins()[0]->GetActorLocation() + FVector(0, 0, 20), DeltaSeconds, false)) return false;
        if (!CheckCount(1)) return false;
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderScarletTree.png"), false, false);
        ScarletKey(PC, EKeys::SpaceBar, true); SetPhase(7);
    }
    else if (Phase == 7)
    {
        if (PhaseTime > .18f) ScarletKey(PC, EKeys::SpaceBar, false);
        if (Move->IsFalling()) Player->AddMovementInput(FVector::ForwardVector);
        if (PhaseTime > .4f && Move->IsMovingOnGround())
            SetRoute({FVector(-650, -500, 82), FVector(-650, 350, 82), FVector(-1150, 350, 182), FVector(-1550, 700, 182), FVector(-1650, 1200, 282), FVector(-1450, 1650, 282), FVector(-900, 1650, 382), FVector(-700, 1800, 382)}, 9);
    }
    else if (Phase == 9 && WalkRoute(Player, DeltaSeconds))
    {
        if (!CheckCount(2)) return false;
        TArray<FVector> Points = {FVector(-3000, 1800, 82), FVector(-3100, -600, 82), FVector(-3100, -1200, 82), FVector(-2500, -1200, 182)};
        for (const FVector& Foot : Stage->GetBridgeFootPath()) Points.Add(Foot + FVector(0, 0, 82)); Points.Add(FVector(-1200, -1200, 182)); SetRoute(MoveTemp(Points), 10);
    }
    else if (Phase == 10 && WalkRoute(Player, DeltaSeconds))
    {
        if (!CheckCount(3)) return false;
        SetRoute({FVector(-1300, -1700, 82), FVector(-850, -2100, 262), FVector(-500, -2100, 262), FVector(0, -2100, 262), FVector(850, -2100, 262), FVector(1400, -2100, 82)}, 11);
    }
    else if (Phase == 11 && WalkRoute(Player, DeltaSeconds))
    {
        if (!CheckCount(4)) return false;
        SetRoute({FVector(1400, -900, 82), FVector(850, 300, 82), FVector(850, 500, 82)}, 12);
    }
    else if (Phase == 12 && WalkRoute(Player, DeltaSeconds)) SetPhase(13);
    else if (Phase == 13)
    {
        auto* Platform = Stage->GetMovingPlatform();
        if (Walk(Player, Platform->GetActorLocation() + FVector(0, 0, 112), DeltaSeconds) && Player->GetMovementBaseObject() == Platform->GetRootComponent()) SetPhase(14);
    }
    else if (Phase == 14)
    {
        auto* Platform = Stage->GetMovingPlatform(); const FVector Edge = Platform->GetActorLocation() + FVector(150, 0, 112);
        const bool bReady = Walk(Player, Edge, DeltaSeconds);
        if (bReady && Platform->GetActorLocation().X > 650 && Player->GetMovementBaseObject() == Platform->GetRootComponent()) { Player->Jump(); SetPhase(15); }
    }
    else if (Phase == 15)
    {
        if (Walk(Player, FVector(1250, 800, 302), DeltaSeconds))
        {
            if (!CheckCount(5)) return false; FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderScarletPlatform.png"), false, false);
            TArray<FVector> Points = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
            for (int32 Index = 0; Index <= 2; ++Index) Points.Add(Stage->GetMountainFootPath()[Index] + FVector(0, 0, 82)); SetRoute(MoveTemp(Points), 16);
        }
    }
    else if (Phase == 16 && WalkRoute(Player, DeltaSeconds))
    {
        if (!CheckCount(5)) return false;
        // 岩の終点より下の区間で実際に待ち、通過後に道の縁の赤へ歩きます。
        SetPhase(43);
    }
    else if (Phase == 43 && Stage->GetRollingBoulder()->IsWaiting())
    {
        SetRoute({Stage->GetMountainFootPath()[3] + FVector(0, 0, 82), Trail->GetCoins()[5]->GetActorLocation() + FVector(0, 0, 20)}, 44);
    }
    else if (Phase == 44 && WalkRoute(Player, DeltaSeconds))
    {
        if (!CheckCount(6)) return false; TArray<FVector> Points = {Stage->GetMountainFootPath()[3] + FVector(0, 0, 82)};
        for (int32 Index = 2; Index >= 0; --Index) Points.Add(Stage->GetMountainFootPath()[Index] + FVector(0, 0, 82));
        Points.Append({FVector(600, 1850, 82), FVector(0, 1100, 82), FVector(-600, -600, 82), FVector(-2800, -600, 82), FVector(-3000, 1500, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-140, 0, 82)}); SetRoute(MoveTemp(Points), 17);
    }
    else if (Phase == 17 && WalkRoute(Player, DeltaSeconds)) { ScarletTap(PC, EKeys::G); SetPhase(18); }
    else if (Phase == 18 && PhaseTime > .4f)
    {
        if (!Player->GetLaunch()->IsAiming()) { Error = TEXT("Actual ground approach did not enter Skybloom aim"); return false; }
        LaunchOrigin = Player->GetActorLocation(); ScarletTap(PC, EKeys::LeftMouseButton); SetPhase(19);
    }
    else if (Phase == 19)
    {
        HighestFeet = FMath::Max(HighestFeet, static_cast<float>(Player->GetActorLocation().Z - 82));
        if (PhaseTime > 1 && Move->IsMovingOnGround())
        {
            if (Player->GetLaunch()->GetLaunchCount() != LaunchBefore + 1 || HighestFeet < 1800 || FVector::Dist2D(Player->GetActorLocation(), LaunchOrigin) < 2000 || FMath::Abs(Player->GetActorLocation().Z - 82 - Stage->GetSkyIslandTop().Z) > 5) { Error = TEXT("Actual scarlet route launch did not land on sky island"); return false; }
            SetPhase(20);
        }
    }
    else if (Phase == 20)
    {
        if (Walk(Player, Trail->GetCoins()[6]->GetActorLocation() + FVector(0, 0, 20), DeltaSeconds)) SetPhase(21);
    }
    else if (Phase == 21 && PhaseTime > .3f)
    {
        if (!CheckCount(7)) return false; FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderScarletSky.png"), false, false);
        SetRoute({FVector(-500, 1300, 82), FVector(0, 1100, 82), FVector(-600, -600, 82), FVector(1400, -900, 82), Trail->GetCoins()[7]->GetActorLocation() + FVector(0, 0, 20)}, 22);
    }
    else if (Phase == 22 && WalkRoute(Player, DeltaSeconds)) SetPhase(23);
    else if (Phase == 23 && PhaseTime > .3f)
    {
        if (!CheckCount(8) || Ledger->GetRedCount() != 8 || Ledger->GetTotal() != (bCampaignRun ? 100 : 20) || !Trail->GetReward() || Trail->GetRewardGenerationCount() != 1) { if (Error.IsEmpty()) Error = FString::Printf(TEXT("Eight actual scarlets with the expected real gold baseline, total%d reds%d reward%d"), Ledger->GetTotal(), Ledger->GetRedCount(), Trail->GetReward() != nullptr); return false; }
        if (bCampaignRun && (!Stage->GetHundredLantern()->GetReward() || Stage->GetHundredLantern()->GetReward()->IsCollected() || Stage->GetHundredLantern()->GetGenerationCount() != 1 || Stage->GetHundredLantern()->GetTriggerTotal() != 100 || Trail->GetReward()->IsCollected())) { Error = TEXT("Actual final scarlet did not leave both main4 and bonus7 separately uncollected"); return false; }
        if (bCampaignRun) UE_LOG(LogTemp, Display, TEXT("CAMPAIGN SIMULTANEOUS:84real gold +8scarlet=100; both reward4 and7 exist uncollected in this same World."));
        if (bCampaignRun) { SetPhase(52); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderScarletEight.png"), false, false);
        const FVector Last = Trail->GetCoins()[7]->GetActorLocation() + FVector(0, 0, 20); SetRoute({Last + FVector(-150, 0, 0), Last}, 24);
    }
    else if (Phase == 52)
    {
        auto* Bonus = Stage->GetHundredLantern();
        if (!Trail->GetReward() || !Bonus->GetReward() || Trail->GetReward()->IsCollected() || Bonus->GetReward()->IsCollected() || Progress->GetEarnedRewardCount() != RewardBaseline.BeforeCount()) { Error = TEXT("Both simultaneous rewards did not remain independently uncollected for observation"); return false; }
        const FVector Middle = (Trail->GetRewardRestLocation() + Bonus->GetRewardLocation()) * .5f;
        PC->SetControlRotation(FRotator(-18, (Middle - Player->GetActorLocation()).Rotation().Yaw, 0));
        if (PhaseTime > .3f && !bSimultaneousShot) { bSimultaneousShot = true; FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCampaignBoth.png"), false, false); }
        if (PhaseTime > 1.3f) { const FVector Last = Trail->GetCoins()[7]->GetActorLocation() + FVector(0, 0, 20); SetRoute({Last + FVector(-150, 0, 0), Last}, 24); }
    }
    else if (Phase == 24 && WalkRoute(Player, DeltaSeconds)) SetPhase(25);
    else if (Phase == 25 && PhaseTime > .3f)
    {
        if (!CheckCount(8) || Trail->GetRewardGenerationCount() != 1 || Ledger->GetRedCount() != 8 || Ledger->GetTotal() != (bCampaignRun ? 100 : 20)) { if (Error.IsEmpty()) Error = TEXT("Repeated real scarlet overlap duplicated count/reward"); return false; } SetPhase(26);
    }
    else if (Phase == 26)
    {
        if (Walk(Player, Trail->GetRewardRestLocation() - FVector(0, 0, 18), DeltaSeconds)) SetPhase(27);
    }
    else if (Phase == 27 && PhaseTime > .4f)
    {
        if (!Progress->HasEarnedReward(4) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Trail->GetReward()->IsCollected() || Trail->GetReward()->GetSuccessfulCollections() != 1) { Error = TEXT("Actual court lantern pickup did not save reward 4 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderScarletReward.png"), false, false);
        if (bCampaignRun)
        {
            if (Progress->HasEarnedReward(7) || !Stage->GetHundredLantern()->GetReward() || Stage->GetHundredLantern()->GetReward()->IsCollected()) { Error = TEXT("Main4 pickup consumed or replaced the separate hundred reward7"); return false; }
            SetPhase(45);
        }
        else { ScarletTap(PC, EKeys::R); SetPhase(28); }
    }
    else if (Phase == 45)
    {
        Walk(Player, Stage->GetHundredLantern()->GetRewardLocation() - FVector(0, 0, 18), DeltaSeconds);
        if (Progress->HasEarnedReward(7)) SetPhase(46);
    }
    else if (Phase == 46 && PhaseTime > .4f)
    {
        auto* Bonus = Stage->GetHundredLantern();
        if (!ThunderHasExactRewards(Progress, {1, 2, 3, 4, 5, 6, 7}) || !Bonus->GetReward() || !Bonus->GetReward()->IsCollected() || Bonus->GetReward()->GetSuccessfulCollections() != 1 || Bonus->GetGenerationCount() != 1 || !Trail->GetReward()->IsCollected() || Ledger->GetTotal() != 100) { Error = TEXT("Both real contacts did not retain all7 unique rewards in the same actual campaign save"); return false; }
        UE_LOG(LogTemp, Display, TEXT("CAMPAIGN ALL7 ACTUALLY EARNED: real main4 and bonus7 collected separately in the same visit; saved IDs1,2,3,4,5,6,7."));
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCampaignSeven.png"), false, false); SetPhase(47);
    }
    else if (Phase == 47 && PhaseTime > 1) { ScarletTap(PC, EKeys::R); SetPhase(28); }
    else if (Phase == 28 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Ledger->GetTotal() != 0 || Trail->GetReward() || !CheckCount(0) || !Progress->HasEarnedReward(4) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() + (bCampaignRun && !bReadRun ? 1 : 0)) { if (Error.IsEmpty()) Error = TEXT("R did not clear visit targets and retain earned reward 4"); return false; }
        if (bCampaignRun && (!ThunderHasExactRewards(Progress, {1, 2, 3, 4, 5, 6, 7}) || Stage->GetHundredLantern()->GetReward() || Stage->GetHundredLantern()->HasReachedHundred())) { Error = TEXT("Campaign R lost actual all7 or kept bonus visit state"); return false; }
        for (APlayseedCoin* Coin : Trail->GetCoins()) if (!Coin || Coin->IsCollected() || Coin->IsHidden()) { Error = TEXT("R did not restore all eight target actors"); return false; }
        UE_LOG(LogTemp, Display, TEXT("SCARLET TEST PASS: training excluded, eight unique targets in one continuous tree/terraces/bridge/tilt/moving-base jump/mountain/actuallaunch/island/court route, duplicate excluded, actual lantern4 and R persistence.")); return true;
    }
    else if (Phase == 40 && PhaseTime > 1)
    {
        if (!CheckCount(0) || Trail->GetReward() || Ledger->GetTotal() != 0 || !Progress->HasEarnedReward(4) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount()) { if (Error.IsEmpty()) Error = TEXT("Restart did not load actual reward 4 with a fresh visit"); return false; }
        for (APlayseedCoin* Coin : Trail->GetCoins()) if (!Coin || Coin->IsCollected() || Coin->IsHidden()) { Error = TEXT("Restarted visit did not restore eight targets"); return false; }
        ScarletTap(PC, EKeys::R); SetPhase(41);
    }
    else if (Phase == 41 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || !Progress->HasEarnedReward(4) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !CheckCount(0)) { if (Error.IsEmpty()) Error = TEXT("Restart-loaded scarlet reward lost on R"); return false; }
        UE_LOG(LogTemp, Display, TEXT("SCARLET READ TEST PASS: separate process loaded actually earned reward4, restored eight targets, R retained reward; no fixtures.")); return true;
    }
    return false;
}
