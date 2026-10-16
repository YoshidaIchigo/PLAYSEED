#include "ReferenceCourseTestSubsystem.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ReferenceSummitWalk.h"
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
    if(Elapsed>420){Finish(false,TEXT("Reference terrain route timed out"));return;}
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
        FVector StartFloor;
        if(!Terrain->FindFloor(FVector(-6558,-6464,0),StartFloor) || !Player->GetCharacterMovement()->IsMovingOnGround())
        {Finish(false,TEXT("Reference start does not have an upward facing walkable floor and grounded character"));return;}
        if(FVector::Dist2D(Player->GetActorLocation(),FVector(-6558,-6464,0))>30){Finish(false,TEXT("Reference spawn is not at the documented start"));return;}
        RecoveryBaseline=Player->RecoveryCount;
        Route.Reset();for(const auto& Point:ReferenceSummitWalk)Route.Add(FVector(Point.X,Point.Y,Point.Z));
        bFlatJumpStarted=true;JumpStartZ=Player->GetActorLocation().Z;PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpTime=.14f;
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderReferenceStart.png"),false,false);
        Phase=2;Elapsed=0;return;
    }
    if(Phase==2)
    {
        if(!Player || !Terrain){Finish(false,TEXT("Reference terrain or player disappeared"));return;}
        if(Player->RecoveryCount!=RecoveryBaseline){Finish(false,TEXT("Reference route fell or died; no teleport recovery is accepted"));return;}
        auto* Movement=Player->GetCharacterMovement();
        if(bFlatJumpStarted && !bFlatJumpFinished)
        {
            JumpObservationTime+=DeltaSeconds;
            if(Movement->IsFalling()){bFlatJumpAirborne=true;JumpPeak=FMath::Max(JumpPeak,static_cast<float>(Player->GetActorLocation().Z)-JumpStartZ);}
            else if(bFlatJumpAirborne && Movement->IsMovingOnGround())bFlatJumpFinished=true;
            if(JumpObservationTime>3 && !bFlatJumpFinished){Finish(false,FString::Printf(TEXT("Jump input did not take off and land: airborne%d finished%d peak%.1f hero%s velocity%s movement%d"),bFlatJumpAirborne,bFlatJumpFinished,JumpPeak,*Player->GetActorLocation().ToString(),*Player->GetVelocity().ToString(),Movement->MovementMode));return;}
        }
        if(JumpTime>0){JumpTime-=DeltaSeconds;if(JumpTime<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
        if(Waypoint>=Route.Num())
        {
            if(Player->GetActorLocation().Z<4200 || !bFlatJumpFinished || JumpPeak<100){Finish(false,TEXT("Walking route did not reach the real summit or ordinary jump did not rise 100cm"));return;}
            if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
            PC->SetControlRotation(FRotator(-20,210,0));
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderReferenceSummit.png"),false,false);
            Phase=3;Elapsed=0;return;
        }
        const FVector Target=Route[Waypoint];const FVector Delta=Target-Player->GetActorLocation();LegElapsed+=DeltaSeconds;
        if(LegElapsed>18){Finish(false,FString::Printf(TEXT("Ordinary terrain walking blocked at waypoint %d: %s target %s"),Waypoint,*Player->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25 && FMath::Abs(Player->GetActorLocation().Z-82-Target.Z)<80)
        {
            UE_LOG(LogTemp,Display,TEXT("REFERENCE WALK %d feet %s"),Waypoint,*(Player->GetActorLocation()-FVector(0,0,82)).ToString());
            if(ReferenceSummitWalk[Waypoint].Milestone>0)FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/(ReferenceSummitWalk[Waypoint].Milestone==1?TEXT("Validation/ThunderReferenceBridge.png"):TEXT("Validation/ThunderReferenceMountain.png")),false,false);
            ++Waypoint;LegElapsed=0;bRouteJumpIssued=false;return;
        }
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Movement->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470);
            const FVector Correction=Desired-FVector(Player->GetVelocity().X,Player->GetVelocity().Y,0);
            Player->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Player->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
        // 目標足場が高く近い時だけ通常Jumpを使います。加速・座標・速度を直接設定しません。
        const float Feet=Player->GetActorLocation().Z-82;
        if(ReferenceSummitWalk[Waypoint].bJump && !bRouteJumpIssued && Movement->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpTime=.14f;bRouteJumpIssued=true;UE_LOG(LogTemp,Display,TEXT("REFERENCE WALK: ordinary jump across real mountain path gap"));}
        if(Movement->IsMovingOnGround() && JumpTime<=0 && Delta.Size2D()<450 && Target.Z>Feet+70){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpTime=.14f;}
        return;
    }
    if(Phase==3)
    {
        if(Elapsed>.3f && !bHeroViewTurned)
        {
            PC->SetControlRotation(FRotator(-8,Player->GetActorRotation().Yaw+180,0));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis,IE_Axis,4));
            bHeroViewTurned=true;
        }
        if(Elapsed>1.5f && !bHeroViewCaptured)
        {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderReferenceHero.png"),false,false);bHeroViewCaptured=true;}
        if(Elapsed>3.5f){Tap(PC,EKeys::R);Phase=4;Elapsed=0;}
        return;
    }
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
