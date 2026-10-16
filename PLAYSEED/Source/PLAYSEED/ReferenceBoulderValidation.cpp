#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceBoulderField.h"
#include "ReferenceBoulderWalk.h"
#include "ThunderMeadowStage.h"
#include "ThunderRollingBoulder.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceBoulders(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Field=Stage?Stage->GetReferenceBoulderField():nullptr;
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Hero||!Field||Field->GetBalls().Num()!=5||Field->GetFloorFailures()!=0||Progress->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Reference boulder field lost its five real balls, floors or legitimate earned7"));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Walk=[&](FVector Target)
    {
        const FVector Delta=Target-Hero->GetActorLocation();
        const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(Delta.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Move->IsMovingOnGround()&&Delta.Size2D()<350&&Target.Z>Hero->GetActorLocation().Z-82+38&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    if(Phase==130||Phase==133)
    {
        const bool bPit=Phase==130;const auto* Points=bPit?ReferencePitApproach:ReferenceRoadApproach;
        const int32 Count=bPit?UE_ARRAY_COUNT(ReferencePitApproach):UE_ARRAY_COUNT(ReferenceRoadApproach);
        const int32 ExpectedRecovery=bPit?0:1;
        if(Hero->RecoveryCount!=ExpectedRecovery){Finish(false,TEXT("Reference walking route unexpectedly recovered"));return;}
        if(Health->GetHealth()!=8)
        {
            // 最後の足元へ着く前に目的の実鉄球が当たっても、その本物の接触を検証します。
            if(Health->GetHealth()==6&&Field->GetBalls()[bPit?2:1]->GetConfirmedHits()==1){Phase=bPit?131:134;Elapsed=0;return;}
            Finish(false,TEXT("Reference walking route took unintended damage"));return;
        }
        if(BoulderWaypoint>=Count)
        {if(!Move->IsMovingOnGround())return;Move->StopMovementImmediately();Phase=bPit?131:134;Elapsed=0;return;}
        const auto& Point=Points[BoulderWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();BoulderLegElapsed+=DeltaSeconds;
        if(BoulderLegElapsed>22){Finish(false,FString::Printf(TEXT("Boulder walk blocked phase%d point%d hero%s target%s floor%s"),Phase,BoulderWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<(Move->IsMovingOnGround()?160:80))
        {++BoulderWaypoint;BoulderLegElapsed=0;return;}
        Walk(Target);return;
    }
    if(Phase==131||Phase==134)
    {
        const bool bPit=Phase==131;auto* Ball=Field->GetBalls()[bPit?2:1];
        if(Hero->RecoveryCount!=(bPit?0:1)){Finish(false,TEXT("Boulder contact unexpectedly killed or recovered the hero"));return;}
        if(Health->GetHealth()<8)
        {
            if(Health->GetHealth()!=6||Ball->GetConfirmedHits()!=1||Health->GetLastImpulse().Size2D()<400)
            {Finish(false,TEXT("Moving reference ball did not deal one real two-point impact and knockback"));return;}
            if(bPit)bBoulderPitVerified=true;else bBoulderRoadVerified=true;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/(bPit?TEXT("Validation/ThunderPitBoulderImpact.png"):TEXT("Validation/ThunderRoadBoulderImpact.png")),false,false);
            Phase=bPit?132:135;Elapsed=0;return;
        }
        if(bPit)Walk(Ball->GetActorLocation()-FVector(0,0,100));
        else
        {
            const auto& Last=ReferenceRoadApproach[UE_ARRAY_COUNT(ReferenceRoadApproach)-1];Walk(FVector(Last.X,Last.Y,Last.Z));
            if(!Ball->IsWaiting())PC->SetControlRotation(FRotator(-18,(Ball->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        }
        if(Elapsed>55)Finish(false,TEXT("Ordinary waiting/walking did not contact a moving reference ball"));return;
    }
    if(Phase==132||Phase==135)
    {
        if(Elapsed<.6f)return;
        const bool bPit=Phase==132;const int32 Before=bPit?0:1;
        if(Hero->RecoveryCount==Before){Tap(PC,EKeys::R);return;}
        if(Hero->RecoveryCount!=Before+1||Health->GetHealth()!=8||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
        {Finish(false,TEXT("Real R did not restore hero vitality and course entrance"));return;}
        for(AThunderRollingBoulder* Ball:Field->GetBalls())if(Ball->IsWaiting()||Ball->GetTravelledDistance()>150)
        {Finish(false,TEXT("Real R did not reset all five reference balls"));return;}
        if(bPit){Phase=133;BoulderWaypoint=0;BoulderLegElapsed=0;Elapsed=0;return;}
        bBoulderResetVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderBoulderFieldReset.png"),false,false);Tap(PC,EKeys::M);Phase=9;Elapsed=0;return;
    }
}
