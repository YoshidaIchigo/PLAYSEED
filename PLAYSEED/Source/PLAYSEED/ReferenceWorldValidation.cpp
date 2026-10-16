#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWorldMechanisms.h"
#include "ReferenceWorldWalk.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderWarpPair.h"
#include "ThunderMeadowStage.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceWorld(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* World=Stage?Stage->GetReferenceWorldMechanisms():nullptr;
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Hero||!World||!Terrain||World->GetFloorFailures()!=0||World->GetWarpPairs().Num()!=2||Progress->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Reference world lost its real mechanisms, four floors or actual earned7"));return;}
    const auto& Pairs=World->GetWarpPairs();auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();
    const int32 Recovery=((Phase>=165&&Phase<=169)||Phase==176)?1:0;
    if(Phase==165&&Hero->RecoveryCount==0&&Elapsed<2)return;
    if(Hero->RecoveryCount!=Recovery){Finish(false,TEXT("Reference world suffered an unintended recovery"));return;}
    for(AThunderWarpPair* Pair:Pairs)if(Pair->HasFailedWarp()){Finish(false,TEXT("Actual warp destination failed its capsule collision check"));return;}
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Photo=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/FString(Name),false,false);};
    auto Walk=[&](FVector Target,bool bJump=false)
    {
        const FVector D=Target-Hero->GetActorLocation(),Desired=(D.GetSafeNormal2D()*D.Size2D()*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(D.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,D.Rotation().Yaw,0),DeltaSeconds,4));
        if(bJump&&!bWorldRouteJump&&Move->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bWorldRouteJump=true;}
        if(Move->IsMovingOnGround()&&JumpHold<=0&&D.Size2D()<400&&Target.Z>Hero->GetActorLocation().Z-82+38)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    auto Near=[&](FVector Target){return FVector::Dist2D(Hero->GetActorLocation(),Target)<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<(Move->IsMovingOnGround()?160:80);};
    auto Route=[&](const FReferenceWalkPoint* Points,int32 Count,bool bReverse=false)
    {
        if(WorldWaypoint>=Count){if(Move->IsMovingOnGround())Move->StopMovementImmediately();return true;}
        const auto& P=Points[bReverse?Count-1-WorldWaypoint:WorldWaypoint];const FVector Target(P.X,P.Y,P.Z);WorldLegElapsed+=DeltaSeconds;
        if(WorldLegElapsed>22){Finish(false,FString::Printf(TEXT("World ordinary walk blocked phase%d point%d hero%s target%s floor%s"),Phase,WorldWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString()));return false;}
        if(Near(Target)){if(WorldWaypoint%10==0)UE_LOG(LogTemp,Display,TEXT("WORLD WALK phase%d point%d hero%s"),Phase,WorldWaypoint,*Hero->GetActorLocation().ToString());++WorldWaypoint;WorldLegElapsed=0;return false;}
        Walk(Target,P.bJump);return false;
    };
    auto Begin=[&](int32 Next){Phase=Next;Elapsed=0;WorldWaypoint=0;WorldLegElapsed=0;};
    auto Hold=[&](FVector Subject){if(Move->IsMovingOnGround())Move->StopMovementImmediately();PC->SetControlRotation(FRotator(-15,(Subject-Hero->GetActorLocation()).Rotation().Yaw+20,0));};
    auto PlanExit=[&](FVector Floor)
    {
        FCollisionQueryParams Query(SCENE_QUERY_STAT(WorldWarpExitPlanning),false,Hero);
        for(int32 N=0;N<8;++N)
        {
            const FVector Direction=FRotator(0,Hero->GetActorRotation().Yaw+N*45,0).Vector();FVector Target;
            if(!Terrain->FindFloor(Floor+Direction*260,Target)||FMath::Abs(Target.Z-Floor.Z)>80)continue;
            if(GetWorld()->SweepTestByChannel(Hero->GetActorLocation(),Target+FVector(0,0,82),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(34,70),Query))continue;
            WorldExitTarget=Target;return true;
        }
        return false;
    };
    const FVector Normal=World->GetActorForwardVector(),Centre=World->GetActorLocation();
    if(Phase==160){if(Route(ReferenceWorldGateWalk,UE_ARRAY_COUNT(ReferenceWorldGateWalk)))Begin(161);return;}
    if(Phase==161)
    {
        if(World->IsGateOpen()||World->GetGateActivations()!=0){Finish(false,TEXT("Reference grill opened without its real floor switch"));return;}
        Walk(Centre+Normal*180);
        if(Elapsed>1.2f)
        {
            const float Side=FVector::DotProduct(Hero->GetActorLocation()-Centre,Normal);
            if(Side>-35||FVector::Dist2D(Hero->GetActorLocation(),Centre)>150){Finish(false,FString::Printf(TEXT("Closed real grill did not stop normal approach side%.1f hero%s"),Side,*Hero->GetActorLocation().ToString()));return;}
            Photo(TEXT("ThunderWorldGateClosed.png"));Begin(162);
        }
        return;
    }
    if(Phase==162){if(Route(ReferenceWorldSwitchWalk,UE_ARRAY_COUNT(ReferenceWorldSwitchWalk)))Begin(163);return;}
    if(Phase==163)
    {
        Walk(World->GetSwitchTop());
        if(World->IsGateOpen()&&World->GetGateOpening()>=1)
        {if(World->GetGateActivations()!=1){Finish(false,TEXT("Real floor switch activated more than once"));return;}Photo(TEXT("ThunderWorldGateOpen.png"));Begin(164);return;}
        if(Elapsed>10)Finish(false,TEXT("Ordinary standing on the actual purple switch did not open both leaves"));return;
    }
    if(Phase==164)
    {
        const FVector Target=Centre+Normal*220;Walk(Target);
        if(Near(Target)&&Move->IsMovingOnGround())
        {bWorldGateChecked=true;Photo(TEXT("ThunderWorldGateCrossed.png"));Tap(PC,EKeys::R);Begin(165);}
        if(Elapsed>10)Finish(false,TEXT("Open double grill still blocked ordinary crossing"));return;
    }
    if(Phase==165)
    {
        if(World->IsGateOpen()||World->GetGateOpening()!=0||World->GetGateActivations()!=0||Health->GetHealth()!=8||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
        {Finish(false,TEXT("Ordinary R did not close/rearm the grill and restore real entrance health"));return;}
        for(AThunderWarpPair* Pair:Pairs)if(Pair->GetWarpCount()!=0){Finish(false,TEXT("R did not rearm actual warp pairs"));return;}
        if(Elapsed>.35f){bWorldGateResetChecked=true;Photo(TEXT("ThunderWorldGateReset.png"));Begin(166);}return;
    }
    if(Phase==166)
    {
        if(Pairs[1]->GetWarpCount()==1){Begin(167);return;}
        if(Route(ReferenceWorldWestWalk,UE_ARRAY_COUNT(ReferenceWorldWestWalk)))Walk(Pairs[1]->GetCaveFloor(0));return;
    }
    if(Phase==167||Phase==176||Phase==172||Phase==178)
    {
        const bool bWest=Phase==167||Phase==176,bReturn=Phase==176||Phase==178;AThunderWarpPair* Pair=Pairs[bWest?1:0];const int32 Destination=bReturn?0:1;
        Hold(Pair->GetCaveFloor(Destination));
        if(Pair->GetWarpCount()!=(bReturn?2:1)||FVector::Dist2D(Hero->GetActorLocation(),Pair->GetCaveFloor(Destination))>90||FMath::Abs(Hero->GetActorLocation().Z-82-Pair->GetCaveFloor(Destination).Z)>30)
        {Finish(false,TEXT("Actual warp did not stay at its expected arrival without echo"));return;}
        if(Elapsed<2.2f)return;
        if(bWest&&!bReturn)
        {Photo(TEXT("ThunderWorldWestArrival.png"));if(!PlanExit(Pair->GetCaveFloor(1))){Finish(false,TEXT("No clear ordinary exit from the real west-warp destination"));return;}Begin(168);}
        else if(bWest)
        {bWorldWestWarpsChecked=true;bWorldFirstVisitDone=true;Photo(TEXT("ThunderWorldWestReturn.png"));Tap(PC,EKeys::M);Begin(13);}
        else if(!bReturn)
        {WorldHealthBaseline=Health->GetHealth();Photo(TEXT("ThunderWorldMountainArrival.png"));Begin(174);}
        else{bWorldMountainWarpsChecked=true;Photo(TEXT("ThunderWorldMountainReturn.png"));Begin(9);}
        return;
    }
    if(Phase==168){Walk(WorldExitTarget);if(Near(WorldExitTarget)&&Move->IsMovingOnGround())Begin(169);if(Elapsed>8)Finish(false,TEXT("Ordinary warp exit was blocked"));return;}
    if(Phase==169){if(Pairs[1]->GetWarpCount()==2){Begin(176);return;}Walk(Pairs[1]->GetCaveFloor(1));if(Elapsed>8)Finish(false,TEXT("Physical exit/reentry did not trigger the reverse west warp"));return;}
    if(Phase==170){if(Route(ReferenceWorldMountainWalk,UE_ARRAY_COUNT(ReferenceWorldMountainWalk)))Begin(171);return;}
    if(Phase==171){if(Pairs[0]->GetWarpCount()==1){Begin(172);return;}if(Route(ReferenceWorldWarpWalk,UE_ARRAY_COUNT(ReferenceWorldWarpWalk)))Walk(Pairs[0]->GetCaveFloor(0));return;}
    if(Phase==174){if(Route(ReferenceWorldHeartWalk,UE_ARRAY_COUNT(ReferenceWorldHeartWalk)))Begin(175);return;}
    if(Phase==175)
    {
        Hold(World->GetHeartLocation());
        if(Health->GetHealth()==8&&World->GetHeartHeals()>0&&WorldHealthBaseline<8)
        {bWorldHeartChecked=true;Photo(TEXT("ThunderWorldHeartHealed.png"));Begin(177);return;}
        if(Elapsed>18)Finish(false,FString::Printf(TEXT("Actual heart contact did not heal real damage baseline%d health%d heals%d hero%s heart%s"),WorldHealthBaseline,Health->GetHealth(),World->GetHeartHeals(),*Hero->GetActorLocation().ToString(),*World->GetHeartLocation().ToString()));return;
    }
    if(Phase==177)
    {
        if(Pairs[0]->GetWarpCount()==2){Begin(178);return;}
        if(Route(ReferenceWorldHeartWalk,UE_ARRAY_COUNT(ReferenceWorldHeartWalk),true))Walk(Pairs[0]->GetCaveFloor(1));return;
    }
}
