#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceChainWalk.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderBoundSentinel.h"
#include "ThunderLivesComponent.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

// 自分から普通に近づき、製品の実接触だけで死亡・残機・報酬保持を確かめます。
void UReferencePaintingTestSubsystem::UpdateReferenceDeath(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Hero||!Stage||!Stage->GetBoundSentinel()||!Mode||!Mode->GetLives()||Progress->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Actual death test lost its normal world or seven-star save"));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();auto* Lives=Mode->GetLives();auto* Chomp=Stage->GetBoundSentinel();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Walk=[&](FVector Target,bool bJump)
    {
        const FVector D=Target-Hero->GetActorLocation(),Desired=(D.GetSafeNormal2D()*D.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(D.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,D.Rotation().Yaw,0),DeltaSeconds,4));
        if(bJump&&Move->IsMovingOnGround()&&JumpHold<=0&&D.Size2D()<350&&Target.Z>Hero->GetActorLocation().Z-82+38)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    if(Phase==200)
    {
        if(Hero->RecoveryCount!=0||Lives->GetRemaining()!=3||Health->GetDefeatCount()!=0){Finish(false,TEXT("Ordinary death approach recovered unexpectedly"));return;}
        if(UpdateReferenceRouteBombs(DeltaSeconds,Hero,PC)||UpdateReferenceRouteCombat(DeltaSeconds,Hero,PC)){Elapsed-=DeltaSeconds;return;}
        // 接触が始まる前の同じ上段で計測へ移り、最初の実被弾も数えます。
        const bool bNearChomp=Move->IsMovingOnGround()&&FVector::Dist2D(Hero->GetActorLocation(),Chomp->GetAnchor())<1500&&FMath::Abs(Hero->GetActorLocation().Z-82-Chomp->GetAnchor().Z)<80;
        if(bNearChomp||DeathWaypoint>=UE_ARRAY_COUNT(ReferenceChainApproach))
        {
            if(!Move->IsMovingOnGround())return;
            if(Health->GetHealth()!=8){Finish(false,TEXT("Ordinary death approach must reach Chomp with real health8"));return;}
            DeathImpactBaseline=Health->GetSuccessfulImpacts();DeathDefeatBaseline=Health->GetDefeatCount();bDeathStarted=true;Phase=201;Elapsed=0;return;
        }
        const auto& P=ReferenceChainApproach[DeathWaypoint];const FVector Target(P.X,P.Y,P.Z);DeathLegElapsed+=DeltaSeconds;
        if(FVector::Dist2D(Target,Hero->GetActorLocation())<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80){++DeathWaypoint;DeathLegElapsed=0;return;}
        Walk(Target,true);if(DeathLegElapsed>22)Finish(false,TEXT("Ordinary walking to actual death contact was blocked"));return;
    }
    if(Phase==201)
    {
        if(Hero->RecoveryCount==1)
        {
            if(!bDeathStarted||Health->GetSuccessfulImpacts()!=DeathImpactBaseline+4||Health->GetDefeatCount()!=DeathDefeatBaseline+1||Lives->GetRemaining()!=2||Lives->GetConsumedLives()!=1||Health->GetHealth()!=8||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
            {Finish(false,TEXT("Four actual contacts did not consume exactly one life and restore the normal entrance"));return;}
            for(int32 Number=1;Number<=7;++Number)if(!Progress->HasEarnedReward(Number)){Finish(false,TEXT("A real death lost a saved star ID"));return;}
            bDeathVerified=true;Phase=202;Elapsed=0;UE_LOG(LogTemp,Display,TEXT("DEATH QA: four actual2damage contacts, defeat1/recovery1/lives2/health8/earned7 retained"));return;
        }
        if(Hero->RecoveryCount!=0||Lives->GetRemaining()!=3){Finish(false,TEXT("Actual death contact consumed unexpected lives"));return;}
        const FVector Center=Chomp->GetActorLocation();Walk(FVector(Center.X,Center.Y,Chomp->GetAnchor().Z),false);
        if(Elapsed>60)Finish(false,TEXT("Normal repeated Chomp contact did not cause a real death"));return;
    }
    if(Phase==202&&Elapsed>1)
    {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderFinalDeath.png"),false,false);Phase=9;Elapsed=0;}
}
