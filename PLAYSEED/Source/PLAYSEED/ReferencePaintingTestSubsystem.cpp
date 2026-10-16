#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceEntranceGameMode.h"
#include "ReferenceTravelSubsystem.h"
#include "ThunderMissionSelectionGameMode.h"
#include "ReferenceHero.h"
#include "ReferenceChainWalk.h"
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
    bFadeTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderFadeTest"));
    bCameraTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderCameraTest"));
    bSaveFailureTest=bCameraTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderSaveFailureTest"));
    bSelectionTest=FParse::Param(FCommandLine::Get(),TEXT("ThunderSelectionTest"));
    bStarTest=bSelectionTest||bSaveFailureTest||bFadeTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderStarTest"));
    bReadMode=FParse::Param(FCommandLine::Get(),TEXT("ThunderStarReadTest"));
    bChainTest=bStarTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderChainTest"));
    bDropTest=bChainTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderDropTest"));
    bEnabled=bReadMode||bDropTest||FParse::Param(FCommandLine::Get(),TEXT("ThunderPaintingTest"));
#endif
}
void UReferencePaintingTestSubsystem::Tap(APlayerController* PC,FKey Key)
{PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Pressed,1));PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Released,0));}
void UReferencePaintingTestSubsystem::Finish(bool bPassed,const FString& Message)
{
    if(bSelectionTest&&bPassed&&(!bSelectionInitialVerified||!bSelectionEarnedVerified))bPassed=false;
    if(bCameraTest&&bPassed&&(!bSawCameraHidden||!bSawCameraRestored))bPassed=false;
    if(bSaveFailureTest&&bPassed&&(!bSaveObstacleCreated||!bSaveFailureObserved||!bSaveObstacleRemoved))bPassed=false;
    if(bFadeTest&&bPassed&&(ObservedFadeOut.Num()!=7||ObservedFadeCovered.Num()!=7||ObservedFadeIn.Num()!=7||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetCompletedFades()!=7)){bPassed=false;UE_LOG(LogTemp,Error,TEXT("FADE QA incomplete sequences: out%d covered%d in%d"),ObservedFadeOut.Num(),ObservedFadeCovered.Num(),ObservedFadeIn.Num());}
    bFinished=true;auto* Recording=GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();const int32 Frames=Recording->FinishRecording();
    bPassed=bPassed&&!Recording->HasCaptureError()&&Frames>=150;
    const FString Details=Message+(bSaveFailureTest&&bPassed?TEXT(" Actual SaveGameToSlot failure from own empty QA directory, rollback0 and visible star, physical exit, nonrecursive removal of own empty obstacle, real recontact saved6 once and returned to painting."):TEXT(""));
    const FString Result=FString::Printf(TEXT("%s\n%s\nCaptureFrames: %d\nUTC: %s\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Details,Frames,*FDateTime::UtcNow().ToIso8601());
    FFileHelper::SaveStringToFile(Result,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp,Display,TEXT("PAINTING TEST %s"),*Result);FPlatformMisc::RequestExit(false);
}
void UReferencePaintingTestSubsystem::Tick(float DeltaSeconds)
{
    if(!GetWorld()||!GetWorld()->IsGameWorld())return;
    auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);if(!PC)return;Elapsed+=DeltaSeconds;
    if(Elapsed>(bChainTest?420:35)){Finish(false,TEXT("Painting or reference challenge timed out"));return;}
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
    if(bChainTest&&Phase>=20){UpdateChain(DeltaSeconds,Hero,PC);return;}
    if(Phase==0)
    {
        if(Map!=TEXT("Entrance")||!Hero||!Mode||Elapsed<1)return;
        if(!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_"))||Progress->GetEarnedRewardCount()!=(bReadMode?1:0)||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)!=bReadMode||(bReadMode&&!Progress->HasEarnedReward(6)))
        {Finish(false,TEXT("Painting QA requires a fresh isolated save"));return;}
        if(!GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>()->StartRecording(TEXT("Validation/CoursePlaythrough.avi"))){Finish(false,TEXT("Painting recording failed"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingRoom.png"),false,false);
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
            if(Map!=TEXT("MissionSelect")||Progress->GetEarnedRewardCount()!=1||!Progress->HasEarnedReward(6)||!UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
            {Finish(false,TEXT("Earned star6 did not persist through painting entry"));return;}
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
        if(Map!=TEXT("MissionSelect")||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
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
            Tap(PC,EKeys::One);Phase=6;Elapsed=0;return;
        }
        if(Elapsed>10)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingSelection.png"),false,false);
            Phase=4;Elapsed=0;
        }
        return;
    }
    if(Phase==4&&Elapsed>1){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=5;Elapsed=0;return;}
    if(Phase==5&&Elapsed>1)Finish(!bStarTest||(bMReentryChecked&&bPoseObserved&&bPoseShot),bReadMode?(bStarTest?TEXT("Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted."):TEXT("Separate process loaded the actual star6 save, walked and jumped into the painting, and displayed earned6/count1 in selection without awarding progress.")):TEXT("Normal walking and Space jump into painting, ripple/dive/fade to star selection; R/M/Space/Ctrl/Enter spam blocked during transition; fresh progress remains zero and unsaved."));
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
        ++VerifiedDropVisits;Phase=7;Elapsed=0;return;
    }
    if(Phase==7&&Elapsed>1)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;Phase=8;Elapsed=0;return;
    }
    if(Phase==8)
    {
        if(!Hero||Map!=TEXT("ThunderMeadow")||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
        {Finish(false,TEXT("Drop sequence changed map or progress unexpectedly"));return;}
        if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
        if(Hero->GetCharacterMovement()->IsFalling()&&Hero->GetActorLocation().Z>210)bSawJump=true;
        if(Elapsed>4)
        {
            if(!bSawJump||!Hero->GetCharacterMovement()->IsMovingOnGround()){Finish(false,TEXT("Ordinary jump was not available after drop landing"));return;}
            if(bStarTest&&VerifiedDropVisits==1){Tap(PC,EKeys::M);Phase=13;Elapsed=0;return;}
            if(bStarTest&&VerifiedDropVisits>=2)bMReentryChecked=true;
            if(bChainTest){Phase=20;Elapsed=0;ChainWaypoint=0;ChainLegElapsed=0;return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=9;Elapsed=0;
        }
        return;
    }
    if(Phase==9&&Elapsed>1){Tap(PC,EKeys::M);Phase=10;Elapsed=0;return;}
    if(Phase==10&&Elapsed>1)
    {
        if(Flow->IsFading())return;
        if(Map!=TEXT("Entrance")||!Hero||Flow->GetState()!=EReferenceTravelState::AtPainting||FVector::Dist2D(Hero->GetActorLocation(),FVector(-600,0,0))>40||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
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
            if(Flow->GetState()!=EReferenceTravelState::InCourse||Progress->GetEarnedRewardCount()!=0||Progress->HasEarnedReward(6)||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)||!ObservedStar.IsValid()||ObservedStar->IsCollected()||ObservedStar->IsHidden()||ObservedStar->GetSuccessfulCollections()!=0||!IFileManager::Get().DirectoryExists(*SaveObstaclePath))
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
    if(!Hero||Progress->GetEarnedRewardCount()!=1||!Progress->HasEarnedReward(6)||!UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
    {Finish(false,TEXT("Real star6 was not saved before the capture animation"));return;}
    if(Phase==26)
    {
        if(Flow->IsCelebrating())
        {
            if(!ObservedStar.IsValid()||ObservedStar->GetSuccessfulCollections()!=1||!ObservedStar->IsCollected())
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
    if(!Hero||Hero->RecoveryCount!=0||!ObservedStar.IsValid()||ObservedStar->IsCollected()||ObservedStar->IsHidden()||ObservedStar->GetSuccessfulCollections()!=0||Flow->GetState()!=EReferenceTravelState::InCourse||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)||!bSaveFailureObserved||!bSaveObstacleCreated||bSaveObstacleRemoved)
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
