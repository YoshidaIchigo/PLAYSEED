#include "PlayseedSelectionTestSubsystem.h"
#include "PlayseedGameModeBase.h"
#include "PlayseedRecordingSubsystem.h"
#include "WindseedCharacter.h"
#include "WindseedStage.h"
#include "ThunderExpeditionSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderTraversalTestSubsystem.h"
#include "ThunderPlatformTestSubsystem.h"
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
    if (Elapsed > (Step == 10 || Step == 12 ? 120 : 20)) { Fail(TEXT("Stage selection timed out")); return; }
    auto* PC = UGameplayStatics::GetPlayerController(CurrentMode, 0);
    if (!PC) return;
    const FString Map = UGameplayStatics::GetCurrentLevelName(CurrentMode, true);
    auto* Player = Cast<AWindseedCharacter>(PC->GetPawn());
    // 起動中は Tick が先に進む場合があるため、実際に選択画面を録画できてから入力します。
    const auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (bProgressTest && (Expedition->GetEarnedRewardCount() != 2 || !Expedition->HasEarnedReward(1) || !Expedition->HasEarnedReward(7)))
    { Fail(TEXT("Saved rewards changed during restart or re-entry")); return; }
    if (Step == 0 && Map == TEXT("StageSelect") && Elapsed > 3 && Recording->GetCapturedFrameCount() > 15)
    {
        Tap(PC, EKeys::Two);
        Step = 1;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: pressed 2 to enter Thunder Meadow."));
    }
    else if (Step == 1 && Map == TEXT("MissionSelect") && Elapsed > 3)
    {
        Tap(PC, bSecondMissionEntry ? EKeys::Two : EKeys::Six);
        Step = 7;
    }
    else if (Step == 7 && Map == TEXT("ThunderMeadow") && Player && Elapsed > 2)
    {
        const int32 ExpectedMission = bSecondMissionEntry ? 2 : 6;
        if (Expedition->GetSelectedMissionNumber() != ExpectedMission || Expedition->SelectMission(0) || Expedition->SelectMission(7)
            || Expedition->GetSelectedMissionNumber() != ExpectedMission)
        { Fail(TEXT("Mission selection did not survive travel or accepted an invalid number")); return; }
        if (bSecondMissionEntry)
        {
            Tap(PC, EKeys::M);
            Step = 8;
            UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: re-entry with mission 2 passed after mission 6."));
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
            || Expedition->GetSelectedMissionNumber() != 6)
        { Fail(TEXT("Restart in the new stage failed")); return; }
        const bool bIslandTest = FParse::Param(FCommandLine::Get(), TEXT("ThunderIslandTest"));
        if (FParse::Param(FCommandLine::Get(), TEXT("ThunderMountainTest")) || bIslandTest)
        {
            auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(CurrentMode, AThunderMeadowStage::StaticClass()));
            if (!Meadow || Meadow->GetMountainFootPath().Num() < 25) { Fail(TEXT("The mountain route was not created")); return; }
            GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginMountainRoute(Player, Meadow->GetMountainFootPath(), bIslandTest);
            Step = 10;
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
            Tap(PC, EKeys::M);
            Step = 4;
        }
    }
    else if (Step == 12 && Player)
    {
        auto* PlatformTest = GetGameInstance()->GetSubsystem<UThunderPlatformTestSubsystem>();
        const bool bComplete = PlatformTest->UpdateTest(Player, DeltaSeconds);
        if (!PlatformTest->GetError().IsEmpty()) { Fail(PlatformTest->GetError()); return; }
        if (bComplete) { Elapsed = 0; Tap(PC, EKeys::M); Step = 4; }
    }
    else if (Step == 4 && Map == TEXT("StageSelect") && Elapsed > 3)
    {
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
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed."));
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
