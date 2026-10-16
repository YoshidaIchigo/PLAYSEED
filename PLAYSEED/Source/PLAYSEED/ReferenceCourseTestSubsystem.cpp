#include "ReferenceCourseTestSubsystem.h"
#include "ReferenceBattlefieldTerrain.h"
#include "PlayseedRecordingSubsystem.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferenceCourseTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
    bEnabled=FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceTest"));
#endif
}
void UReferenceCourseTestSubsystem::Tap(APlayerController* PC,FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Pressed,1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Released,0));
}
void UReferenceCourseTestSubsystem::Finish(bool bPassed,const FString& Message)
{
    bFinished=true;
    auto* Recording=GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    const int32 Frames=Recording->FinishRecording();
    bPassed=bPassed && !Recording->HasCaptureError() && Frames>=150;
    const FString Result=FString::Printf(TEXT("%s\n%s\nCaptureFrames: %d\nUTC: %s\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Message,Frames,*FDateTime::UtcNow().ToIso8601());
    FFileHelper::SaveStringToFile(Result,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp,Display,TEXT("REFERENCE TEST %s"),*Result);
    FPlatformMisc::RequestExit(false);
}
void UReferenceCourseTestSubsystem::Tick(float DeltaSeconds)
{
    if(!GetWorld() || !GetWorld()->IsGameWorld())return;
    auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);if(!PC)return;
    Elapsed+=DeltaSeconds;
    if(Elapsed>240){Finish(false,TEXT("Reference terrain route timed out"));return;}
    const FString Map=UGameplayStatics::GetCurrentLevelName(GetWorld(),true);
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(Phase==0)
    {
        if(Map!=TEXT("MissionSelect"))return;
        if(!bRecording)
        {
            if(!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || Progress->GetEarnedRewardCount()!=0 || UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)){Finish(false,TEXT("Terrain QA requires a fresh isolated save"));return;}
            bRecording=GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>()->StartRecording(TEXT("Validation/CoursePlaythrough.avi"));
            if(!bRecording){Finish(false,TEXT("Terrain recording unavailable"));return;}
        }
        if(Elapsed>2){Tap(PC,EKeys::One);Phase=1;Elapsed=0;}return;
    }
    auto* Player=Cast<AWindseedCharacter>(PC->GetPawn());
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
    if(Phase==1)
    {
        if(!Terrain || !Player || Elapsed<1)return;
        if(FVector::Dist2D(Player->GetActorLocation(),FVector(-6558,-6464,0))>30){Finish(false,TEXT("Reference spawn is not at the documented start"));return;}
        RecoveryBaseline=Player->RecoveryCount;
        Route={FVector(-6400,-6900,0),FVector(-5700,-6900,0),FVector(-5000,-6900,0),FVector(-4300,-6200,0),FVector(-4004,-5221,0),FVector(-3500,-5800,0),FVector(-2220,-5520,204),FVector(-2020,-5820,204),FVector(760,-5680,765),FVector(1920,-5040,768),FVector(2000,-4360,768),FVector(1240,-3600,768),FVector(-280,-2720,768),FVector(-1680,-1840,768),FVector(-2280,-1800,0),FVector(-2720,-1120,0),FVector(-2520,-480,0),FVector(-1720,-120,0),FVector(560,840,630),FVector(2700,1500,1571),FVector(4520,2600,1830),FVector(6240,3500,1943),FVector(6380,6180,2066),FVector(4000,7720,2390),FVector(-120,5515,2608),FVector(-180,4860,2641),FVector(580,3380,2769),FVector(1670,2580,2867),FVector(2216,2126,2900),FVector(3167,1870,2965),FVector(4069,2487,3050),FVector(4880,3416,3037),FVector(5020,5760,3181),FVector(4660,6300,3354),FVector(3940,6800,3514),FVector(3200,6850,3619),FVector(1290,5793,3768),FVector(1150,3670,3900),FVector(2980,2930,4046),FVector(4334,3680,4186),FVector(4180,4220,4242),FVector(3660,4380,4242),FVector(2800,5200,4293)};
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderReferenceStart.png"),false,false);
        Phase=2;Elapsed=0;return;
    }
    if(Phase==2)
    {
        if(!Player || !Terrain){Finish(false,TEXT("Reference terrain or player disappeared"));return;}
        if(Player->RecoveryCount!=RecoveryBaseline){Finish(false,TEXT("Reference route fell or died; no teleport recovery is accepted"));return;}
        auto* Movement=Player->GetCharacterMovement();
        if(bFlatJumpStarted)JumpPeak=FMath::Max(JumpPeak,static_cast<float>(Player->GetActorLocation().Z)-JumpStartZ);
        if(JumpTime>0){JumpTime-=DeltaSeconds;if(JumpTime<=0)Player->StopJumping();}
        if(Waypoint>=Route.Num())
        {
            if(Player->GetActorLocation().Z<4200 || JumpPeak<100){Finish(false,TEXT("Walking route did not reach the real summit or ordinary jump did not rise 100cm"));return;}
            if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
            PC->SetControlRotation(FRotator(-20,210,0));
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderReferenceSummit.png"),false,false);
            Phase=3;Elapsed=0;return;
        }
        const FVector Target=Route[Waypoint];const FVector Delta=Target-Player->GetActorLocation();LegElapsed+=DeltaSeconds;
        if(Waypoint==1 && LegElapsed>.5f && !bFlatJumpStarted && Movement->IsMovingOnGround())
        {bFlatJumpStarted=true;JumpStartZ=Player->GetActorLocation().Z;Player->Jump();JumpTime=.14f;}
        if(LegElapsed>18){Finish(false,FString::Printf(TEXT("Ordinary terrain walking blocked at waypoint %d: %s target %s"),Waypoint,*Player->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<65)
        {
            UE_LOG(LogTemp,Display,TEXT("REFERENCE WALK %d feet %s"),Waypoint,*(Player->GetActorLocation()-FVector(0,0,82)).ToString());
            if(Waypoint==8 || Waypoint==19)FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/(Waypoint==8?TEXT("Validation/ThunderReferenceBridge.png"):TEXT("Validation/ThunderReferenceMountain.png")),false,false);
            ++Waypoint;LegElapsed=0;return;
        }
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        Player->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        // 目標足場が高く近い時だけ通常Jumpを使います。加速・座標・速度を直接設定しません。
        const float Feet=Player->GetActorLocation().Z-82;
        if(Movement->IsMovingOnGround() && JumpTime<=0 && Delta.Size2D()<450 && Target.Z>Feet+70){Player->Jump();JumpTime=.14f;}
        return;
    }
    if(Phase==3 && Elapsed>2){Tap(PC,EKeys::R);Phase=4;Elapsed=0;return;}
    if(Phase==4 && Elapsed>1)
    {
        if(!Player || Player->RecoveryCount!=RecoveryBaseline+1 || FVector::Dist2D(Player->GetActorLocation(),FVector(-6558,-6464,0))>40){Finish(false,TEXT("R did not reset through the normal stage spawn"));return;}
        Tap(PC,EKeys::M);Phase=5;Elapsed=0;return;
    }
    if(Phase==5 && Map==TEXT("MissionSelect") && Elapsed>2)
    {
        if(Progress->GetEarnedRewardCount()!=0 || UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(),0)){Finish(false,TEXT("Terrain-only test unexpectedly wrote progress"));return;}
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/CourseComplete.png"),false,false);Phase=6;Elapsed=0;return;
    }
    if(Phase==6 && Elapsed>1)Finish(true,TEXT("Reference terrain ordinary walking: start, wooden bridge, winding mountain summit, actual R and M; no deaths, no reward fixtures or progress writes."));
}
