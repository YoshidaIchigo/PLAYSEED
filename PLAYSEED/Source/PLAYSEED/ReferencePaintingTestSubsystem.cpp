#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceEntranceGameMode.h"
#include "ReferenceTravelSubsystem.h"
#include "ReferenceHero.h"
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
#include "UnrealClient.h"
void UReferencePaintingTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
    bEnabled=FParse::Param(FCommandLine::Get(),TEXT("ThunderPaintingTest"));
#endif
}
void UReferencePaintingTestSubsystem::Tap(APlayerController* PC,FKey Key)
{PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Pressed,1));PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Released,0));}
void UReferencePaintingTestSubsystem::Finish(bool bPassed,const FString& Message)
{
    bFinished=true;auto* Recording=GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();const int32 Frames=Recording->FinishRecording();
    bPassed=bPassed&&!Recording->HasCaptureError()&&Frames>=150;
    const FString Result=FString::Printf(TEXT("%s\n%s\nCaptureFrames: %d\nUTC: %s\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Message,Frames,*FDateTime::UtcNow().ToIso8601());
    FFileHelper::SaveStringToFile(Result,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp,Display,TEXT("PAINTING TEST %s"),*Result);FPlatformMisc::RequestExit(false);
}
void UReferencePaintingTestSubsystem::Tick(float DeltaSeconds)
{
    if(!GetWorld()||!GetWorld()->IsGameWorld())return;
    auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);if(!PC)return;Elapsed+=DeltaSeconds;
    if(Elapsed>35){Finish(false,TEXT("Painting entry timed out"));return;}
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    const FString Map=UGameplayStatics::GetCurrentLevelName(GetWorld(),true);
    auto* Hero=Cast<AReferenceHero>(PC->GetPawn());auto* Mode=Cast<AReferenceEntranceGameMode>(GetWorld()->GetAuthGameMode());
    if(Phase==0)
    {
        if(Map!=TEXT("Entrance")||!Hero||!Mode||Elapsed<1)return;
        if(!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_"))||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
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
            for(FKey Key:{EKeys::R,EKeys::M,EKeys::SpaceBar,EKeys::LeftControl,EKeys::Enter})Tap(PC,Key);
            if(!Mode->IsEntering()||Flow->GetState()!=EReferenceTravelState::EnteringPainting||Hero->RecoveryCount!=0)
            {Finish(false,TEXT("Input spam interrupted painting entry"));return;}
            if(Elapsed>.3f&&!bCapturedDive){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingDive.png"),false,false);bCapturedDive=true;}
            return;
        }
        if(Map!=TEXT("MissionSelect")||Flow->GetState()!=EReferenceTravelState::SelectingStar){Finish(false,TEXT("Painting did not arrive at star selection"));return;}
        Phase=3;Elapsed=0;return;
    }
    if(Phase==3)
    {
        if(Map!=TEXT("MissionSelect")||Progress->GetEarnedRewardCount()!=0||UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0))
        {Finish(false,TEXT("Painting entry changed progress or travelled twice"));return;}
        if(Elapsed>10)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPaintingSelection.png"),false,false);
            Phase=4;Elapsed=0;
        }
        return;
    }
    if(Phase==4&&Elapsed>1){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=5;Elapsed=0;return;}
    if(Phase==5&&Elapsed>1)Finish(true,TEXT("Normal walking and Space jump into painting, ripple/dive/fade to star selection; R/M/Space/Ctrl/Enter spam blocked during transition; fresh progress remains zero and unsaved."));
}
