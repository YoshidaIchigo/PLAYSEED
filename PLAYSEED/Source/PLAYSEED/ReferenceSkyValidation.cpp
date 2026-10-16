#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWingWalk.h"
#include "ReferenceEntranceGameMode.h"
#include "ReferenceHero.h"
#include "ReferenceTravelSubsystem.h"
#include "ReferenceWingBox.h"
#include "ReferenceWingCap.h"
#include "ThunderFlightSwitch.h"
#include "ThunderMeadowStage.h"
#include "ThunderSkyChimes.h"
#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedActionComponent.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceSky(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    if(!Hero||Hero->RecoveryCount!=0){Finish(false,TEXT("Wing ordinary route lost the hero or recovered unexpectedly"));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Flight=Hero->GetFlight();auto* Launch=Hero->GetLaunch();
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Jump=[&]()
    {if(Move->IsMovingOnGround()&&JumpHold<=0){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}};
    auto Walk=[&](FVector Target)
    {
        const FVector Delta=Target-Hero->GetActorLocation();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Move->IsFalling())
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else Hero->AddMovementInput(Delta.GetSafeNormal2D(),FMath::Clamp(Delta.Size2D()/140,.25f,1.f));
    };
    if(Phase<=92)
    {
        auto* Room=Cast<AReferenceEntranceRoom>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceEntranceRoom::StaticClass()));
        auto* Switch=Room?Room->GetWingSwitch():nullptr;if(!Switch){Finish(false,TEXT("Ordinary wing switch was not present in the entrance"));return;}
        if(Phase==90)
        {
            if(Progress->IsFlightUnlocked()&&Switch->IsActivated())
            {if(Elapsed<.7f){PC->SetControlRotation(FRotator(-25,-90,0));return;}bWingHubChecked=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWingSwitch.png"),false,false);Phase=92;Elapsed=0;return;}
            // まっすぐ進むと固いボタンに当たるため、横を通ってジャンプ開始地点へ回り込みます。
            static const FVector Approach[]={FVector(-900,0,0),FVector(-900,-750,0),FVector(-600,-750,0)};
            const FVector Target=Approach[FMath::Min(WingWaypoint,2)];Walk(Target);
            if(FVector::Dist2D(Hero->GetActorLocation(),Target)<25&&Move->IsMovingOnGround())
            {Move->StopMovementImmediately();Elapsed=0;if(++WingWaypoint==3)Phase=91;return;}
            if(Elapsed>10)Finish(false,FString::Printf(TEXT("Ordinary hub walking did not reach the side of the red switch: point%d hero%s"),WingWaypoint,*Hero->GetActorLocation().ToString()));return;
        }
        if(Phase==91)
        {
            const FVector Target(-600,-550,40);const FVector Delta=Target-Hero->GetActorLocation();
            if(!bWingHubJumped&&Move->IsMovingOnGround()){Jump();bWingHubJumped=true;}
            Walk(Target);
            if(!bWingPoundSent&&Move->IsFalling()&&Hero->GetActorLocation().Z-82>100&&Hero->GetVelocity().Z<150&&Delta.Size2D()<70)
            {Tap(PC,EKeys::LeftControl);bWingPoundSent=true;}
            if(Progress->IsFlightUnlocked()&&Switch->IsActivated()&&Move->IsMovingOnGround()&&!Hero->GetActions()->IsGroundPounding())
            {
                if(!bWingPoundSent||Switch->GetActivationCount()!=1){Finish(false,TEXT("Wing access changed without one actual red-switch ground pound"));return;}
                bWingHubChecked=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWingSwitch.png"),false,false);
                UE_LOG(LogTemp,Display,TEXT("WING QA: normal hub pound actually saved the unlock; earned count remains%d"),Progress->GetEarnedRewardCount());Phase=92;Elapsed=0;return;
            }
            if(Elapsed>10)Finish(false,FString::Printf(TEXT("Normal Space/Ctrl did not activate the red hub switch: hero%s pound%d"),*Hero->GetActorLocation().ToString(),bWingPoundSent));return;
        }
        const FVector Target(-600,0,0);Walk(Target);
        if(FVector::Dist2D(Hero->GetActorLocation(),Target)<25&&Move->IsMovingOnGround())
        {Move->StopMovementImmediately();PC->SetControlRotation(FRotator(-10,0,0));Phase=1;Elapsed=0;return;}
        if(Elapsed>10)Finish(false,TEXT("Ordinary walking did not return from the switch to the painting approach"));return;
    }
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    if(!Stage||Stage->GetReferenceWingBoxes().Num()!=3||Stage->GetReferenceLaunchers().Num()!=6||!Stage->GetSkyChimes())
    {Finish(false,TEXT("Wing cap boxes, real rings or reference cannons were missing"));return;}
    AReferenceWingBox* Box=Stage->GetReferenceWingBoxes()[2];AThunderLauncher* Cannon=Stage->GetReferenceLaunchers()[0];auto* Rings=Stage->GetSkyChimes();
    if(Rings->GetCoins().Num()!=5||Rings->GetOuterCoins().Num()!=40||!Box->IsUnlocked())
    {Finish(false,TEXT("The real 5+40 gold coins or saved wing box access were missing"));return;}
    if(Phase==93||Phase==95)
    {
        const auto* Points=Phase==93?ReferenceWingCapWalk:ReferenceWingCannonWalk;
        const int32 Count=Phase==93?UE_ARRAY_COUNT(ReferenceWingCapWalk):UE_ARRAY_COUNT(ReferenceWingCannonWalk);
        if(WingWaypoint>=Count)
        {
            if(!Move->IsMovingOnGround())return;Move->StopMovementImmediately();Phase=Phase==93?94:96;Elapsed=0;WingWaypoint=0;WingLegElapsed=0;return;
        }
        const auto& Point=Points[WingWaypoint];const FVector Target(Point.X,Point.Y,Point.Z),Delta=Target-Hero->GetActorLocation();WingLegElapsed+=DeltaSeconds;
        if(WingLegElapsed>20){Finish(false,FString::Printf(TEXT("Wing ordinary walk blocked phase%d point%d hero%s target%s"),Phase,WingWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<80)
        {UE_LOG(LogTemp,Display,TEXT("WING WALK phase%d point%d power%.1f"),Phase,WingWaypoint,Flight->GetRemaining());++WingWaypoint;WingLegElapsed=0;return;}
        Walk(Target);if(Move->IsMovingOnGround()&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)Jump();return;
    }
    if(Phase==94)
    {
        const FVector Delta=Box->GetActorLocation()-Hero->GetActorLocation();PC->SetControlRotation(FRotator(-12,0,0));
        if(Delta.Size2D()>12)Walk(Box->GetActorLocation());else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        if(Flight->HasPower())
        {
            if(Box->GetSpawnCount()!=1||Box->GetHeadBumpCount()!=1||!Box->GetCap()||!Box->GetCap()->IsCollected()||Box->GetCap()->GetCollections()!=1||Flight->GetPowerGrants()!=1||Flight->GetRemaining()>60)
            {Finish(false,TEXT("Wing power did not follow one normal cap-box head bump and actual capsule pickup"));return;}
            if(!bWingPowerObserved){bWingPowerObserved=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWingCapPickup.png"),false,false);}
            if(Move->IsMovingOnGround()){Phase=95;Elapsed=0;WingWaypoint=0;WingLegElapsed=0;return;}
        }
        else if(Delta.Size2D()<50)Jump();
        if(Elapsed>12)Finish(false,FString::Printf(TEXT("Normal jumps did not break/pick up the wing cap: bumps%d spawn%d hero%s"),Box->GetHeadBumpCount(),Box->GetSpawnCount(),*Hero->GetActorLocation().ToString()));return;
    }
    if(Phase==96)
    {
        if(!Flight->HasPower()){Finish(false,TEXT("Wing power expired during the ordinary return to the field cannon"));return;}
        PC->SetControlRotation(FRotator(-12,(Cannon->GetActorLocation()-Hero->GetActorLocation()).Rotation().Yaw,0));
        if(!bWingAimSent&&Elapsed>.4f){Tap(PC,EKeys::G);bWingAimSent=true;}
        if(Launch->IsAiming())
        {
            const FVector Delta=Rings->GetCoins()[0]->GetActorLocation()-Cannon->GetSeatLocation();WingGoalYaw=Delta.Rotation().Yaw;WingGoalPitch=Delta.Rotation().Pitch;
            Phase=97;Elapsed=0;return;
        }
        if(Elapsed>4)Finish(false,TEXT("Normal G did not enter the open field cannon while powered"));return;
    }
    if(Phase==97)
    {
        if(!Launch->IsAiming()||!Flight->HasPower()){Finish(false,TEXT("Wing aim or power ended before normal LMB fire"));return;}
        const float Yaw=FRotator::NormalizeAxis(WingGoalYaw-Launch->GetAimYaw()),Pitch=WingGoalPitch-Launch->GetAimPitch();
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,FMath::Clamp(Yaw*.4f,-20.f,20.f)));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,FMath::Clamp(-Pitch*.4f,-20.f,20.f)));
        if(FMath::Abs(Yaw)<.08f&&FMath::Abs(Pitch)<.08f)WingAimStable+=DeltaSeconds;else WingAimStable=0;
        if(WingAimStable>.3f&&!bWingAimShot)
        {
            auto* Model=Cast<AReferenceHero>(Hero);if(!Model||!Model->IsBodyHiddenForCamera()){Finish(false,TEXT("Powered hero body obscured the close cannon camera"));return;}
            bWingCameraHidden=true;bWingAimShot=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWingCannonAim.png"),false,false);
        }
        if(WingAimStable>1){Tap(PC,EKeys::LeftMouseButton);Phase=98;Elapsed=0;return;}
        if(Elapsed>15)Finish(false,TEXT("Normal mouse aiming did not settle for the first real sky centre"));return;
    }
    if(Flow->IsCelebrating())
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0));PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::S,IE_Released,0));
        if(Rings->GetCollectedCount()!=5||Rings->GetRewardGenerationCount()!=1||!Rings->GetReward()||FVector::Dist(Rings->GetReward()->GetRestLocation(),FVector(-6600,-1250,1000))>1)
        {Finish(false,TEXT("Star5 did not follow five actual centre pickups and exactly one fixed reward generation"));return;}
        ObservedStar=Rings->GetReward();Phase=26;Elapsed=0;return;
    }
    if(Flight->IsFlying())bWingFlightObserved=true;
    if(Elapsed<.2f&&!Flight->IsFlying())return;
    if(!Flight->IsFlying()||!Flight->HasPower()||Launch->GetLaunchCount()!=1||Flight->GetFlightStarts()!=1)
    {Finish(false,FString::Printf(TEXT("Actual wing flight stopped before star5: centre%d remaining%.2f hero%s"),WingCentre,Flight->GetRemaining(),*Hero->GetActorLocation().ToString()));return;}
    if(Phase==98&&WingCentre<5&&Rings->GetCoins()[WingCentre]->IsCollected())
    {
        UE_LOG(LogTemp,Display,TEXT("WING RING: actual centre%d power%.2f"),WingCentre+1,Flight->GetRemaining());++WingCentre;Elapsed=0;
        if(WingCentre==5){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWingFiveCentres.png"),false,false);Phase=99;}
    }
    const FVector Target=Phase==99?Rings->GetRewardRestLocation():Rings->GetCoins()[WingCentre]->GetActorLocation(),Delta=Target-Hero->GetActorLocation();
    const FRotator Desired=Delta.Rotation();const float Yaw=FRotator::NormalizeAxis(Desired.Yaw-Flight->GetFlightYaw()),Pitch=FMath::Clamp(Desired.Pitch,-60.f,70.f)-Flight->GetFlightPitch();
    // 製品のマウス軸とW/Sだけで向きと速さを調整します。飛行速度や座標を直接設定しません。
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,FMath::Clamp(Yaw*.6f,-40.f,40.f)));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,FMath::Clamp(-Pitch*.6f,-40.f,40.f)));
    const bool bSlow=Delta.Size()<650;
    if(!bWingFlightKeysSet||bSlow!=bWingSlow)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,bSlow?IE_Released:IE_Pressed,bSlow?0:1));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::S,bSlow?IE_Pressed:IE_Released,bSlow?1:0));bWingSlow=bSlow;bWingFlightKeysSet=true;
    }
    if(Elapsed>(Phase==99?18:12))Finish(false,FString::Printf(TEXT("Normal mouse/W/S flight missed target phase%d centre%d hero%s target%s power%.2f"),Phase,WingCentre,*Hero->GetActorLocation().ToString(),*Target.ToString(),Flight->GetRemaining()));
}
