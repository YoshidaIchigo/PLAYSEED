#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceRedWalk.h"
#include "ReferenceCheckerPlatform.h"
#include "ReferenceTravelSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderTiltingBridge.h"
#include "ThunderScarletTrail.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "PlayseedClimbComponent.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceRed(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Trail=Stage?Stage->GetScarletTrail():nullptr;
    if(!Hero||!Trail||Trail->GetCoins().Num()!=8||Stage->GetReferenceCheckers().Num()!=2||Hero->RecoveryCount!=0)
    {Finish(false,FString::Printf(TEXT("Reference red route failed phase%d point%d recovery%d hero%s"),Phase,RedWaypoint,Hero?Hero->RecoveryCount:-1,Hero?*Hero->GetActorLocation().ToString():TEXT("missing")));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    if((Phase==70||Phase==73||Phase==75||Phase==78)&&UpdateReferenceRouteBombs(DeltaSeconds,Hero,PC)){Elapsed-=DeltaSeconds;return;}
    if((Phase==70||Phase==73||Phase==75||Phase==78)&&UpdateReferenceRouteCombat(DeltaSeconds,Hero,PC)){Elapsed-=DeltaSeconds;return;}
    if((Phase==70||Phase==73||Phase==75||Phase==78)&&UpdateReferenceRoadAvoidance(DeltaSeconds,Hero,PC)){Elapsed-=DeltaSeconds;return;}
    if(Stage->GetTiltingBridge()&&Hero->GetMovementBaseObject()==Stage->GetTiltingBridge()->GetSurface())bRedBridgeRidden=true;
    auto Walk=[&](FVector Target)
    {
        const FVector Delta=Target-Hero->GetActorLocation();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Move->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470);
            const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
    };
    auto Jump=[&]()
    {if(Move->IsMovingOnGround()&&JumpHold<=0){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}};
    if(Phase==70||Phase==73||Phase==75||Phase==78)
    {
        const auto* Points=Phase==70?ReferenceRedFirst:Phase==73?ReferenceRedSecond:Phase==75?ReferenceRedIsland:ReferenceRedReturn;
        const int32 Count=Phase==70?UE_ARRAY_COUNT(ReferenceRedFirst):Phase==73?UE_ARRAY_COUNT(ReferenceRedSecond):Phase==75?UE_ARRAY_COUNT(ReferenceRedIsland):UE_ARRAY_COUNT(ReferenceRedReturn);
        if(RedWaypoint>=Count)
        {
            if(!Move->IsMovingOnGround())return;
            Move->StopMovementImmediately();
            if(Phase==70){Phase=71;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRedElevators.png"),false,false);}
            else if(Phase==73)
            {
                if(Trail->GetCollectedCount()!=7||!bRedPlatformRidden||!bRedBridgeRidden){Finish(false,TEXT("The real ground route did not collect seven reds with ordinary platform and bridge contact"));return;}
                Phase=60;CannonWaypoint=0;CannonLegElapsed=0;
            }
            else if(Phase==75){Phase=76;Tap(PC,EKeys::F);bRedClimbSent=true;}
            else Phase=79;
            RedWaypoint=0;RedLegElapsed=0;Elapsed=0;return;
        }
        const auto& Point=Points[RedWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();RedLegElapsed+=DeltaSeconds;
        if(RedLegElapsed>18){Finish(false,FString::Printf(TEXT("Red ordinary walk blocked phase%d point%d hero%s target%s coin%d"),Phase,RedWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),Point.Coin));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80&&(!Point.Coin||Trail->GetCoins()[Point.Coin-1]->IsCollected()))
        {UE_LOG(LogTemp,Display,TEXT("RED WALK phase%d point%d collected%d/8"),Phase,RedWaypoint,Trail->GetCollectedCount());++RedWaypoint;RedLegElapsed=0;bRedRouteJump=false;return;}
        Walk(Target);
        if(Point.bJump&&!bRedRouteJump&&Move->IsMovingOnGround()){Jump();bRedRouteJump=true;}
        if(Move->IsMovingOnGround()&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)Jump();
        return;
    }
    if(Phase==71)
    {
        AReferenceCheckerPlatform* Platform=Stage->GetReferenceCheckers()[0];const FVector Top=Platform->GetSurfaceTop();
        if(Hero->GetMovementBaseObject()==Platform->GetSurface())
        {
            if(!bRedPlatformRidden){UE_LOG(LogTemp,Display,TEXT("RED ELEVATOR: actual CharacterMovement base contact at%s"),*Hero->GetActorLocation().ToString());FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRedPlatformRide.png"),false,false);}
            bRedPlatformRidden=true;RedPlatformRideSeconds+=DeltaSeconds;
            // 実際に運ばれる時間を残してからジャンプし、接地1フレームだけの動画にならないようにします。
            if(RedPlatformRideSeconds<.2f){Move->StopMovementImmediately();return;}
            if(!bRedPlatformJump){Jump();bRedPlatformJump=true;}
        }
        if(Trail->GetCoins()[4]->IsCollected()&&bRedPlatformRidden)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Released,0));Phase=72;Elapsed=0;return;}
        if(!bRedBoardAttempt&&Platform->IsRising()&&Top.Z>680&&Top.Z<730&&FMath::Abs(Top.Y+4466)<20)
        {bRedBoardAttempt=true;PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Pressed,1));}
        if(bRedBoardAttempt)Walk(bRedPlatformRidden?FVector(1600,-4440,980):Top);
        else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        if(Elapsed>25)Finish(false,FString::Printf(TEXT("Normal elevator boarding/jump did not collect red5: ridden%d jump%d hero%s platform%s actionRising%d"),bRedPlatformRidden,bRedPlatformJump,*Hero->GetActorLocation().ToString(),*Top.ToString(),Platform->IsRising()));return;
    }
    if(Phase==72)
    {
        const FVector Target(1300,-4450,0);Walk(Target);
        if(Move->IsMovingOnGround()&&FVector::Dist2D(Hero->GetActorLocation(),Target)<35&&FMath::Abs(Hero->GetActorLocation().Z-82)<80)
        {Move->StopMovementImmediately();Phase=73;Elapsed=0;RedWaypoint=0;RedLegElapsed=0;return;}
        if(Elapsed>20)Finish(false,TEXT("Ordinary movement did not leave the elevator toward the real lower field"));return;
    }
    if(Phase==76)
    {
        if(Hero->GetClimb()->IsClimbing())
        {
            if(bRedClimbSent){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Pressed,1));bRedClimbSent=false;}
            if(Hero->GetActorLocation().Z>3472)bRedTreeClimbed=true;
        }
        if(Trail->GetCoins()[7]->IsCollected())
        {
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0));if(Hero->GetClimb()->IsClimbing())Tap(PC,EKeys::F);
            if(!bRedTreeClimbed||Trail->GetCollectedCount()!=8||Trail->GetRewardGenerationCount()!=1||!Trail->GetReward()||FVector::Dist(Trail->GetReward()->GetRestLocation(),FVector(-6000,-2400,1000))>1)
            {Finish(false,TEXT("Normal F/W climb did not generate exactly one fixed-position star4 after all eight reds"));return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRedTreeCoin.png"),false,false);
            Phase=bHundredTest?123:77;HundredWaypoint=0;HundredLegElapsed=0;Elapsed=0;return;
        }
        if(Elapsed>12)Finish(false,FString::Printf(TEXT("Normal F/W did not climb the island tree and touch red8: climbing%d hero%s"),Hero->GetClimb()->IsClimbing(),*Hero->GetActorLocation().ToString()));return;
    }
    if(Phase==77)
    {
        // 幹の手前から反対側へ直進するとぶつかるため、東側の実上面を経由します。
        const FVector Target=RedDescentPoint==0?FVector(4300,-1750,3072):FVector(3200,-2500,746.425);Walk(Target);
        if(RedDescentPoint==0&&Move->IsMovingOnGround()&&FVector::Dist2D(Hero->GetActorLocation(),Target)<35&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {++RedDescentPoint;Elapsed=0;return;}
        if(Move->IsMovingOnGround()&&FVector::Dist2D(Hero->GetActorLocation(),Target)<35&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {Move->StopMovementImmediately();Phase=78;Elapsed=0;RedWaypoint=0;RedLegElapsed=0;return;}
        if(Elapsed>25)Finish(false,FString::Printf(TEXT("Normal movement did not descend from island to the lower terrain: hero%s"),*Hero->GetActorLocation().ToString()));return;
    }
    if(Phase==79)
    {
        if(Flow->IsCelebrating())
        {ObservedStar=Trail->GetReward();Phase=26;Elapsed=0;return;}
        const FVector Target(-6000,-2400,1000);const FVector Delta=Target-Hero->GetActorLocation();
        if(Delta.Size2D()>12)Walk(Target);else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        if(Delta.Size2D()<50)Jump();
        if(Elapsed>15)Finish(false,TEXT("Actual eight-red star4 did not collect through ordinary final walking/jump"));
    }
}
