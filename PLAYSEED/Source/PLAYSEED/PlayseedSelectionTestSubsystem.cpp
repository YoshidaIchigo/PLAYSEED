#include "PlayseedSelectionTestSubsystem.h"
#include "PlayseedGameModeBase.h"
#include "PlayseedRecordingSubsystem.h"
#include "WindseedCharacter.h"
#include "WindseedStage.h"
#include "ThunderExpeditionSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderTraversalTestSubsystem.h"
#include "ThunderPlatformTestSubsystem.h"
#include "ThunderBalanceTestSubsystem.h"
#include "ThunderGateTestSubsystem.h"
#include "ThunderWarpTestSubsystem.h"
#include "ThunderDamageTestSubsystem.h"
#include "ThunderPoundTestSubsystem.h"
#include "ThunderCarryTestSubsystem.h"
#include "ThunderClimbTestSubsystem.h"
#include "ThunderStompTestSubsystem.h"
#include "ThunderCombatTestSubsystem.h"
#include "ThunderCoinTestSubsystem.h"
#include "ThunderBoxTestSubsystem.h"
#include "ThunderBoulderTestSubsystem.h"
#include "ThunderWaterTestSubsystem.h"
#include "ThunderChaseTestSubsystem.h"
#include "ThunderBurstTestSubsystem.h"
#include "ThunderGaleleafTestSubsystem.h"
#include "ThunderRideTestSubsystem.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderLaunchTestSubsystem.h"
#include "ThunderFlightTestSubsystem.h"
#include "ThunderTowerTestSubsystem.h"
#include "ThunderBossTestSubsystem.h"
#include "ThunderRaceTestSubsystem.h"
#include "ThunderSkyCacheTestSubsystem.h"
#include "ThunderScarletTestSubsystem.h"
#include "ThunderChimeTestSubsystem.h"
#include "ThunderSentinelTestSubsystem.h"
#include "ThunderPostTestSubsystem.h"
#include "ThunderHundredTestSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "ThunderFlightSwitch.h"
#include "ThunderFlightBloom.h"
#include "ThunderLauncher.h"
#include "ThunderDamageThistle.h"
#include "ThunderCoinLedger.h"
#include "ThunderMeadowGameMode.h"
#include "PlayseedCoin.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    void Key(APlayerController* PC, FKey Key, bool bPressed)
    {
        if (PC) PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0));
    }
    void Tap(APlayerController* PC, FKey PressedKey)
    {
        Key(PC, PressedKey, true);
        Key(PC, PressedKey, false);
    }
}

void UPlayseedSelectionTestSubsystem::BeginStage()
{
    Elapsed = 0;
    auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    if (!Recording->StartRecording(TEXT("Validation/WindseedPlaythrough.avi")))
        Fail(TEXT("Stage selection recording could not start"));
    RunProgressTestOnce();
}

void UPlayseedSelectionTestSubsystem::RunProgressTestOnce()
{
    if (bProgressTestExecuted) return;
    bProgressTestExecuted = true;
    const bool bWrite = FParse::Param(FCommandLine::Get(), TEXT("ProgressWriteTest"));
    bProgressTest = bWrite || FParse::Param(FCommandLine::Get(), TEXT("ProgressReadTest"));
    if (!bProgressTest) return;
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Expedition->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Expedition->GetProgressProblem().IsEmpty())
    { Fail(TEXT("Progress test did not use a healthy isolated save slot")); return; }
    // 保存APIの試験データです。ミッションを実際にクリアしたという意味ではありません。
    // 通常プレイのセーブには触れず、別プロセスで同じ検証スロットを読み直します。
    if (bWrite && (UGameplayStatics::DoesSaveGameExist(Expedition->GetSaveSlotName(), 0) || Expedition->GetEarnedRewardCount() != 0 ||
        !Expedition->TryAwardReward(1) || !Expedition->TryAwardReward(7)))
    { Fail(TEXT("Writing fresh isolated progress failed or the test slot already existed")); return; }
    if (Expedition->GetEarnedRewardCount() != 2 || !Expedition->HasEarnedReward(1) || !Expedition->HasEarnedReward(7) ||
        Expedition->TryAwardReward(1) || Expedition->TryAwardReward(0) || Expedition->TryAwardReward(8) ||
        Expedition->GetEarnedRewardCount() != 2 || !Expedition->GetProgressProblem().IsEmpty())
    { Fail(TEXT("Progress restore, duplicate prevention or reward range validation failed")); return; }
    UE_LOG(LogTemp, Display, TEXT("PROGRESS TEST PASS: %s isolated slot %s; rewards 1 and 7, total 2, duplicate and range rejection passed."),
        bWrite ? TEXT("wrote") : TEXT("loaded after process restart"), *Expedition->GetSaveSlotName());
}

void UPlayseedSelectionTestSubsystem::Update(float DeltaSeconds, APlayseedGameModeBase* CurrentMode)
{
    if (Step < 0 || HasPassed()) return;
    Elapsed += DeltaSeconds;
    if (Elapsed > (Step == 43 ? 300 : Step == 42 ? 300 : Step == 41 ? 180 : Step == 40 ? 420 : Step == 37 || Step == 39 ? 360 : Step == 31 || Step == 33 || Step == 34 || Step == 35 || Step == 36 || Step == 38 ? 300 : Step == 10 || (Step >= 12 && Step <= 32) ? 120 : 20)) { Fail(TEXT("Stage selection timed out")); return; }
    auto* PC = UGameplayStatics::GetPlayerController(CurrentMode, 0);
    if (!PC) return;
    const FString Map = UGameplayStatics::GetCurrentLevelName(CurrentMode, true);
    auto* Player = Cast<AWindseedCharacter>(PC->GetPawn());
    // 起動中は Tick が先に進む場合があるため、実際に選択画面を録画できてから入力します。
    const auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const bool bCampaignRun = FParse::Param(FCommandLine::Get(), TEXT("ThunderCampaignTest"));
    const bool bCampaignRead = FParse::Param(FCommandLine::Get(), TEXT("ThunderCampaignReadTest"));
    const bool bHundredTest = FParse::Param(FCommandLine::Get(), TEXT("ThunderHundredTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderHundredReadTest"));
    const int32 CampaignTargets[] = {1, 2, 3, 5, 6, 4};
    const int32 CampaignTarget = CampaignTargets[FMath::Clamp(CampaignIndex, 0, 5)];
    const bool bBossTest = (bCampaignRun && CampaignTarget == 1) || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderBossTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderBossReadTest"))));
    const bool bRaceTest = (bCampaignRun && CampaignTarget == 2) || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderRaceTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderRaceReadTest"))));
    const bool bSkyCacheTest = (bCampaignRun && CampaignTarget == 3) || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderSkyCacheTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderSkyCacheReadTest"))));
    const bool bScarletTest = (bCampaignRun && CampaignTarget == 4) || bCampaignRead || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderScarletTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderScarletReadTest"))));
    const bool bPostTest = (bCampaignRun && CampaignTarget == 6) || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderPostTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderPostReadTest"))));
    const bool bChimeTest = (bCampaignRun && CampaignTarget == 5) || (!bCampaignRun && (FParse::Param(FCommandLine::Get(), TEXT("ThunderChimeTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderChimeReadTest"))));
    if (bProgressTest && (Expedition->GetEarnedRewardCount() != 2 || !Expedition->HasEarnedReward(1) || !Expedition->HasEarnedReward(7)))
    { Fail(TEXT("Saved rewards changed during restart or re-entry")); return; }
    if (Step == 0 && Map == TEXT("StageSelect") && Elapsed > 3 && Recording->GetCapturedFrameCount() > 15)
    {
        if (bCampaignRun && (Expedition->GetEarnedRewardCount() != 0 || Expedition->IsLauncherUnlocked() || Expedition->IsFlightUnlocked() || UGameplayStatics::DoesSaveGameExist(Expedition->GetSaveSlotName(), 0))) { Fail(TEXT("All7 campaign must start with a genuinely fresh isolated save"); return; }
        Tap(PC, EKeys::Two);
        Step = 1;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: pressed 2 to enter Thunder Meadow."));
    }
    else if (Step == 1 && Map == TEXT("MissionSelect") && Elapsed > 3)
    {
        Tap(PC, bSecondMissionEntry || bRaceTest ? EKeys::Two : bBossTest ? EKeys::One : bSkyCacheTest ? EKeys::Three : bScarletTest ? EKeys::Four : bChimeTest ? EKeys::Five : EKeys::Six);
        Step = 7;
    }
    else if (Step == 7 && Map == TEXT("ThunderMeadow") && Player && Elapsed > 2)
    {
        const int32 ExpectedMission = bSecondMissionEntry || bRaceTest ? 2 : bBossTest ? 1 : bSkyCacheTest ? 3 : bScarletTest ? 4 : bChimeTest ? 5 : 6;
        if (Expedition->GetSelectedMissionNumber() != ExpectedMission || Expedition->SelectMission(0) || Expedition->SelectMission(7)
            || Expedition->GetSelectedMissionNumber() != ExpectedMission)
        { Fail(TEXT("Mission selection did not survive travel or accepted an invalid number")); return; }
        if (bSecondMissionEntry)
        {
            if (bCampaignRun || bCampaignRead)
            {
                if (!ThunderHasExactRewards(Expedition, {1, 2, 3, 4, 5, 6, 7}) || !Expedition->IsLauncherUnlocked() || !Expedition->IsFlightUnlocked()) { Fail(TEXT("Actual all7 campaign rewards or real unlocks lost on fresh World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("CAMPAIGN REENTRY TEST PASS: all7 actual saved unique rewards and real guide/tower unlocks retained in a new World."));
                Tap(PC, EKeys::M); Step = 8; return;
            }
            if (bHundredTest && (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(7))) { Fail(TEXT("Actual hundred reward7 lost on new World entry")); return; }
            if (bPostTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(6)) { Fail(TEXT("Actually earned post reward6 lost on new World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("POST REENTRY TEST PASS: actual reward6 retained in new World."));
            }
            if (bChimeTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(5) || !Expedition->IsLauncherUnlocked() || !Expedition->IsFlightUnlocked()) { Fail(TEXT("Chime reward or actual unlocks lost in new World")); return; }
                UE_LOG(LogTemp, Display, TEXT("CHIME REENTRY TEST PASS: actually earned reward5 and real guide/tower unlocks retained in new World."));
            }
            if (bScarletTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(4) || !Expedition->IsLauncherUnlocked()) { Fail(TEXT("Scarlet reward or actual launcher unlock lost on new World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("SCARLET REENTRY TEST PASS: actually earned reward 4 and launcher retained in new World."));
            }
            if (bSkyCacheTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(3) || !Expedition->IsLauncherUnlocked()) { Fail(TEXT("Sky reward or launcher lost on new World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("SKY CACHE REENTRY TEST PASS: actually earned reward 3 and launcher retained in new World."));
            }
            if (bRaceTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(2) || !Expedition->IsLauncherUnlocked()) { Fail(TEXT("Race reward or real guide unlock lost on new World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("RACE REENTRY TEST PASS: actually earned reward 2 and launcher retained in a new World."));
            }
            if (bBossTest)
            {
                if (Expedition->GetEarnedRewardCount() != 1 || !Expedition->HasEarnedReward(1)) { Fail(TEXT("Boss reward lost across new World entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("BOSS REENTRY TEST PASS: actually earned reward 1 retained across new World entry."));
            }
            if (FParse::Param(FCommandLine::Get(), TEXT("ThunderTowerTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderTowerReadTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderFlightTest")))
            {
                auto* ReenteredTower = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
                if (!ReenteredTower || !Expedition->IsFlightUnlocked() || !ReenteredTower->GetFlightSwitch()->IsActivated() || !ReenteredTower->GetFlightBloom()->IsAvailable() || Expedition->GetEarnedRewardCount() != 0)
                { Fail(TEXT("Re-entry lost switch and flower unlock")); return; }
                UE_LOG(LogTemp, Display, TEXT("TOWER REENTRY TEST PASS: saved switch and flower restored in a new World, rewards zero."));
            }
            if (FParse::Param(FCommandLine::Get(), TEXT("ThunderGuideTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderGuideReadTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderLaunchTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderFlightTest")))
            {
                auto* ReenteredMeadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
                if (!ReenteredMeadow || !ReenteredMeadow->GetLauncher() || !ReenteredMeadow->GetLauncher()->IsOpen() || !Expedition->IsLauncherUnlocked() || Expedition->GetEarnedRewardCount() != 0)
                { Fail(TEXT("Re-entry did not retain the unlocked launcher with zero rewards")); return; }
                UE_LOG(LogTemp, Display, TEXT("GUIDE REENTRY TEST PASS: saved launcher is open in a newly created World; quest rewards remain zero."));
            }
            if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCoinTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderCoinTrailTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderBoulderTest")))
            {
                auto* MeadowMode = Cast<AThunderMeadowGameMode>(CurrentMode);
                auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
                if (!MeadowMode || !Meadow || MeadowMode->GetCoinLedger()->GetTotal() != 0 || MeadowMode->GetCoinLedger()->GetRedCount() != 0)
                { Fail(TEXT("Coin totals survived a new stage entry")); return; }
                for (APlayseedCoin* Coin : Meadow->GetTrainingCoins())
                    if (!Coin || Coin->IsCollected()) { Fail(TEXT("Collected coin visibility survived a new entry")); return; }
                for (APlayseedCoin* Coin : Meadow->GetTrailCoins())
                    if (!Coin || Coin->IsCollected()) { Fail(TEXT("A trail coin remained collected after re-entry")); return; }
                UE_LOG(LogTemp, Display, TEXT("COIN TEST: fresh re-entry cleared totals and restored all placed coin actors."));
            }
            Tap(PC, EKeys::M);
            Step = 8;
            UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: a new World entry with mission 2 passed."));
            return;
        }
        TArray<AActor*> Gardens;
        UGameplayStatics::GetAllActorsOfClass(CurrentMode, AWindseedStage::StaticClass(), Gardens);
        TArray<AActor*> Meadows;
        UGameplayStatics::GetAllActorsOfClass(CurrentMode, AThunderMeadowStage::StaticClass(), Meadows);
        if (Gardens.Num() || Meadows.Num() != 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Fail(TEXT("The new stage has mixed actors or a wrong start")); return; }
        InputStart = Player->GetActorLocation();
        PC->SetControlRotation(FRotator(-18, 0, 0));
        Key(PC, EKeys::W, true);
        Key(PC, EKeys::SpaceBar, true);
        Step = 2;
        Elapsed = 0;
    }
    else if (Step == 2 && Elapsed > .4f && Player)
    {
        Key(PC, EKeys::W, false);
        Key(PC, EKeys::SpaceBar, false);
        if (Player->GetActorLocation().X < InputStart.X + 70 || Player->GetActorLocation().Z < InputStart.Z + 40)
        { Fail(TEXT("Movement and jump in the new stage failed")); return; }
        Tap(PC, EKeys::R);
        Step = 3;
        Elapsed = 0;
    }
    else if (Step == 3 && Elapsed > 1 && Player)
    {
        if (FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 || Player->RecoveryCount != 1
            || Expedition->GetSelectedMissionNumber() != (bBossTest ? 1 : bRaceTest ? 2 : bSkyCacheTest ? 3 : bScarletTest ? 4 : bChimeTest ? 5 : 6))
        { Fail(TEXT("Restart in the new stage failed")); return; }
        const bool bIslandTest = FParse::Param(FCommandLine::Get(), TEXT("ThunderIslandTest"));
        if (bHundredTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetHundredLantern()) { Fail(TEXT("Hundred lantern manager missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderHundredTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderHundredReadTest")));
            Step = 43; Elapsed = 0;
        }
        else if (bPostTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetBindingPost() || !Meadow->GetPrisonGate() || !Meadow->GetBoundSentinel()) { Fail(TEXT("Binding post/prison missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderPostTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderPostReadTest")));
            Step = 42; Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderSentinelTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetBoundSentinel()) { Fail(TEXT("Bound sentinel missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderSentinelTestSubsystem>()->BeginTest(Player, Meadow->GetBoundSentinel());
            Step = 41; Elapsed = 0;
        }
        else if (bChimeTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetSkyChimes() || !Meadow->GetFlightBloom() || !Meadow->GetFlightSwitch() || !Meadow->GetLauncher()) { Fail(TEXT("Sky chimes or actual flight prerequisites missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderChimeTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderChimeReadTest")));
            Step = 40; Elapsed = 0;
        }
        else if (bScarletTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetScarletTrail() || !Meadow->GetLauncher()) { Fail(TEXT("Scarlet trail or launcher missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderScarletTestSubsystem>()->BeginTest(Player, Meadow, bCampaignRead || FParse::Param(FCommandLine::Get(), TEXT("ThunderScarletReadTest")));
            Step = 39; Elapsed = 0;
        }
        else if (bSkyCacheTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetSkyCache() || !Meadow->GetLauncher()) { Fail(TEXT("Mission 3 sky cache or launcher missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderSkyCacheTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderSkyCacheReadTest")));
            Step = 38; Elapsed = 0;
        }
        else if (bRaceTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetCloudflagRace() || !Meadow->GetWarpPair() || !Meadow->GetLauncher()) { Fail(TEXT("Mission 2 racer, cave or launcher missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderRaceTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderRaceReadTest")));
            Step = 37; Elapsed = 0;
        }
        else if (bBossTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetStormkeeper()) { Fail(TEXT("Mission 1 keeper missing")); return; }
            GetGameInstance()->GetSubsystem<UThunderBossTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderBossReadTest")));
            Step = 36; Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCoinTrailTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || Meadow->GetMeadowCoinFootPath().Num() != 80 || Meadow->GetTrailCoins().Num() != 100)
            { Fail(TEXT("The hundred-coin walking trail was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginCoinTrailRoute(Player, Meadow->GetMeadowCoinFootPath(), Meadow->GetMountainFootPath());
            Step = 10;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderMountainTest")) || bIslandTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || Meadow->GetMountainFootPath().Num() < 25) { Fail(TEXT("The mountain route was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginMountainRoute(Player, Meadow->GetMountainFootPath(), bIslandTest);
            Step = 10;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderTowerTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderTowerReadTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetFlightSwitch() || !Meadow->GetFlightBloom() || Meadow->GetWingTowerFootPath().Num() != 8) { Fail(TEXT("Wing tower was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderTowerTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderTowerReadTest")));
            Step = 35;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderFlightTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetFlightBloom() || !Meadow->GetLauncher() || !Meadow->GetGuide()) { Fail(TEXT("Flight bloom, launcher and guide were not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderFlightTestSubsystem>()->BeginTest(Player, Meadow);
            Step = 34;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderLaunchTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetLauncher() || !Meadow->GetGuide()) { Fail(TEXT("Launcher and guide were not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderLaunchTestSubsystem>()->BeginTest(Player, Meadow);
            Step = 33;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderGuideTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderGuideReadTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetGuide() || !Meadow->GetLauncher()) { Fail(TEXT("Guide and launcher were not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, Meadow, FParse::Param(FCommandLine::Get(), TEXT("ThunderGuideReadTest")));
            Step = 32;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderRideTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetGaleleafEnemy()) { Fail(TEXT("The ride source enemy was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderRideTestSubsystem>()->BeginTest(Player, Meadow);
            Step = 31;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderGaleleafTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetGaleleafEnemy()) { Fail(TEXT("The galeleaf carrier was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderGaleleafTestSubsystem>()->BeginTest(Player, Meadow->GetGaleleafEnemy());
            Step = 30;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderBurstTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetBurstEnemy()) { Fail(TEXT("The fuse enemy was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderBurstTestSubsystem>()->BeginTest(Player, Meadow->GetBurstEnemy());
            Step = 29;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderChaseTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetChasingEnemy()) { Fail(TEXT("The chasing enemy was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderChaseTestSubsystem>()->BeginTest(Player, Meadow->GetChasingEnemy());
            Step = 28;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderWaterTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetWaterBomb()) { Fail(TEXT("The falling water bomb was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderWaterTestSubsystem>()->BeginTest(Player, Meadow->GetWaterBomb());
            Step = 27;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderBoulderTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetRollingBoulder() || !Meadow->GetDamageThistle() || Meadow->GetMountainFootPath().Num() != 27)
            { Fail(TEXT("The rolling boulder and mountain path were not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderBoulderTestSubsystem>()->BeginTest(Player, Meadow->GetRollingBoulder(), Meadow->GetMountainFootPath(), Meadow->GetDamageThistle()->GetActorLocation());
            Step = 25;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderBoxTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetPracticeBox()) { Fail(TEXT("The practice box was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderBoxTestSubsystem>()->BeginTest(Player, Meadow->GetPracticeBox());
            Step = 23;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCoinTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow) { Fail(TEXT("The coin stage was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderCoinTestSubsystem>()->BeginTest(Player, Meadow);
            Step = 22;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCombatTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetPracticeEnemy()) { Fail(TEXT("The combat target was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderCombatTestSubsystem>()->BeginTest(Player, Meadow->GetPracticeEnemy());
            Step = 21;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderStompTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetPracticeEnemy()) { Fail(TEXT("The practice enemy was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderStompTestSubsystem>()->BeginTest(Player, Meadow->GetPracticeEnemy());
            Step = 20;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderClimbTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetClimbTree()) { Fail(TEXT("The climb tree was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderClimbTestSubsystem>()->BeginTest(Player, Meadow->GetClimbTree());
            Step = 19;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCarryTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetCarryProp()) { Fail(TEXT("The carry prop was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderCarryTestSubsystem>()->BeginTest(Player, Meadow->GetCarryProp());
            Step = 18;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderPoundTest")))
        {
            GetGameInstance()->GetSubsystem<UThunderPoundTestSubsystem>()->BeginTest(Player);
            Step = 17;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderDamageTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetDamageThistle()) { Fail(TEXT("The damage thistle was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderDamageTestSubsystem>()->BeginTest(Player, Meadow->GetDamageThistle());
            Step = 16;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderWarpTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetWarpPair() || Meadow->GetMountainFootPath().Num() != 27) { Fail(TEXT("The warp caves were not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderWarpTestSubsystem>()->BeginTest(Player, Meadow->GetWarpPair(), Meadow->GetMountainFootPath());
            Step = 15;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderGateTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetShortcutGate()) { Fail(TEXT("The shortcut gate was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderGateTestSubsystem>()->BeginTest(Player, Meadow->GetShortcutGate());
            Step = 14;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderBalanceTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetTiltingBridge()) { Fail(TEXT("The tilting bridge was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderBalanceTestSubsystem>()->BeginTest(Player, Meadow->GetTiltingBridge());
            Step = 13;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderPlatformTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || !Meadow->GetMovingPlatform()) { Fail(TEXT("The moving platform was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderPlatformTestSubsystem>()->BeginTest(Player, Meadow->GetMovingPlatform());
            Step = 12;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderBridgeTest")))
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || Meadow->GetBridgeFootPath().Num() != 13) { Fail(TEXT("The wooden bridge was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginBridgeRoute(Player, Meadow->GetBridgeFootPath());
            Step = 10;
            Elapsed = 0;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("ThunderTerraceTest")))
        {
            GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginTerraceRoute(Player);
            Step = 10;
            Elapsed = 0;
        }
        else { Tap(PC, EKeys::M); Step = 4; }
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: Thunder movement, jump and R passed."));
    }
    else if (Step == 10 && Player)
    {
        auto* Traversal = GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>();
        const bool bComplete = Traversal->UpdateRoute(Player, DeltaSeconds);
        if (Traversal->HasFailed()) { Fail(Traversal->GetError()); return; }
        if (bComplete)
        {
            // 長い徒歩検証の時間を、次のメニュー遷移のタイムアウトへ持ち越しません。
            Elapsed = 0;
            if (FParse::Param(FCommandLine::Get(), TEXT("ThunderCoinTrailTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderBoulderTest")))
            { TrailRecoveryBeforeReset = Player->RecoveryCount; Tap(PC, EKeys::R); Step = 24; }
            else { Tap(PC, EKeys::M); Step = 4; }
        }
    }
    else if (Step == 12 && Player)
    {
        auto* PlatformTest = GetGameInstance()->GetSubsystem<UThunderPlatformTestSubsystem>();
        const bool bComplete = PlatformTest->UpdateTest(Player, DeltaSeconds);
        if (!PlatformTest->GetError().IsEmpty()) { Fail(PlatformTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 13 && Player)
    {
        auto* BalanceTest = GetGameInstance()->GetSubsystem<UThunderBalanceTestSubsystem>();
        const bool bComplete = BalanceTest->UpdateTest(Player, DeltaSeconds);
        if (!BalanceTest->GetError().IsEmpty()) { Fail(BalanceTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 14 && Player)
    {
        auto* GateTest = GetGameInstance()->GetSubsystem<UThunderGateTestSubsystem>();
        const bool bComplete = GateTest->UpdateTest(Player, DeltaSeconds);
        if (!GateTest->GetError().IsEmpty()) { Fail(GateTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 15 && Player)
    {
        auto* WarpTest = GetGameInstance()->GetSubsystem<UThunderWarpTestSubsystem>();
        const bool bComplete = WarpTest->UpdateTest(Player, DeltaSeconds);
        if (!WarpTest->GetError().IsEmpty()) { Fail(WarpTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 16 && Player)
    {
        auto* DamageTest = GetGameInstance()->GetSubsystem<UThunderDamageTestSubsystem>();
        const bool bComplete = DamageTest->UpdateTest(Player, DeltaSeconds);
        if (!DamageTest->GetError().IsEmpty()) { Fail(DamageTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 17 && Player)
    {
        auto* PoundTest = GetGameInstance()->GetSubsystem<UThunderPoundTestSubsystem>();
        const bool bComplete = PoundTest->UpdateTest(Player, DeltaSeconds);
        if (!PoundTest->GetError().IsEmpty()) { Fail(PoundTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 18 && Player)
    {
        auto* CarryTest = GetGameInstance()->GetSubsystem<UThunderCarryTestSubsystem>();
        const bool bComplete = CarryTest->UpdateTest(Player, DeltaSeconds);
        if (!CarryTest->GetError().IsEmpty()) { Fail(CarryTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 19 && Player)
    {
        auto* ClimbTest = GetGameInstance()->GetSubsystem<UThunderClimbTestSubsystem>();
        const bool bComplete = ClimbTest->UpdateTest(Player, DeltaSeconds);
        if (!ClimbTest->GetError().IsEmpty()) { Fail(ClimbTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 20 && Player)
    {
        auto* StompTest = GetGameInstance()->GetSubsystem<UThunderStompTestSubsystem>();
        const bool bComplete = StompTest->UpdateTest(Player, DeltaSeconds);
        if (!StompTest->GetError().IsEmpty()) { Fail(StompTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 21 && Player)
    {
        auto* CombatTest = GetGameInstance()->GetSubsystem<UThunderCombatTestSubsystem>();
        const bool bComplete = CombatTest->UpdateTest(Player, DeltaSeconds);
        if (!CombatTest->GetError().IsEmpty()) { Fail(CombatTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 22 && Player)
    {
        auto* CoinTest = GetGameInstance()->GetSubsystem<UThunderCoinTestSubsystem>();
        const bool bComplete = CoinTest->UpdateTest(Player, DeltaSeconds);
        if (!CoinTest->GetError().IsEmpty()) { Fail(CoinTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 23 && Player)
    {
        auto* BoxTest = GetGameInstance()->GetSubsystem<UThunderBoxTestSubsystem>();
        const bool bComplete = BoxTest->UpdateTest(Player, DeltaSeconds);
        if (!BoxTest->GetError().IsEmpty()) { Fail(BoxTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 43 && Player)
    {
        auto* HundredTest = GetGameInstance()->GetSubsystem<UThunderHundredTestSubsystem>();
        const bool bComplete = HundredTest->UpdateTest(Player, DeltaSeconds);
        if (!HundredTest->GetError().IsEmpty()) { Fail(HundredTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 42 && Player)
    {
        auto* PostTest = GetGameInstance()->GetSubsystem<UThunderPostTestSubsystem>();
        const bool bComplete = PostTest->UpdateTest(Player, DeltaSeconds);
        if (!PostTest->GetError().IsEmpty()) { Fail(PostTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 41 && Player)
    {
        auto* SentinelTest = GetGameInstance()->GetSubsystem<UThunderSentinelTestSubsystem>();
        const bool bComplete = SentinelTest->UpdateTest(Player, DeltaSeconds);
        if (!SentinelTest->GetError().IsEmpty()) { Fail(SentinelTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 40 && Player)
    {
        auto* ChimeTest = GetGameInstance()->GetSubsystem<UThunderChimeTestSubsystem>();
        const bool bComplete = ChimeTest->UpdateTest(Player, DeltaSeconds);
        if (!ChimeTest->GetError().IsEmpty()) { Fail(ChimeTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 39 && Player)
    {
        auto* ScarletTest = GetGameInstance()->GetSubsystem<UThunderScarletTestSubsystem>();
        const bool bComplete = ScarletTest->UpdateTest(Player, DeltaSeconds);
        if (!ScarletTest->GetError().IsEmpty()) { Fail(ScarletTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 38 && Player)
    {
        auto* SkyTest = GetGameInstance()->GetSubsystem<UThunderSkyCacheTestSubsystem>();
        const bool bComplete = SkyTest->UpdateTest(Player, DeltaSeconds);
        if (!SkyTest->GetError().IsEmpty()) { Fail(SkyTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 37 && Player)
    {
        auto* RaceTest = GetGameInstance()->GetSubsystem<UThunderRaceTestSubsystem>();
        const bool bComplete = RaceTest->UpdateTest(Player, DeltaSeconds);
        if (!RaceTest->GetError().IsEmpty()) { Fail(RaceTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 36 && Player)
    {
        auto* BossTest = GetGameInstance()->GetSubsystem<UThunderBossTestSubsystem>();
        const bool bComplete = BossTest->UpdateTest(Player, DeltaSeconds);
        if (!BossTest->GetError().IsEmpty()) { Fail(BossTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 35 && Player)
    {
        auto* TowerTest = GetGameInstance()->GetSubsystem<UThunderTowerTestSubsystem>();
        const bool bComplete = TowerTest->UpdateTest(Player, DeltaSeconds);
        if (!TowerTest->GetError().IsEmpty()) { Fail(TowerTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 34 && Player)
    {
        auto* FlightTest = GetGameInstance()->GetSubsystem<UThunderFlightTestSubsystem>();
        const bool bComplete = FlightTest->UpdateTest(Player, DeltaSeconds);
        if (!FlightTest->GetError().IsEmpty()) { Fail(FlightTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 33 && Player)
    {
        auto* LaunchTest = GetGameInstance()->GetSubsystem<UThunderLaunchTestSubsystem>();
        const bool bComplete = LaunchTest->UpdateTest(Player, DeltaSeconds);
        if (!LaunchTest->GetError().IsEmpty()) { Fail(LaunchTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 32 && Player)
    {
        auto* GuideTest = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>();
        const bool bComplete = GuideTest->UpdateTest(Player, DeltaSeconds);
        if (!GuideTest->GetError().IsEmpty()) { Fail(GuideTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 31 && Player)
    {
        auto* RideTest = GetGameInstance()->GetSubsystem<UThunderRideTestSubsystem>();
        const bool bComplete = RideTest->UpdateTest(Player, DeltaSeconds);
        if (!RideTest->GetError().IsEmpty()) { Fail(RideTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 30 && Player)
    {
        auto* GaleleafTest = GetGameInstance()->GetSubsystem<UThunderGaleleafTestSubsystem>();
        const bool bComplete = GaleleafTest->UpdateTest(Player, DeltaSeconds);
        if (!GaleleafTest->GetError().IsEmpty()) { Fail(GaleleafTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 29 && Player)
    {
        auto* BurstTest = GetGameInstance()->GetSubsystem<UThunderBurstTestSubsystem>();
        const bool bComplete = BurstTest->UpdateTest(Player, DeltaSeconds);
        if (!BurstTest->GetError().IsEmpty()) { Fail(BurstTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 28 && Player)
    {
        auto* ChaseTest = GetGameInstance()->GetSubsystem<UThunderChaseTestSubsystem>();
        const bool bComplete = ChaseTest->UpdateTest(Player, DeltaSeconds);
        if (!ChaseTest->GetError().IsEmpty()) { Fail(ChaseTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 27 && Player)
    {
        auto* WaterTest = GetGameInstance()->GetSubsystem<UThunderWaterTestSubsystem>();
        const bool bComplete = WaterTest->UpdateTest(Player, DeltaSeconds);
        if (!WaterTest->GetError().IsEmpty()) { Fail(WaterTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 25 && Player)
    {
        auto* BoulderTest = GetGameInstance()->GetSubsystem<UThunderBoulderTestSubsystem>();
        const bool bComplete = BoulderTest->UpdateTest(Player, DeltaSeconds);
        if (!BoulderTest->GetError().IsEmpty()) { Fail(BoulderTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::R); Step = 26; }
    }
    else if (Step == 26 && Player && Elapsed > .3f)
    {
        auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
        if (!Meadow) { Fail(TEXT("Coin trail regression stage missing")); return; }
        GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginCoinTrailRoute(Player, Meadow->GetMeadowCoinFootPath(), Meadow->GetMountainFootPath());
        Elapsed = 0;
        Step = 10;
    }
    else if (Step == 24 && Player && Elapsed > .5f)
    {
        const auto* MeadowMode = Cast<AThunderMeadowGameMode>(CurrentMode);
        const auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
        if (!MeadowMode || !Meadow || MeadowMode->GetCoinLedger()->GetTotal() != 0 || Player->RecoveryCount != TrailRecoveryBeforeReset + 1)
        { Fail(TEXT("R after the hundred-coin trail did not clear totals and restore start")); return; }
        for (const APlayseedCoin* Coin : Meadow->GetTrailCoins())
            if (!Coin || Coin->IsCollected()) { Fail(TEXT("R did not restore a placed trail coin")); return; }
        UE_LOG(LogTemp, Display, TEXT("COIN TRAIL RESET TEST PASS: R restored all 100 trail coins and total zero."));
        Elapsed = 0;
        Tap(PC, EKeys::M);
        Step = 4;
    }
    else if (Step == 4 && Map == TEXT("StageSelect") && Elapsed > 3)
    {
        if (bCampaignRun)
        {
            TArray<int32> Expected = ThunderCampaignPrefix(CampaignTarget); Expected.Add(CampaignTarget); if (CampaignTarget == 4) Expected.Add(7);
            if (!ThunderHasExactRewards(Expedition, Expected)) { Fail(TEXT("Campaign leg finished without exactly its actual cumulative reward set")); return; }
            UE_LOG(LogTemp, Display, TEXT("CAMPAIGN LEG COMPLETE: actual target%d saved total%d captureframe%d in the same slot %s."), CampaignTarget, Expedition->GetEarnedRewardCount(), Recording->GetCapturedFrameCount(), *Expedition->GetSaveSlotName());
            if (CampaignIndex < 5) { ++CampaignIndex; bSecondMissionEntry = false; Tap(PC, EKeys::Two); Step = 1; return; }
        }
        bSecondMissionEntry = true;
        Tap(PC, EKeys::Two);
        Step = 1;
    }
    else if (Step == 8 && Map == TEXT("StageSelect") && Elapsed > 3)
    {
        Tap(PC, EKeys::One);
        Step = 5;
    }
    else if (Step == 5 && Map == TEXT("WindseedGarden") && Player && Elapsed > 1)
    {
        Step = 6;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST PASS: selected mission, R, new World entry with mission 2, invalid numbers and Garden entry passed."));
    }
}

void UPlayseedSelectionTestSubsystem::Fail(const FString& Message)
{
    Step = -1;
    auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    const int32 Frames = Recording->FinishRecording();
    const FString Report = FString::Printf(TEXT("FAIL\n%s\nCaptureFrames: %d\n"), *Message, Frames);
    FFileHelper::SaveStringToFile(Report, *(FPaths::ProjectSavedDir() / TEXT("Validation/Result.txt")));
    UE_LOG(LogTemp, Error, TEXT("SELECTION TEST FAIL: %s"), *Message);
    FPlatformMisc::RequestExitWithStatus(false, 1);
}
