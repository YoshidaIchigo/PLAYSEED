#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceEntranceGameMode.h"
#include "ReferenceTravelSubsystem.h"
#include "ThunderMissionSelectionGameMode.h"
#include "ReferenceHero.h"
#include "ReferenceChainWalk.h"
#include "ReferenceSummitWalk.h"
#include "ReferenceRaceRoute.h"
#include "ReferenceCannonWalk.h"
#include "ReferenceRedWalk.h"
#include "ReferenceHundredWalk.h"
#include "ReferenceCheckerPlatform.h"
#include "ThunderTiltingBridge.h"
#include "ThunderScarletTrail.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedClimbTree.h"
#include "PlayseedCoin.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "ThunderGuide.h"
#include "ThunderLauncher.h"
#include "ThunderSkyCache.h"
#include "PlayseedLaunchComponent.h"
#include "ThunderCloudflagRace.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderStormkeeper.h"
#include "PlayseedCarryComponent.h"
#include "ThunderMeadowStage.h"
#include "ThunderBoundSentinel.h"
#include "ThunderBindingPost.h"
#include "ThunderPrisonGate.h"
#include "PlayseedActionComponent.h"
#include "PlayseedLantern.h"
#include "PlayseedVitalityComponent.h"
#include "Camera/CameraActor.h"
#include "PlayseedRecordingSubsystem.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void UReferencePaintingTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
    bRainTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceRainTest"));
    bBoulderTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceBoulderTest"));
    bHundredTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceHundredTest"));
    bSequentialChainTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderSequentialChainTest"));
    bAllStarReadTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderAllStarReadTest"));
    bGroundCoinTest=bHundredTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceGroundTest"));
    bDefaultStartTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderDefaultStartTest"));
    bFadeTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderFadeTest"));
    bCameraTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderCameraTest"));
    bSaveFailureTest=bCameraTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderSaveFailureTest"));
    bSelectionTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderSelectionTest"));
    bReferenceBossTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceBossTest"));
    bReferenceRaceTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceRaceTest"));
    bReferenceCannonTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceCannonTest"));
    bReferenceRedTest=bHundredTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceRedTest"));
    bWingOwnedReplay=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceSkyOwnedTest"));
    bReferenceSkyTest=bHundredTest||bWingOwnedReplay||FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceSkyTest"));
    ExpectedReward=(bHundredTest||bAllStarReadTest)?7:bReferenceSkyTest?5:bReferenceRedTest?4:bReferenceBossTest?1:bReferenceRaceTest?2:bReferenceCannonTest?3:6;
    BeforeRewardCount=(bHundredTest||bAllStarReadTest)?6:bSequentialChainTest?5:bReferenceSkyTest?4:bReferenceRedTest?3:bReferenceCannonTest?2:bReferenceRaceTest?1:0;AfterRewardCount=BeforeRewardCount+1;
    // 獲得済みの実セーブを消さずに再プレイし、同じスターで数が増えないことも検証します。
    if(bWingOwnedReplay)BeforeRewardCount=AfterRewardCount=5;
    if(bBoulderTest||bRainTest)BeforeRewardCount=AfterRewardCount=7;
    bStarTest=bSequentialChainTest||bReferenceSkyTest||bReferenceRedTest||bReferenceCannonTest||bReferenceRaceTest||bReferenceBossTest||bSelectionTest||bSaveFailureTest||bFadeTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderStarTest"));
    bReadMode=bAllStarReadTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderStarReadTest"));
    bChainTest=bStarTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderChainTest"));
    bDropTest=bRainTest||bBoulderTest||bGroundCoinTest||bDefaultStartTest||bChainTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderDropTest"));
    bEnabled=bReadMode||bDropTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderPaintingTest"));
#endif
}
void UReferencePaintingTestSubsystem::Tap(APlayerController* PC,FKey Key)
{PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Pressed,1));PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Released,0));}
void UReferencePaintingTestSubsystem::Finish(bool bPassed,const FString& Message)
{
    if(bRainTest&&bPassed&&(!bRainContactVerified||!bRainDodgeVerified||!bRainResetVerified||!bRainBounceShot))bPassed=false;
    if(bBoulderTest&&bPassed&&(!bBoulderPitVerified||!bBoulderRoadVerified||!bBoulderResetVerified))bPassed=false;
    if(bGroundCoinTest&&bPassed&&(!bGroundCoinsVerified||(!bHundredTest&&!bGroundResetVerified)))bPassed=false;
    if(bReferenceSkyTest&&bPassed&&(!bWingHubChecked||!bWingPowerObserved||!bWingFlightObserved||!bWingCameraHidden)){bPassed=false;UE_LOG(LogTemp,Error,TEXT("WING QA missing actual hub/cap/flight/camera proof"));}
    if(bReferenceRedTest&&bPassed&&(!bRedPlatformRidden||!bRedTreeClimbed||!bRedBridgeRidden||!bCannonSawFlight||!bCannonAimHidden))bPassed=false;
    if(bReferenceCannonTest&&bPassed&&(!bCannonAimHidden||!bCannonSawFlight))bPassed=false;
    if(bReferenceRaceTest&&bPassed&&!bRaceSprintSeen)bPassed=false;
    if(bSelectionTest&&bPassed&&(!bSelectionInitialVerified||!bSelectionEarnedVerified))bPassed=false;
    if(bCameraTest&&bPassed&&(!bSawCameraHidden||!bSawCameraRestored))bPassed=false;
    if(bSaveFailureTest&&bPassed&&(!bSaveObstacleCreated||!bSaveFailureObserved||!bSaveObstacleRemoved))bPassed=false;
    if(bFadeTest&&bPassed&&(ObservedFadeOut.Num()!=7||ObservedFadeCovered.Num()!=7||ObservedFadeIn.Num()!=7||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetCompletedFades()!=7)){bPassed=false;UE_LOG(LogTemp,Error,TEXT("FADE QA incomplete sequences: out%d covered%d in%d"),ObservedFadeOut.Num(),ObservedFadeCovered.Num(),ObservedFadeIn.Num());}
    if(bHundredTest&&bPassed&&(!bHundredMountainGold||!bHundredHighGold||!bHundredSkyComplete))bPassed=false;
    bFinished=true;auto* Recording=GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();const int32 Frames=Recording->FinishRecording();
    bPassed=bPassed&&!Recording->HasCaptureError()&&Frames>=150;
    const FString Details=(bRainTest&&bPassed?TEXT("Three reference rain spawners at original positions/ranges: ordinary walking into a real falling drop caused one damage and physical knockback, normal R restored health and cleared all three, ordinary movement left the next real shadow before the actual burst with no extra hit, second R cleared all rain and M returned to painting with actual earned7 unchanged. No injected position, velocity, health or progress."):bBoulderTest&&bPassed?TEXT("Reference metal balls: two source mountain paths and three actual-floor pit loops; normal walking into a moving pit ball and actual mountain ball caused real damage/knockback, R restored hero and all five mechanisms twice, M returned to painting with legitimate earned1..7 unchanged. No injected hero position, velocity, health or progress."):bAllStarReadTest&&bPassed?TEXT("Separate process loaded the same actual save with earned stars1..7, walked/jumped into the painting, and displayed all six main stars earned with total7, without awarding progress."):bSequentialChainTest&&bPassed?TEXT("Actual sequential star6 visit after legitimate earned1..5: ordinary post3 ground pounds, released Chomp breaks gate, real star6 new save total6, pose/white fades/painting return and earned1..6 menu."):bHundredTest&&bPassed?TEXT("Actual hundred-star visit after legitimate earned1..6: real36 field golds, all eight reds, both mountain gold lines, island cap and ordinary second cannon launch, actual sky golds reaching100, one threshold100 star7, actual new save total7, pose/white fades/painting return and all main stars earned. No injected coins, position, velocity, power or progress."):bGroundCoinTest&&bPassed?TEXT("Actual36 field golds: normal wooden box punch and three pickups, source ground line5 and ring8, four posts with actual3circles under200/30 seconds and five pickups each, one generation per post. Real R restored score0/static coins/posts/box and removed only own drops, then M returned to painting. No progress awarded or injected. Mountain/island10 golds are placed but their actual pickup follows in the hundred-star visit."):bWingOwnedReplay&&bPassed?TEXT("Owned star5 full replay: legitimate saved wing unlock, ordinary cap-box head bump and actual pickup, normal G/mouse/LMB cannon, mouse/W/S flight through five actual centre coins, one fixed blue owned star5, NEW0 and earned count unchanged5, visible pose, white fades, painting return and next6 menu. No save copy/reset/injection."):Message)+(bSaveFailureTest&&bPassed?TEXT(" Actual SaveGameToSlot failure from own empty QA directory, rollback0 and visible star, physical exit, nonrecursive removal of own empty obstacle, real recontact saved6 once and returned to painting."):TEXT(""));
    const FString Result=FString::Printf(TEXT("%s\n%s\nCaptureFrames: %d\nUTC: %s\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Details,Frames,*FDateTime::UtcNow().ToIso8601());
    FFileHelper::SaveStringToFile(Result,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp,Display,TEXT("PAINTING TEST %s"),*Result);FPlatformMisc::RequestExit(false);
}
void UReferencePaintingTestSubsystem::Tick(float DeltaSeconds)
{
    if(!GetWorld()||!GetWorld()->IsGameWorld())return;
    auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);if(!PC)return;Elapsed+=DeltaSeconds;
    if(Elapsed>((bRainTest||bBoulderTest)?900:bHundredTest?1800:bGroundCoinTest?900:bChainTest?420:35)){Finish(false,TEXT("Painting or reference challenge timed out"));return;}
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    const FString Map=UGameplayStatics::GetCurrentLevelName(GetWorld(),true);
    auto* Hero=Cast<AReferenceHero>(PC->GetPawn());auto* Mode=Cast<AReferenceEntranceGameMode>(GetWorld()->GetAuthGameMode());
    if(bCameraTest&&Hero&&bSaveFailureObserved&&Flow->GetState()==EReferenceTravelState::InCourse)
    {
        if(Hero->IsBodyHiddenForCamera()&&!bSawCameraHidden)
        {bSawCameraHidden=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCameraClose.png"),false,false);}
        if(bSawCameraHidden&&!Hero->IsBodyHiddenForCamera()&&!bSawCameraRestored)
        {bSawCameraRestored=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCameraRestored.png"),false,false);}
    }
    if(bFadeTest&&Phase>0)ObserveFade(PC);
    if(bFinished)return;
    if(bRainTest&&Phase>=140&&Phase<=147){UpdateReferenceRain(DeltaSeconds,Hero,PC);return;}
    if(Phase>=130&&Phase<=137){UpdateReferenceBoulders(DeltaSeconds,Hero,PC);return;}
    if(bHundredTest&&Phase>=120){UpdateReferenceHundred(DeltaSeconds,Hero,PC);return;}
    if(bHundredTest&&(Phase==98||Phase==99)){UpdateHundredFlight(DeltaSeconds,Hero,PC);return;}
    if(bGroundCoinTest&&Phase>=110){UpdateReferenceGround(DeltaSeconds,Hero,PC);return;}
    if(bReferenceSkyTest&&Phase>=90){UpdateReferenceSky(DeltaSeconds,Hero,PC);return;}
    if(bReferenceRedTest&&Phase>=70){UpdateReferenceRed(DeltaSeconds,Hero,PC);return;}
    if((bReferenceCannonTest||bReferenceRedTest)&&Phase>=60){UpdateReferenceCannon(DeltaSeconds,Hero,PC);return;}
    if(bReferenceRaceTest&&Phase>=50){UpdateReferenceRace(DeltaSeconds,Hero,PC);return;}
    if(bReferenceBossTest&&Phase>=40){UpdateReferenceBoss(DeltaSeconds,Hero,PC);return;}
    if(bChainTest&&Phase>=20){UpdateChain(DeltaSeconds,Hero,PC);return;}
    if(Phase==0)
    {
        if(Map!=TEXT("Entrance")||!Hero||!Mode||Elapsed<1)return;
        if(bDefaultStartTest&&(!AReferenceBattlefieldTerrain::IsReferenceCourse()||FParse::Param(FCommandLine::Get(),TEXT("ReferenceCourse"))))
        {Finish(false,TEXT("Default startup still depends on the preview flag"));return;}
        if(!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_"))||Progress->GetEarnedRewardCount()!=(bReadMode?AfterRewardCount:BeforeRewardCount)||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=(bReadMode||BeforeRewardCount>0)||(bReadMode&&!Progress->HasEarnedReward(ExpectedReward)))
        {Finish(false,TEXT("Painting QA requires a fresh isolated save"));return;}
        if(bHundredTest)for(int32 Number=1;Number<=6;++Number)if(!Progress->HasEarnedReward(Number)){Finish(false,TEXT("Hundred QA requires the actual previously earned stars1..6"));return;}
        if(bReferenceSkyTest&&(!Progress->HasEarnedReward(1)||!Progress->HasEarnedReward(2)||!Progress->HasEarnedReward(3)||!Progress->HasEarnedReward(4)||!Progress->IsLauncherUnlocked()||(bWingOwnedReplay&&!Progress->HasEarnedReward(5)))){Finish(false,TEXT("Wing QA requires actual earned stars1/2/3/4 and legitimate cannon access, plus actual5 for owned replay"));return;}
        if(bReferenceRedTest&&(!Progress->HasEarnedReward(1)||!Progress->HasEarnedReward(2)||!Progress->HasEarnedReward(3)||!Progress->IsLauncherUnlocked())){Finish(false,TEXT("Red QA requires actual earned stars1/2/3 and legitimate cannon access"));return;}
        if(bReferenceCannonTest&&(!Progress->HasEarnedReward(1)||!Progress->HasEarnedReward(2))){Finish(false,TEXT("Cannon QA requires actually earned star1/2"));return;}
        if(bReferenceRaceTest&&!Progress->HasEarnedReward(1)){Finish(false,TEXT("Race requires the previously actually earned star1 save"));return;}
        if(!GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>()->StartRecording(TEXT("Validation/CoursePlaythrough.avi"))){Finish(false,TEXT("Painting recording failed"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingRoom.png"),false,false);
        if(bReferenceSkyTest&&!bWingHubChecked){bWingHubInitiallyUnlocked=Progress->IsFlightUnlocked();Phase=90;Elapsed=0;return;}
        Phase=1;Elapsed=0;return;
    }
    if(Phase==1)
    {
        if(!Hero||!Mode){Finish(false,TEXT("Entrance player disappeared before entry"));return;}
        if(Mode->IsEntering()){Phase=2;Elapsed=0;return;}
        if(Hero->RecoveryCount!=0){Finish(false,TEXT("QA route recovered instead of walking and jumping"));return;}
        if(Flow->IsFading())return;
        Hero->AddMovementInput(FVector::ForwardVector,1);
        if(Hero->GetActorLocation().X>155&&!bJumped)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));bJumped=true;JumpHold=.14f;}
        if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
        return;
    }
    if(Phase==2)
    {
        // 製品が入力を止めている間に実際のキーを連打し、R/Mで演出を飛ばせないことを確かめます。
        if(Mode)
        {
            if(Flow->GetState()==EReferenceTravelState::EnteringPainting)
                for(FKey Key:{EKeys::R,EKeys::M,EKeys::SpaceBar,EKeys::LeftControl,EKeys::Enter})Tap(PC,Key);
            const bool bValidState=Flow->GetState()==EReferenceTravelState::EnteringPainting||Flow->GetState()==EReferenceTravelState::SelectingStar;
            if(!Mode->IsEntering()||!bValidState||!Hero||Hero->RecoveryCount!=0)
            {Finish(false,FString::Printf(TEXT("Input spam interrupted painting entry: state%d recovery%d"),static_cast<int32>(Flow->GetState()),Hero?Hero->RecoveryCount:-1));return;}
            if(Elapsed>.3f&&!bCapturedDive){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingDive.png"),false,false);bCapturedDive=true;}
            return;
        }
        if(Map!=TEXT("MissionSelect")||Flow->GetState()!=EReferenceTravelState::SelectingStar){Finish(false,TEXT("Painting did not arrive at star selection"));return;}
        Phase=3;Elapsed=0;return;
    }
    if(Phase==3)
    {
        if(bReadMode)
        {
            if(Map!=TEXT("MissionSelect")||Progress->GetEarnedRewardCount()!=AfterRewardCount||!Progress->HasEarnedReward(ExpectedReward)||!UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
            {Finish(false,TEXT("Earned star6 did not persist through painting entry"));return;}
            if(bAllStarReadTest||bHundredTest)
            {
                for(int32 Number=1;Number<=7;++Number)if(!Progress->HasEarnedReward(Number)){Finish(false,TEXT("Actual seven-star save did not persist every earned ID"));return;}
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=6){Finish(false,TEXT("Final total7 selection did not show all six main stars"));return;}
                for(int32 Number=1;Number<=6;++Number)if(!Menu->IsMissionSelectable(Number)){Finish(false,TEXT("An earned main star became unavailable after total7"));return;}
            }
            if(bReferenceSkyTest&&!bHundredTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=6||!Menu->IsMissionSelectable(5)||!Menu->IsMissionSelectable(6))
                {Finish(false,TEXT("Actual star5 did not reveal exactly the next mission6"));return;}
            }
            if(bReferenceRedTest&&!bHundredTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=5||!Menu->IsMissionSelectable(4)||!Menu->IsMissionSelectable(5)||Menu->IsMissionSelectable(6))
                {Finish(false,TEXT("Actual star4 did not reveal exactly the next mission5"));return;}
            }
            if(bReferenceCannonTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=4||!Menu->IsMissionSelectable(1)||!Menu->IsMissionSelectable(2)||!Menu->IsMissionSelectable(3)||!Menu->IsMissionSelectable(4)||Menu->IsMissionSelectable(5))
                {Finish(false,TEXT("Actual stars1/2/3 did not reveal exactly the next mission4"));return;}
            }
            if(bReferenceRaceTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=3||!Menu->IsMissionSelectable(1)||!Menu->IsMissionSelectable(2)||!Menu->IsMissionSelectable(3)||Menu->IsMissionSelectable(4))
                {Finish(false,TEXT("Earned1/2 did not reveal exactly the next mission3"));return;}
            }
            if(bReferenceBossTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=2||!Menu->IsMissionSelectable(1)||!Menu->IsMissionSelectable(2)||Menu->IsMissionSelectable(3))
                {Finish(false,TEXT("Earned1 did not reveal exactly the next mission2"));return;}
            }
            if(bSelectionTest)
            {
                auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
                if(!Menu||Menu->GetVisibleMissionCount()!=6||!Menu->IsMissionSelectable(1)||!Menu->IsMissionSelectable(6)||Menu->IsMissionSelectable(2))
                {Finish(false,TEXT("Earned6-first UI did not show6 and allow only earned6/next1"));return;}
                bSelectionEarnedVerified=true;
            }
            if(Elapsed>10){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderStarSelection.png"),false,false);Phase=4;Elapsed=0;}
            return;
        }
        if(Map!=TEXT("MissionSelect")||Progress->GetEarnedRewardCount()!=BeforeRewardCount||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=(BeforeRewardCount>0))
        {Finish(false,TEXT("Painting entry changed progress or travelled twice"));return;}
        if(bSelectionTest&&Elapsed>1.1f)
        {
            auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());
            if(!Menu||Menu->GetVisibleMissionCount()!=1||Menu->GetHighlightedMission()!=1||!Menu->IsMissionSelectable(1)||Menu->IsMissionSelectable(2)||Progress->GetSelectedMissionNumber()!=1)
            {Finish(false,TEXT("Fresh selection UI did not keep only mission1 after invalid number/navigation inputs"));return;}
            if(Elapsed<1.8f)for(FKey Key:{EKeys::Two,EKeys::Right,EKeys::Left})Tap(PC,Key);
            if(Elapsed>1.7f)bSelectionInitialVerified=true;
        }
        if(bDropTest&&Elapsed>2)
        {
            if(bSelectionTest)
            {
                if(!bSelectionInitialShot){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderSelectionInitial.png"),false,false);bSelectionInitialShot=true;}
            }
            else FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingSelection.png"),false,false);
            Tap(PC,(bRainTest||bBoulderTest||bHundredTest||bSequentialChainTest)?EKeys::Six:bReferenceSkyTest?EKeys::Five:bReferenceRedTest?EKeys::Four:bReferenceCannonTest?EKeys::Three:bReferenceRaceTest?EKeys::Two:EKeys::One);Phase=6;Elapsed=0;return;
        }
        if(Elapsed>10)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingSelection.png"),false,false);
            Phase=4;Elapsed=0;
        }
        return;
    }
    if(Phase==4&&Elapsed>1){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=5;Elapsed=0;return;}
    if(Phase==5&&Elapsed>1)Finish(!bStarTest||(bMReentryChecked&&bPoseObserved&&bPoseShot),bReadMode?(bReferenceSkyTest?TEXT("Actual entrance-switch ground pound and saved wing access, ordinary cap-box head bump and actual pickup, normal field cannon G/mouse/LMB, real controlled flight through five actual centre coins, fixed star5 save total5, pose/fades/painting return and earned1/2/3/4/5 next6 menu."):bReferenceRedTest?TEXT("Actual eight reference red coins through seesaw bridge, ordinary moving-platform ride and jump, normal G/mouse/LMB cannon, F/W tree climb, one fixed-position star4, real save total4, pose/fades/painting return and earned1/2/3/4 next5 menu."):bReferenceCannonTest?TEXT("Loaded actually earned stars1/2, normal guide dialogue and saved cannon access, ordinary mountain walking, normal G aim/mouse controls/LMB physical2600 launch and island landing, actual upward jump head-bump box once, real star3 save total3, pose/fades/painting return and earned1/2/3 next4 menu."):bReferenceRaceTest?TEXT("Loaded previously actually earned star1, normal G race start, real sprint/walk/jump summit arrival before rival, waited for rival, G win dialogue, actual fixed-position star2 save, pose/fades/painting return and earned1/2 next3 menu."):bReferenceBossTest?TEXT("Actual M return and painting reentry, ordinary mountain route, three E rear pickups and real physical summit throws, actual fixed-position star1 save, pose/fades/painting return and earned1 menu persistence."):bStarTest?TEXT("Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted."):TEXT("Separate process loaded the actual star6 save, walked and jumped into the painting, and displayed earned6/count1 in selection without awarding progress.")):TEXT("Normal walking and Space jump into painting, ripple/dive/fade to star selection; R/M/Space/Ctrl/Enter spam blocked during transition; fresh progress remains zero and unsaved."));
    if(Phase==6)
    {
        if(Map!=TEXT("ThunderMeadow")||!Hero)return;
        if(Hero->RecoveryCount!=0){Finish(false,TEXT("R interrupted the physical entry drop"));return;}
        auto* Movement=Hero->GetCharacterMovement();
        if(Flow->GetState()==EReferenceTravelState::Dropping)
        {
            if(!bSawDrop)
            {
                if(!Movement->IsFalling()||Hero->GetActorLocation().Z<900){Finish(false,TEXT("Course entry did not start physically falling from above"));return;}
                bSawDrop=true;UE_LOG(LogTemp,Display,TEXT("DROP QA physical falling begins Z %.1f"),Hero->GetActorLocation().Z);
            }
            for(FKey Key:{EKeys::R,EKeys::M,EKeys::SpaceBar,EKeys::LeftControl,EKeys::Enter})Tap(PC,Key);
            if(!Movement->IsMovingOnGround()&&Elapsed>.5f)FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCourseDrop.png"),false,false);
            if(Movement->IsMovingOnGround()&&!bCapturedLanding)
            {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCourseLanding.png"),false,false);bCapturedLanding=true;}
            return;
        }
        if(Flow->GetState()!=EReferenceTravelState::InCourse||!bSawDrop||!bCapturedLanding||!Movement->IsMovingOnGround()||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
        {Finish(false,TEXT("Drop did not land at the reference entrance and release input once"));return;}
        if(bDefaultStartTest||bHundredTest)
        {
            if(!Cast<AReferenceHero>(Hero)||!UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()))
            {Finish(false,TEXT("Default entry did not create the current hero and reference terrain"));return;}
            Tap(PC,EKeys::H);
        }
        ++VerifiedDropVisits;Phase=7;Elapsed=0;return;
    }
    if(Phase==7&&Elapsed>1)
    {
        if(bDefaultStartTest||bHundredTest)Tap(PC,EKeys::H);
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;Phase=8;Elapsed=0;return;
    }
    if(Phase==8)
    {
        if(!Hero||Map!=TEXT("ThunderMeadow")||Progress->GetEarnedRewardCount()!=BeforeRewardCount||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=(BeforeRewardCount>0))
        {Finish(false,TEXT("Drop sequence changed map or progress unexpectedly"));return;}
        if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
        if(Hero->GetCharacterMovement()->IsFalling()&&Hero->GetActorLocation().Z>210)bSawJump=true;
        if(Elapsed>4)
        {
            if(!bSawJump||!Hero->GetCharacterMovement()->IsMovingOnGround()){Finish(false,TEXT("Ordinary jump was not available after drop landing"));return;}
            if(bStarTest&&VerifiedDropVisits==1){Tap(PC,EKeys::M);Phase=13;Elapsed=0;return;}
            if(bStarTest&&VerifiedDropVisits>=2)bMReentryChecked=true;
            if(bRainTest){Phase=140;Elapsed=0;RainWaypoint=0;RainLegElapsed=0;return;}
            if(bBoulderTest){Phase=130;Elapsed=0;BoulderWaypoint=0;BoulderLegElapsed=0;return;}
            if(bGroundCoinTest){Phase=110;Elapsed=0;GroundWaypoint=0;GroundLegElapsed=0;return;}
            if(bReferenceSkyTest){Phase=93;Elapsed=0;WingWaypoint=0;WingLegElapsed=0;return;}
            if(bReferenceRedTest){Phase=70;Elapsed=0;RedWaypoint=0;RedLegElapsed=0;return;}
            if(bReferenceCannonTest){Phase=60;Elapsed=0;CannonWaypoint=0;CannonLegElapsed=0;return;}
            if(bReferenceRaceTest){Phase=50;Elapsed=0;RaceWaypoint=0;RaceLegElapsed=0;return;}
            if(bReferenceBossTest){Phase=40;Elapsed=0;BossWaypoint=0;BossLegElapsed=0;return;}
            if(bChainTest){Phase=20;Elapsed=0;ChainWaypoint=0;ChainLegElapsed=0;return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=9;Elapsed=0;
        }
        return;
    }
    if(Phase==9&&Elapsed>1){Tap(PC,EKeys::M);Phase=10;Elapsed=0;return;}
    if(Phase==10&&Elapsed>1)
    {
        if(Flow->IsFading())return;
        if(Map!=TEXT("Entrance")||!Hero||Flow->GetState()!=EReferenceTravelState::AtPainting||FVector::Dist2D(Hero->GetActorLocation(),FVector(-600,0,0))>40||Progress->GetEarnedRewardCount()!=BeforeRewardCount||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=(BeforeRewardCount>0))
        {Finish(false,TEXT("M did not return to the painting front with unchanged progress"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingReturn.png"),false,false);Phase=11;Elapsed=0;return;
    }
    if(Phase==11&&Elapsed>1){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=12;Elapsed=0;return;}
    if(Phase==12&&Elapsed>1)Finish(!bChainTest||bChainGateVerified,bChainTest?TEXT("Painting entry, physical drop, ordinary walking to reference post, three real ground pounds, released Chomp physical sweep breaks gate once, visible original-position star6, stop before collection and M returns to painting. No recovery, no injected state, zero progress unchanged."):TEXT("Painting dive -> actual star selection -> physical 1000cm drop -> ground contact and landing pose -> ordinary Space jump -> M returns to painting front. R/M/Space/Ctrl/Enter spam blocked during transition, no recovery and fresh zero progress unchanged."));
    if(Phase==13&&Elapsed>1)
    {
        if(Flow->IsFading())return;
        if(Map!=TEXT("Entrance")||!Hero||Flow->GetState()!=EReferenceTravelState::AtPainting||FVector::Dist2D(Hero->GetActorLocation(),FVector(-600,0,0))>40)
        {Finish(false,TEXT("M return did not reach a usable painting entrance"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingReentry.png"),false,false);
        bJumped=false;bCapturedDive=false;bSawDrop=false;bCapturedLanding=false;bSawJump=false;JumpHold=0;Phase=1;Elapsed=0;
    }
}

void UReferencePaintingTestSubsystem::UpdateChain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    if(bSaveFailureTest&&Phase==28){UpdateSaveRetry(DeltaSeconds,Hero,PC);return;}
    if(bStarTest&&Phase>=26){UpdateStarCapture(DeltaSeconds,Hero,PC);return;}
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    if(!Hero||!Stage||!Stage->GetBindingPost()||!Stage->GetPrisonGate()||!Stage->GetBoundSentinel()||Hero->RecoveryCount!=0)
    {Finish(false,TEXT("Reference challenge disappeared or the route died/recovered"));return;}
    auto* Post=Stage->GetBindingPost();auto* Gate=Stage->GetPrisonGate();auto* Chomp=Stage->GetBoundSentinel();auto* Movement=Hero->GetCharacterMovement();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    if(Phase==20||Phase==23)
    {
        const auto* Points=Phase==20?ReferenceChainApproach:ReferenceChainGateApproach;
        const int32 Count=Phase==20?UE_ARRAY_COUNT(ReferenceChainApproach):UE_ARRAY_COUNT(ReferenceChainGateApproach);
        if(ChainWaypoint>=Count)
        {
            if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
            if(Phase==20)
            {
                PC->SetControlRotation(FRotator(-12,(Chomp->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderChainBefore.png"),false,false);
                Phase=21;Elapsed=0;return;
            }
            if(!Gate->IsBroken()||Gate->GetBreakCount()!=1||!Gate->GetReward()||!Chomp->IsDeparted())
            {Finish(false,TEXT("Reference gate did not break once from the released Chomp"));return;}
            bChainGateVerified=true;
            PC->SetControlRotation(FRotator(10,(Gate->GetRewardLocation()-Hero->GetActorLocation()).Rotation().Yaw+35,0));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis,IE_Axis,2));
            Phase=24;Elapsed=0;return;
        }
        const auto& Point=Points[ChainWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();ChainLegElapsed+=DeltaSeconds;
        if(ChainLegElapsed>18){Finish(false,FString::Printf(TEXT("Chain ordinary walking blocked at %d: hero%s target%s"),ChainWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {++ChainWaypoint;ChainLegElapsed=0;return;}
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Movement->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        if(Movement->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        return;
    }
    if(Phase==21)
    {
        if(Post->GetPoundCount()>=3)
        {
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));
            if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderChainRelease.png"),false,false);Phase=22;Elapsed=0;return;
        }
        if(Post->GetPoundCount()>ChainPounds)
        {ChainPounds=Post->GetPoundCount();bChainPoundCycle=false;bChainPoundTriggered=false;ChainPoundElapsed=0;}
        const FVector Delta=Post->GetActorLocation()-Hero->GetActorLocation();
        if(Delta.Size2D()>8)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/100,.15f,1.f));
        else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
        if(!bChainPoundCycle&&Movement->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bChainPoundCycle=true;ChainPoundElapsed=0;}
        if(bChainPoundCycle)
        {
            ChainPoundElapsed+=DeltaSeconds;
            if(!bChainPoundTriggered&&Movement->IsFalling()&&Delta.Size2D()<50&&Hero->GetActorLocation().Z-82>Post->GetActorLocation().Z+Post->GetTopHeight()+35)
            {Tap(PC,EKeys::LeftControl);bChainPoundTriggered=true;}
            if(ChainPoundElapsed>5){Finish(false,FString::Printf(TEXT("Normal jump/pound did not hit reference post: pounds%d hero%s"),Post->GetPoundCount(),*Hero->GetActorLocation().ToString()));return;}
        }
        return;
    }
    if(Phase==22&&Gate->IsBroken()&&Chomp->IsDeparted())
    {Phase=23;Elapsed=0;ChainWaypoint=0;ChainLegElapsed=0;}
    if(Phase==24&&Elapsed>1.2f)
    {
        if(bSaveFailureTest&&!PrepareSaveFailure()){Finish(false,TEXT("Could not safely create a new empty obstacle for own QA save"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderChainGate.png"),false,false);Phase=bStarTest?25:9;Elapsed=0;
    }
    if(Phase==25)
    {
        auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
        auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
        if(bSaveFailureTest&&!bSaveFailureObserved&&!Progress->GetProgressProblem().IsEmpty())
        {
            ObservedStar=Gate->GetReward();
            if(Flow->GetState()!=EReferenceTravelState::InCourse||Progress->GetEarnedRewardCount()!=0||Progress->HasEarnedReward(ExpectedReward)||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)||!ObservedStar.IsValid()||ObservedStar->IsCollected()||ObservedStar->IsHidden()||ObservedStar->GetSuccessfulCollections()!=0||!IFileManager::Get().DirectoryExists(*SaveObstaclePath))
            {Finish(false,TEXT("Actual failed save did not keep progress0, visible uncollected star and ordinary play"));return;}
            bSaveFailureObserved=true;Phase=28;Elapsed=0;RetryOutsideElapsed=0;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderSaveFailure.png"),false,false);
            UE_LOG(LogTemp,Display,TEXT("SAVE RETRY QA: real star contact failed SaveGameToSlot; rollback0, visible star, no celebration."));return;
        }
        if(Flow->IsCelebrating())
        {ObservedStar=Gate->GetReward();Phase=26;Elapsed=0;return;}
        const FVector Delta=Gate->GetRewardLocation()-Hero->GetActorLocation();
        if(Delta.Size2D()>15)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
        else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
        if(Delta.Size2D()<220&&Movement->IsMovingOnGround()&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        if(Elapsed>12)Finish(false,TEXT("Ordinary walking and jump did not touch the real star6"));
    }
}

void UReferencePaintingTestSubsystem::UpdateStarCapture(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const FString Map=UGameplayStatics::GetCurrentLevelName(GetWorld(),true);
    if(Flow->IsFading()&&!Flow->IsCelebrating())return;
    if(bSaveFailureTest&&(!bSaveFailureObserved||!bSaveObstacleRemoved||!Progress->GetProgressProblem().IsEmpty()||IFileManager::Get().DirectoryExists(*SaveObstaclePath)))
    {Finish(false,TEXT("Save retry did not clear the error or remove only the own obstacle"));return;}
    if(!Hero||Progress->GetEarnedRewardCount()!=AfterRewardCount||!Progress->HasEarnedReward(ExpectedReward)||!UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
    {Finish(false,TEXT("Real star6 was not saved before the capture animation"));return;}
    if(Phase==26)
    {
        if(Flow->IsCelebrating())
        {
            if(!ObservedStar.IsValid()||ObservedStar->GetSuccessfulCollections()!=(bWingOwnedReplay?0:1)||!ObservedStar->IsCollected())
            {Finish(false,TEXT("Star collection duplicated or real actor was not collected"));return;}
            for(FKey Key:{EKeys::R,EKeys::M,EKeys::SpaceBar,EKeys::LeftControl,EKeys::Enter,EKeys::LeftMouseButton})Tap(PC,Key);
            if(Flow->HasStartedStarPose())
            {
                if(!bPoseObserved){bPoseObserved=true;PoseLocation=Hero->GetActorLocation();PoseHealth=Hero->GetVitality()->GetHealth();}
                auto* Model=Cast<AReferenceHero>(Hero);
                if(!Model||!Model->IsShowingStarPose()||FVector::Dist(Hero->GetActorLocation(),PoseLocation)>1||Hero->GetVitality()->GetHealth()!=PoseHealth)
                {Finish(false,TEXT("Pose, position or vitality changed during locked capture"));return;}
                if(Elapsed>1.6f&&!bPoseShot)
                {
                    if(bCameraTest&&Model->IsBodyHiddenForCamera()){Finish(false,TEXT("Star pose stayed hidden after the camera moved out"));return;}
                    if(!Cast<ACameraActor>(PC->GetViewTarget())){Finish(false,TEXT("Star camera did not become the actual view target"));return;}
                    if(bReferenceRaceTest||bReferenceCannonTest||bReferenceRedTest||bReferenceSkyTest)
                    {
                        FCollisionQueryParams Query(SCENE_QUERY_STAT(RaceStarPoseVisibility),true,Hero);FHitResult Obstacle;
                        if(GetWorld()->LineTraceSingleByChannel(Obstacle,PC->GetViewTarget()->GetActorLocation(),Hero->GetActorLocation()+FVector(0,0,60),ECC_Camera,Query))
                        {Finish(false,TEXT("Race rival or terrain blocked the real star-pose camera"));return;}
                    }
                    FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderStarPose.png"),false,false);bPoseShot=true;
                }
            }
            return;
        }
        if(Map!=TEXT("Entrance")||Flow->GetState()!=EReferenceTravelState::AtPainting||!bPoseObserved||!bPoseShot||FVector::Dist2D(Hero->GetActorLocation(),FVector(-600,0,0))>40)
        {Finish(false,TEXT("Star capture did not automatically return to the painting front"));return;}
        Phase=27;Elapsed=0;return;
    }
    if(Phase==27&&Elapsed>1.2f)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderStarReturn.png"),false,false);
        bReadMode=true;bJumped=false;bCapturedDive=false;JumpHold=0;Phase=1;Elapsed=0;
    }
}

bool UReferencePaintingTestSubsystem::PrepareSaveFailure()
{
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();const FString Slot=Progress->GetSaveSlotName();
    if(!bSaveFailureTest||bSaveObstacleCreated||!Slot.StartsWith(TEXT("Validation_"))||Progress->GetEarnedRewardCount()!=0||!Progress->GetProgressProblem().IsEmpty()||UGameplayStatics::DoesSaveGameExist(Slot,0))return false;
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames"));
    SaveObstaclePath=FPaths::ConvertRelativePathToFull(Directory/(Slot+TEXT(".sav")));
    // QA専用の未使用スロットだけに空のフォルダを作ります。既存のファイル・権限・セーブ内容は触りません。
    if(SaveObstaclePath!=Directory/(Slot+TEXT(".sav"))||!SaveObstaclePath.StartsWith(Directory+TEXT("/"))||IFileManager::Get().FileExists(*SaveObstaclePath)||IFileManager::Get().DirectoryExists(*SaveObstaclePath))return false;
    bSaveObstacleCreated=IFileManager::Get().MakeDirectory(*SaveObstaclePath,false);
    if(bSaveObstacleCreated)UE_LOG(LogTemp,Display,TEXT("SAVE RETRY QA: created own fresh empty Validation_ .sav directory; no save data injected."));
    return bSaveObstacleCreated;
}
void UReferencePaintingTestSubsystem::UpdateSaveRetry(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(!Hero||Hero->RecoveryCount!=0||!ObservedStar.IsValid()||ObservedStar->IsCollected()||ObservedStar->IsHidden()||ObservedStar->GetSuccessfulCollections()!=0||Flow->GetState()!=EReferenceTravelState::InCourse||Progress->GetEarnedRewardCount()!=BeforeRewardCount||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=(BeforeRewardCount>0)||!bSaveFailureObserved||!bSaveObstacleCreated||bSaveObstacleRemoved)
    {Finish(false,TEXT("Failed-save state changed before physical exit and retry"));return;}
    auto* Movement=Hero->GetCharacterMovement();
    if(Movement->IsMovingOnGround()&&FVector::Dist2D(Hero->GetActorLocation(),ObservedStar->GetActorLocation())>250)
    {
        Movement->StopMovementImmediately();RetryOutsideElapsed+=DeltaSeconds;
        if(RetryOutsideElapsed<1.25f)return;
        const FString Expected=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames")/(Progress->GetSaveSlotName()+TEXT(".sav")));
        TArray<FString> Entries;IFileManager::Get().FindFiles(Entries,*(SaveObstaclePath/TEXT("*")),true,true);
        // 自分で作った同一の空フォルダだけを、再帰なしで消します。中身があれば必ず中止します。
        if(SaveObstaclePath!=Expected||!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_"))||Entries.Num()!=0||!IFileManager::Get().DirectoryExists(*SaveObstaclePath)||!IFileManager::Get().DeleteDirectory(*SaveObstaclePath,true,false))
        {Finish(false,TEXT("Own save obstacle was not safely removable as an empty nonrecursive directory"));return;}
        bSaveObstacleRemoved=true;Phase=25;Elapsed=0;
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderSaveRetryReady.png"),false,false);
        UE_LOG(LogTemp,Display,TEXT("SAVE RETRY QA: actual physical exit and cooldown; removed only own empty directory, now walking/jumping to real star again."));return;
    }
    RetryOutsideElapsed=0;const FVector Delta=FVector(1300,-700,768)-Hero->GetActorLocation();
    PC->SetControlRotation(FRotator(-12,Delta.Rotation().Yaw,0));Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
    if(Elapsed>12)Finish(false,TEXT("Ordinary movement did not leave the failed-save star contact volume"));
}

void UReferencePaintingTestSubsystem::ObserveFade(APlayerController* PC)
{
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();if(!Flow->IsFading())return;
    const int32 Sequence=Flow->GetFadeSequence();const auto Fade=Flow->GetFadePhase();const float Opacity=Flow->GetTransitionOpacity();
    if(!Flow->IsTransitionLocked()||Opacity<0||Opacity>1){Finish(false,TEXT("Fade did not lock transition or opacity was invalid"));return;}
    if(Fade==EReferenceFadePhase::Out)ObservedFadeOut.Add(Sequence);
    if(Fade==EReferenceFadePhase::Covered)
    {
        if(Opacity!=1){Finish(false,TEXT("World change was not completely covered by white"));return;}
        ObservedFadeCovered.Add(Sequence);
    }
    if(Fade==EReferenceFadePhase::In)ObservedFadeIn.Add(Sequence);
    // 実際の入力経路へ連打します。保存・座標・体力を直接変える検証ではありません。
    for(FKey Key:{EKeys::R,EKeys::M,EKeys::SpaceBar,EKeys::LeftControl,EKeys::Enter,EKeys::One,EKeys::Two,EKeys::Left,EKeys::Right})Tap(PC,Key);
    if(auto* Menu=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode()))
    {
        const int32 Current=Menu->GetHighlightedMission();
        if(const int32* Expected=FadeMenuSelection.Find(Sequence)){if(*Expected!=Current){Finish(false,TEXT("Input spam changed selection during fade"));return;}}
        else FadeMenuSelection.Add(Sequence,Current);
    }
    const int32 ShotKey=Sequence*10+static_cast<int32>(Fade);
    if(FadeShots.Contains(ShotKey)||!((Fade==EReferenceFadePhase::Covered)||(Opacity>.4f&&Opacity<.6f)))return;
    const TCHAR* Name=nullptr;
    if(Sequence==1)Name=Fade==EReferenceFadePhase::Out?TEXT("ThunderFadeEntryOut.png"):Fade==EReferenceFadePhase::Covered?TEXT("ThunderFadeEntryWhite.png"):TEXT("ThunderFadeSelectionIn.png");
    if(Sequence==2)Name=Fade==EReferenceFadePhase::Out?TEXT("ThunderFadeSelectionOut.png"):Fade==EReferenceFadePhase::Covered?TEXT("ThunderFadeDropWhite.png"):TEXT("ThunderFadeDropIn.png");
    if(Sequence==6)Name=Fade==EReferenceFadePhase::Out?TEXT("ThunderFadeReturnOut.png"):Fade==EReferenceFadePhase::Covered?TEXT("ThunderFadeReturnWhite.png"):TEXT("ThunderFadeEntranceIn.png");
    if(Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation")/Name,false,false);FadeShots.Add(ShotKey);}
}

void UReferencePaintingTestSubsystem::UpdateReferenceBoss(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Boss=Stage?Stage->GetStormkeeper():nullptr;
    if(!Hero||!Boss||Hero->RecoveryCount!=0||Boss->GetOffArenaReturns()!=0)
    {Finish(false,TEXT("Reference boss/player disappeared, route recovered or a throw left summit"));return;}
    auto* Movement=Hero->GetCharacterMovement();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    if(Phase==40)
    {
        if(BossWaypoint>=UE_ARRAY_COUNT(ReferenceSummitWalk))
        {
            if(!Movement->IsMovingOnGround())return;
            Movement->StopMovementImmediately();FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderBossSummit.png"),false,false);
            Phase=41;Elapsed=0;return;
        }
        const auto& Point=ReferenceSummitWalk[BossWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();BossLegElapsed+=DeltaSeconds;
        if(BossLegElapsed>18){Finish(false,FString::Printf(TEXT("Reference boss walk blocked %d hero%s target%s"),BossWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {UE_LOG(LogTemp,Display,TEXT("BOSS WALK %d"),BossWaypoint);++BossWaypoint;BossLegElapsed=0;bBossRouteJump=false;return;}
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Movement->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        if(Point.bJump&&!bBossRouteJump&&Movement->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bBossRouteJump=true;}
        if(Movement->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        return;
    }
    PC->SetControlRotation(FRotator(-18,(Boss->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
    if(Phase==41)
    {
        if(Elapsed>30){Finish(false,FString::Printf(TEXT("Rear E pickup timed out: hero%s boss%s valid%d"),*Hero->GetActorLocation().ToString(),*Boss->GetActorLocation().ToString(),Boss->GetValidThrows()));return;}
        if(Boss->IsRecovering()||Boss->IsInThrownFlight())return;
        if(!bBossCoaxed)
        {
            // 外周で一周せず、通常歩行で内側へ誘ってから背後へ回ります。
            if(Boss->GetActorLocation().X>=2000){bBossCoaxed=true;Elapsed=0;}
            else
            {
                const FVector Delta=FVector(2400,5200,4293)-Hero->GetActorLocation();
                if(Delta.Size2D()>18)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
                else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
                return;
            }
        }
        if(Boss->IsHeldBy(Hero))
        {
            if(Hero->GetCarry()->GetHeldObject()!=Boss){Finish(false,TEXT("Boss bypassed normal carry component"));return;}
            BossCarryStart=Hero->GetActorLocation();Phase=42;Elapsed=0;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderBossLift.png"),false,false);return;
        }
        const FVector Radial=(Hero->GetActorLocation()-Boss->GetActorLocation()).GetSafeNormal2D();
        const float Distance=FVector::Dist2D(Hero->GetActorLocation(),Boss->GetActorLocation());
        const float Rear=FVector::DotProduct(Radial,Boss->GetActorForwardVector());
        if(Boss->CanBePickedUp(Hero)&&Rear<-.55f&&Movement->IsMovingOnGround())
        {Movement->StopMovementImmediately();Tap(PC,EKeys::E);return;}
        const FVector Tangent(-Radial.Y,Radial.X,0);
        FVector Desired=Tangent*430+Radial*FMath::Clamp((295-Distance)*4,-470.f,470.f);
        auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
        FVector Floor;
        if(!Terrain||!Terrain->FindFloor(Hero->GetActorLocation()+Desired.GetSafeNormal2D()*180-FVector(0,0,82),Floor)||FMath::Abs(Floor.Z-4293)>25)
            Desired=-Tangent*430+Radial*FMath::Clamp((295-Distance)*4,-470.f,470.f);
        Hero->AddMovementInput(Desired.GetSafeNormal2D(),FMath::Clamp(Desired.Size2D()/470,.2f,1.f));return;
    }
    if(Phase==42)
    {
        if(!Boss->IsHeldBy(Hero)){Finish(false,TEXT("Normal E pickup did not stay held"));return;}
        // 頭上に持ったまま通常の歩行で山の内側へ向きます。プレイヤーの回転を注入しません。
        const FVector Direction=(FVector(3800,5000,4293)-Hero->GetActorLocation()).GetSafeNormal2D();
        Hero->AddMovementInput(Direction,1);
        if(FVector::Dist2D(Hero->GetActorLocation(),BossCarryStart)>90&&FVector::DotProduct(Hero->GetActorForwardVector(),Direction)>.98f)
        {
            Movement->StopMovementImmediately();BossThrows=Boss->GetValidThrows();bBossThrowObserved=false;Tap(PC,EKeys::E);
            Phase=43;Elapsed=0;return;
        }
        if(Elapsed>4)Finish(false,TEXT("Ordinary carrying walk did not face summit interior"));return;
    }
    if(Phase==43)
    {
        // InputKeyは通常の次フレームで処理されます。送信直後の古い保持状態で失敗させません。
        if(!bBossThrowObserved&&Boss->GetThrowCount()==BossThrows+1&&!Boss->IsHeld()&&!Hero->GetCarry()->GetHeldObject()
            &&(Boss->IsInThrownFlight()||Boss->GetThrownLandingCount()>BossThrows))
        {
            bBossThrowObserved=true;
            UE_LOG(LogTemp,Display,TEXT("BOSS QA: normal E physical throw %d observed"),Boss->GetThrowCount());
        }
        if(Elapsed>.5f&&!bBossThrowObserved){Finish(false,TEXT("Second E did not use the normal physical throw after input processing"));return;}
        if(Boss->GetValidThrows()>BossThrows)
        {
            if(!bBossThrowObserved||Boss->GetPickupCount()!=BossThrows+1||Boss->GetThrowCount()!=BossThrows+1||Boss->GetThrownLandingCount()!=BossThrows+1)
            {Finish(false,TEXT("Physical throw counts did not match real E inputs"));return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderBossThrow.png"),false,false);
            if(Boss->IsDefeated())
            {
                // スターは見た目だけ±8cm浮くため、固定の配置は基準位置で検査します。
                if(Boss->GetValidThrows()!=3||!Boss->GetReward()||Boss->GetReward()->GetRewardNumber()!=1||FVector::Dist(Boss->GetReward()->GetRestLocation(),FVector(2000,4500,4500))>1)
                {Finish(false,TEXT("Three physical throws did not spawn star1 at reference position"));return;}
                Phase=44;Elapsed=0;
            }
            else {Phase=41;Elapsed=0;}
            return;
        }
        if(Elapsed>8)Finish(false,TEXT("Real thrown boss did not land on summit"));return;
    }
    if(Phase==44)
    {
        auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
        if(Flow->IsCelebrating()){ObservedStar=Boss->GetReward();Phase=26;Elapsed=0;return;}
        const FVector Delta=FVector(2000,4500,4500)-Hero->GetActorLocation();
        PC->SetControlRotation(FRotator(-14,Delta.Rotation().Yaw,0));
        if(Delta.Size2D()>15)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
        else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
        if(Delta.Size2D()<220&&Movement->IsMovingOnGround()&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        if(Elapsed>20)Finish(false,TEXT("Ordinary walk/jump did not touch actual star1"));
    }
}

void UReferencePaintingTestSubsystem::UpdateReferenceRace(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Race=Stage?Stage->GetCloudflagRace():nullptr;
    if(!Hero||!Race||Hero->RecoveryCount!=0){Finish(false,TEXT("Reference race disappeared or player recovered"));return;}
    if(Race->GetRaceState()==ECloudflagRaceState::Lost||Race->GetRaceState()==ECloudflagRaceState::Disqualified)
    {Finish(false,FString::Printf(TEXT("Actual race was lost/disqualified: player%.2f rival%.2f"),Race->GetPlayerTime(),Race->GetRivalTime()));return;}
    auto* Movement=Hero->GetCharacterMovement();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    if(Phase==50||Phase==52)
    {
        const auto* Points=Phase==50?ReferenceRaceApproach:ReferenceRaceToSummit;
        const int32 Count=Phase==50?UE_ARRAY_COUNT(ReferenceRaceApproach):UE_ARRAY_COUNT(ReferenceRaceToSummit);
        if(Phase==52&&Movement->MaxWalkSpeed>600)bRaceSprintSeen=true;
        if(RaceWaypoint>=Count)
        {
            if(!Movement->IsMovingOnGround())return;
            Movement->StopMovementImmediately();
            if(Phase==50)
            {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRaceStart.png"),false,false);Phase=51;}
            else
            {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Released,0));FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRaceFlag.png"),false,false);Phase=53;}
            Elapsed=0;return;
        }
        const auto& Point=Points[RaceWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();RaceLegElapsed+=DeltaSeconds;
        if(RaceLegElapsed>18){Finish(false,FString::Printf(TEXT("Reference race walk blocked phase%d point%d hero%s target%s"),Phase,RaceWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {UE_LOG(LogTemp,Display,TEXT("RACE WALK phase%d point%d"),Phase,RaceWaypoint);++RaceWaypoint;RaceLegElapsed=0;bRaceRouteJump=false;return;}
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Movement->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        if(Point.bJump&&!bRaceRouteJump&&Movement->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bRaceRouteJump=true;}
        if(Movement->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        return;
    }
    if(Phase==51)
    {
        PC->SetControlRotation(FRotator(-12,(Race->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        if(!bRaceStartSent&&Elapsed>.5f){Tap(PC,EKeys::G);bRaceStartSent=true;}
        if(Race->GetRaceState()==ECloudflagRaceState::Running)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Pressed,1));Phase=52;Elapsed=0;RaceWaypoint=0;RaceLegElapsed=0;return;}
        if(Elapsed>3)Finish(false,TEXT("Normal G dialogue did not start the reference race"));return;
    }
    if(Phase==53)
    {
        if(Elapsed>1&&!Race->HasPlayerReachedFlag()){Finish(false,TEXT("Ordinary summit route did not actually reach the race flag"));return;}
        PC->SetControlRotation(FRotator(-15,180,0));
        if(Race->GetRaceState()==ECloudflagRaceState::Won)
        {
            if(!Race->HasRivalFinished()||Race->GetPlayerTime()<0||Race->GetPlayerTime()>=Race->GetRivalTime())
            {Finish(false,TEXT("Race declared won without both actual arrival times"));return;}
            Phase=54;Elapsed=0;return;
        }
        if(Elapsed>300)Finish(false,TEXT("Rival did not finish the real reference route"));return;
    }
    if(Phase==54)
    {
        PC->SetControlRotation(FRotator(-12,(Race->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        if(Race->GetReward())
        {
            if(Race->GetReward()->GetRewardNumber()!=2||FVector::Dist(Race->GetReward()->GetRestLocation(),FVector(3030,4600,4500))>1)
            {Finish(false,TEXT("Win dialogue did not spawn star2 at the fixed reference position"));return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRaceWin.png"),false,false);Phase=55;Elapsed=0;return;
        }
        const FVector Delta=Race->GetActorLocation()+FVector(-170,0,82)-Hero->GetActorLocation();
        if(Delta.Size2D()>18)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
        else if(Movement->IsMovingOnGround())
        {Movement->StopMovementImmediately();if(!bRaceClaimSent){Tap(PC,EKeys::G);bRaceClaimSent=true;}}
        if(Elapsed>12)Finish(false,TEXT("Normal walk and G after the rival finished did not produce star2"));return;
    }
    if(Phase==55)
    {
        auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
        if(Flow->IsCelebrating()){ObservedStar=Race->GetReward();Phase=26;Elapsed=0;return;}
        const FVector Delta=Race->GetReward()->GetRestLocation()-Hero->GetActorLocation();
        PC->SetControlRotation(FRotator(-14,Delta.Rotation().Yaw,0));
        if(Delta.Size2D()>15)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
        else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
        if(Delta.Size2D()<220&&Movement->IsMovingOnGround()&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        if(Elapsed>15)Finish(false,TEXT("Ordinary walk/jump did not touch actual star2"));
    }
}

void UReferencePaintingTestSubsystem::UpdateReferenceCannon(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Cannon=Stage?Stage->GetLauncher():nullptr;auto* Guide=Stage?Stage->GetGuide():nullptr;auto* Box=Stage?Stage->GetSkyCache():nullptr;
    if(!Hero||!Cannon||!Guide||(!Box&&!bReferenceRedTest)||Hero->RecoveryCount!=0||Stage->GetReferenceLaunchers().Num()!=6)
    {Finish(false,TEXT("Reference cannon/guide/box disappeared or the ordinary route recovered"));return;}
    auto* Movement=Hero->GetCharacterMovement();auto* Launch=Hero->GetLaunch();
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    if(Phase==60||Phase==62||Phase==66)
    {
        const auto* Points=Phase==60?ReferenceGuideApproach:Phase==62?ReferenceCannonApproach:ReferenceIslandWalk;
        const int32 Count=Phase==60?UE_ARRAY_COUNT(ReferenceGuideApproach):Phase==62?UE_ARRAY_COUNT(ReferenceCannonApproach):UE_ARRAY_COUNT(ReferenceIslandWalk);
        if(CannonWaypoint>=Count)
        {
            if(!Movement->IsMovingOnGround())return;
            Movement->StopMovementImmediately();
            if(bHundredTest&&Phase==62&&!bHundredHighGold){Phase=122;HundredWaypoint=0;HundredLegElapsed=0;Elapsed=0;return;}
            if(Phase==60){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCannonGuide.png"),false,false);Phase=61;}
            else if(Phase==62){Phase=63;}
            else {Phase=67;}
            Elapsed=0;return;
        }
        const auto& Point=Points[CannonWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();CannonLegElapsed+=DeltaSeconds;
        if(CannonLegElapsed>18){Finish(false,FString::Printf(TEXT("Cannon ordinary walk blocked phase%d point%d hero%s target%s"),Phase,CannonWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {
            const bool bDetour=bHundredTest&&Phase==62&&CannonWaypoint==HundredMountainDetourAt&&!bHundredMountainGold;
            UE_LOG(LogTemp,Display,TEXT("CANNON WALK phase%d point%d"),Phase,CannonWaypoint);++CannonWaypoint;CannonLegElapsed=0;bCannonRouteJump=false;
            if(bDetour){Phase=121;HundredWaypoint=0;HundredLegElapsed=0;Elapsed=0;}return;
        }
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Movement->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        if(Point.bJump&&!bCannonRouteJump&&Movement->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bCannonRouteJump=true;}
        if(Movement->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        return;
    }
    if(Phase==61)
    {
        PC->SetControlRotation(FRotator(-12,(Guide->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        if(!bGuideSent&&Elapsed>.5f){Tap(PC,EKeys::G);bGuideSent=true;}
        if(bGuideSent&&Elapsed>1&&Progress->IsLauncherUnlocked()&&Cannon->IsOpen())
        {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCannonOpen.png"),false,false);Phase=62;Elapsed=0;CannonWaypoint=0;CannonLegElapsed=0;return;}
        if(Guide->IsAwaitingConfirmation()&&Elapsed>1.1f&&!bGuideConfirmed){Tap(PC,EKeys::G);bGuideConfirmed=true;}
        if(Elapsed>5)Finish(false,TEXT("Normal two-step G guide dialogue did not save/open the actual cannons"));return;
    }
    if(Phase==63)
    {
        PC->SetControlRotation(FRotator(-12,(Cannon->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        if(!bAimSent&&Elapsed>.4f){Tap(PC,EKeys::G);bAimSent=true;}
        if(Launch->IsAiming())
        {
            const FVector Origin=Cannon->GetSeatLocation(),Target(5000,-1250,3072+82),Delta=Target-Origin;
            const double D=Delta.Size2D(),A=FMath::Abs(Movement->GetGravityZ())*D*D/(2*2600.*2600.);
            const double Discriminant=D*D-4*A*(A+Delta.Z);
            if(Discriminant<=0){Finish(false,TEXT("Reference high cannon cannot physically reach the island"));return;}
            CannonGoalYaw=Delta.Rotation().Yaw;CannonGoalPitch=FMath::RadiansToDegrees(FMath::Atan((D-FMath::Sqrt(Discriminant))/(2*A)));
            UE_LOG(LogTemp,Display,TEXT("CANNON QA: normal input goal yaw%.3f pitch%.3f, actual seat%s"),CannonGoalYaw,CannonGoalPitch,*Origin.ToString());
            Phase=64;Elapsed=0;return;
        }
        if(Elapsed>4)Finish(false,TEXT("Normal G did not enter the open high mountain cannon"));return;
    }
    if(Phase==64)
    {
        if(!Launch->IsAiming()){Finish(false,TEXT("Cannon aim exited before normal LMB fire"));return;}
        const float YawError=FRotator::NormalizeAxis(CannonGoalYaw-Launch->GetAimYaw()),PitchError=CannonGoalPitch-Launch->GetAimPitch();
        // 通常のマウス軸へ少しずつ入力します。照準値やプレイヤー速度は直接設定しません。
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,FMath::Clamp(YawError*.4f,-20.f,20.f)));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,FMath::Clamp(-PitchError*.4f,-20.f,20.f)));
        if(FMath::Abs(YawError)<.08f&&FMath::Abs(PitchError)<.08f)CannonAimStable+=DeltaSeconds;else CannonAimStable=0;
        if(CannonAimStable>.3f&&!bCannonAimShot)
        {
            auto* Model=Cast<AReferenceHero>(Hero);
            if(!Model||!Model->IsBodyHiddenForCamera()){Finish(false,TEXT("The close cannon camera still rendered the hero body"));return;}
            bCannonAimHidden=true;bCannonAimShot=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCannonAim.png"),false,false);
        }
        if(CannonAimStable>1)
        {Tap(PC,EKeys::LeftMouseButton);Phase=65;Elapsed=0;return;}
        if(Elapsed>15)Finish(false,FString::Printf(TEXT("Actual mouse aiming did not settle: yaw%.2f/%.2f pitch%.2f/%.2f"),Launch->GetAimYaw(),CannonGoalYaw,Launch->GetAimPitch(),CannonGoalPitch));return;
    }
    if(Phase==65)
    {
        if(Launch->GetLaunchCount()==1&&!Launch->IsAiming()&&Movement->IsFalling())bCannonSawFlight=true;
        if(Elapsed>.4f&&Movement->IsMovingOnGround())
        {
            if(!bCannonSawFlight||Launch->GetLaunchCount()!=1||FMath::Abs(Hero->GetActorLocation().Z-82-3072)>40||FVector::Dist2D(Hero->GetActorLocation(),FVector(5000,-1250,3072))>450
                ||FVector::Dist(Launch->GetLastLaunchOrigin(),Cannon->GetSeatLocation())>2||FMath::Abs(Launch->GetLastLaunchVelocity().Size()-2600)>1)
            {Finish(false,FString::Printf(TEXT("Actual ballistic shot did not land on island: hero%s origin%s"),*Hero->GetActorLocation().ToString(),*Launch->GetLastLaunchOrigin().ToString()));return;}
            Movement->StopMovementImmediately();FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderCannonLanding.png"),false,false);
            Phase=bReferenceRedTest?75:66;Elapsed=0;RedWaypoint=0;RedLegElapsed=0;CannonWaypoint=0;CannonLegElapsed=0;return;
        }
        if(Elapsed>8)Finish(false,TEXT("Actual LMB physical shot did not land within8s"));return;
    }
    if(Phase==67)
    {
        if(Flow->IsCelebrating())
        {
            if(!Box->IsBroken()||Box->GetHeadBumpCount()!=1||Box->GetBreakCount()!=1||Box->GetRewardGenerationCount()!=1)
            {Finish(false,TEXT("Star3 did not follow one actual upward head bump and one reward generation"));return;}
            ObservedStar=Box->GetReward();Phase=26;Elapsed=0;return;
        }
        if(Box->IsBroken()&&(!Box->GetReward()||Box->GetReward()->GetRewardNumber()!=3))
        {Finish(false,TEXT("Physically broken island box did not produce the actual star3"));return;}
        const FVector Delta=Box->GetActorLocation()-Hero->GetActorLocation();
        PC->SetControlRotation(FRotator(-12,Delta.Rotation().Yaw,0));
        if(Delta.Size2D()>12)Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/120,.2f,1.f));
        else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
        if(Movement->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<50)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
        if(Elapsed>18)Finish(false,FString::Printf(TEXT("Normal upward jumps did not break/collect island box: broken%d bumps%d hero%s"),Box->IsBroken(),Box->GetHeadBumpCount(),*Hero->GetActorLocation().ToString()));
    }
}
