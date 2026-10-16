#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWaterCannon.h"
#include "ReferenceWaterCannonWalk.h"
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

void UReferencePaintingTestSubsystem::UpdateReferenceWaterCannons(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Hero||!Stage||Progress->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Water cannon validation lost its hero, stage or actual earned7"));return;}
    const auto& Cannons=Stage->GetReferenceWaterCannons();
    if(Cannons.Num()!=(bFieldWaterCannonChecked?1:2))
    {Finish(false,TEXT("Actual mission-specific water cannon count differs"));return;}
    AReferenceWaterCannon* Cannon=Cannons[bFieldWaterCannonChecked?0:1];
    if(Cannon->CanFireWater()!=bFieldWaterCannonChecked)
    {Finish(false,TEXT("Field/mountain water cannon firing parameter differs"));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();
    if(Hero->RecoveryCount!=(Phase==155?1:0))
    {Finish(false,TEXT("Unintended recovery during ordinary water cannon approach"));return;}
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Walk=[&](FVector Target,bool bJump)
    {
        const FVector Delta=Target-Hero->GetActorLocation();
        const FVector Desired=(Delta.GetSafeNormal2D()*Delta.Size2D()*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(Delta.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(bJump&&!bWaterCannonRouteJump&&Move->IsMovingOnGround())
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bWaterCannonRouteJump=true;}
        if(Move->IsMovingOnGround()&&JumpHold<=0&&Delta.Size2D()<450&&Target.Z>Hero->GetActorLocation().Z-12)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    if(Phase==150||Phase==152)
    {
        const FReferenceWalkPoint* Points=Phase==152?ReferenceWaterMacroApproach:bFieldWaterCannonChecked?ReferenceWaterMountainApproach:ReferenceFieldWaterApproach;
        const int32 Count=Phase==152?UE_ARRAY_COUNT(ReferenceWaterMacroApproach):bFieldWaterCannonChecked?UE_ARRAY_COUNT(ReferenceWaterMountainApproach):UE_ARRAY_COUNT(ReferenceFieldWaterApproach);
        if(WaterCannonWaypoint>=Count)
        {
            if(Move->IsMovingOnGround())Move->StopMovementImmediately();
            if(bFieldWaterCannonChecked&&Phase==150){Phase=152;WaterCannonWaypoint=0;WaterCannonLegElapsed=0;Elapsed=0;return;}
            WaterCannonHealthBaseline=Health->GetHealth();WaterCannonShotBaseline=Cannon->GetShotCount();
            Phase=bFieldWaterCannonChecked?153:151;Elapsed=0;return;
        }
        const auto& Point=Points[WaterCannonWaypoint];const FVector Target(Point.X,Point.Y,Point.Z);const FVector Delta=Target-Hero->GetActorLocation();WaterCannonLegElapsed+=DeltaSeconds;
        if(WaterCannonLegElapsed>22)
        {Finish(false,FString::Printf(TEXT("Water cannon walk blocked phase%d point%d hero%s target%s floor%s"),Phase,WaterCannonWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<(Move->IsMovingOnGround()?160:80))
        {if(WaterCannonWaypoint%10==0)UE_LOG(LogTemp,Display,TEXT("WATER CANNON WALK phase%d point%d hero%s"),Phase,WaterCannonWaypoint,*Hero->GetActorLocation().ToString());++WaterCannonWaypoint;WaterCannonLegElapsed=0;return;}
        Walk(Target,Point.bJump);return;
    }
    if(Phase==151||Phase==153||Phase==154)
    {
        if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        // 高い砲口を体力表示の後ろへ隠さず、通常のカメラで少し横と上から見ます。
        PC->SetControlRotation(FRotator(bFieldWaterCannonChecked?5:-8,(Cannon->GetActorLocation()+FVector(0,0,200)-Hero->GetActorLocation()).Rotation().Yaw+(bFieldWaterCannonChecked?18:0),0));
        if(!Cannon->IsBaseVisible()||Health->GetHealth()!=WaterCannonHealthBaseline)
        {Finish(false,TEXT("Water cannon disappeared nearby or observation changed actual health"));return;}
        if(Phase==151)
        {
            if(Cannon->GetShotCount()!=0||Cannon->IsShotActive()){Finish(false,TEXT("First-mission field cannon incorrectly fired"));return;}
            if(Elapsed>3)
            {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWaterCannonField.png"),false,false);bFieldWaterCannonChecked=true;Tap(PC,EKeys::M);Phase=13;Elapsed=0;}
            return;
        }
        // 発射済み回数は読取専用です。弾を生成したり主人公の体力を設定したりしません。
        if(Cannon->IsShotActive()&&Cannon->GetShotCount()>WaterCannonShotBaseline&&FVector::Dist(Cannon->GetShotPosition(),Cannon->GetActorLocation())>650&&!bWaterCannonShotPhoto)
        {bWaterCannonShotPhoto=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWaterCannonFire.png"),false,false);}
        if(Phase==153&&Cannon->GetShotCount()>=WaterCannonShotBaseline+2&&bWaterCannonShotPhoto)
        {bWaterCannonShotsChecked=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWaterCannonAim.png"),false,false);Phase=154;Elapsed=0;return;}
        if(Phase==154&&Elapsed>.7f){Tap(PC,EKeys::R);Phase=155;Elapsed=0;return;}
        if(Elapsed>14)Finish(false,TEXT("Nearby mountain water cannon did not fire two real shots"));return;
    }
    if(Phase==155)
    {
        if(Health->GetHealth()!=8||Cannon->IsShotActive()||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
        {Finish(false,TEXT("Normal R did not reset cannon shot, health and actual entrance"));return;}
        if(Elapsed>.35f)
        {bWaterCannonResetChecked=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderWaterCannonReset.png"),false,false);Phase=9;Elapsed=0;}
    }
}
