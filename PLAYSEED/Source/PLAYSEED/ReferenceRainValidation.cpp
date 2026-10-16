#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceRainSpawner.h"
#include "ReferenceRainWalk.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderMeadowStage.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceRain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Hero||!Stage||!Terrain||Stage->GetReferenceRainSpawners().Num()!=3||Progress->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Reference rain lost its three real spawners, terrain or legitimate earned7"));return;}
    const auto& Rains=Stage->GetReferenceRainSpawners();AReferenceRainSpawner* Rain=Rains[RainChosenIndex];
    int32 TotalHits=0;for(AReferenceRainSpawner* Spawner:Rains)TotalHits+=Spawner->GetConfirmedHits();
    if(Rains[0]->GetRange()!=950||Rains[1]->GetRange()!=1450||Rains[2]->GetRange()!=700)
    {Finish(false,TEXT("Actual reference rain ranges differed from source15/25/10 parameters"));return;}
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
    if(Phase==140||Phase==143)
    {
        const bool bFirst=Phase==140;
        if(Hero->RecoveryCount!=(bFirst?0:1)){Finish(false,TEXT("Rain approach recovered without the expected ordinary R input"));return;}
        if(bFirst&&Health->GetHealth()==7&&TotalHits==1)
        {
            for(int32 I=0;I<Rains.Num();++I)if(Rains[I]->GetConfirmedHits()==1){RainChosenIndex=I;Rain=Rains[I];break;}
            if(Health->GetLastImpulse().Size2D()<400){Finish(false,TEXT("Actual one-point rain impact did not physically knock back"));return;}
            UE_LOG(LogTemp,Display,TEXT("RAIN QA: actual source%d contact, health7 and physical knockback"),RainChosenIndex);
            bRainContactVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRainContact.png"),false,false);Phase=141;Elapsed=0;return;
        }
        if(Health->GetHealth()!=8)
        {Finish(false,FString::Printf(TEXT("Unintended rain approach damage phase%d point%d health%d rainhits%d/%d/%d hero%s"),Phase,RainWaypoint,Health->GetHealth(),Rains[0]->GetConfirmedHits(),Rain->GetConfirmedHits(),Rains[2]->GetConfirmedHits(),*Hero->GetActorLocation().ToString()));return;}
        if(!bFirst&&Move->IsMovingOnGround()&&Rain->GetSpawnCount()>RainSpawnBaseline&&Rain->IsFalling()&&Rain->GetRainPosition().Z-Hero->GetActorLocation().Z>1000)
        {
            const FVector Feet=Hero->GetActorLocation()-FVector(0,0,82);
            FVector Away=(Hero->GetActorLocation()-Rain->GetRainTarget()).GetSafeNormal2D();if(Away.IsNearlyZero())Away=-Hero->GetActorForwardVector();
            const FVector2D Directions[]={FVector2D(1,0),FVector2D(-1,0),FVector2D(0,1),FVector2D(0,-1),FVector2D(1,1),FVector2D(1,-1),FVector2D(-1,1),FVector2D(-1,-1)};
            float Best=.1f;bool bFound=false;
            FCollisionQueryParams Query(SCENE_QUERY_STAT(RainDodgePlanning),false,Hero);
            for(const FVector2D& Direction:Directions)
            {
                const FVector Unit=FVector(Direction.X,Direction.Y,0).GetSafeNormal2D();FVector Floor;
                if(!Terrain->FindFloor(Feet+Unit*650,Floor)||FMath::Abs(Floor.Z-Feet.Z)>80)continue;
                bool bOtherRain=false;
                for(int32 I=0;I<Rains.Num();++I)if(I!=RainChosenIndex&&FVector::Dist2D(Floor,Rains[I]->GetActorLocation())<Rains[I]->GetRange()+100)bOtherRain=true;
                if(bOtherRain)continue;
                const float Score=FVector::DotProduct(Unit,Away);if(Score<=Best)continue;
                // 読み取りの衝突調査だけです。移動は実カプセルの通常入力で行います。
                if(GetWorld()->SweepTestByChannel(Hero->GetActorLocation(),Floor+FVector(0,0,82),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(34,70),Query))continue;
                Best=Score;RainDodgeTarget=Floor;bFound=true;
            }
            if(!bFound){Finish(false,TEXT("No ordinary clear floor path away from the real rain shadow"));return;}
            RainBurstBaseline=Rain->GetBurstCount();FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRainWarning.png"),false,false);
            UE_LOG(LogTemp,Display,TEXT("RAIN QA: ordinary dodge from%s shadow%s toward actual floor%s spawn%d"),*Hero->GetActorLocation().ToString(),*Rain->GetRainTarget().ToString(),*RainDodgeTarget.ToString(),Rain->GetSpawnCount());Phase=144;Elapsed=0;return;
        }
        const int32 Count=UE_ARRAY_COUNT(ReferenceRainApproach);
        if(RainWaypoint>=Count)
        {
            const auto& Last=ReferenceRainApproach[Count-1];
            Walk(bFirst&&Rain->IsActive()?Rain->GetRainTarget():FVector(Last.X,Last.Y,Last.Z));
            if(Elapsed>240)Finish(false,TEXT("Ordinary rain approach did not encounter the real drop"));return;
        }
        const auto& Point=ReferenceRainApproach[RainWaypoint];const FVector Target(Point.X,Point.Y,Point.Z);const FVector Delta=Target-Hero->GetActorLocation();RainLegElapsed+=DeltaSeconds;
        if(RainLegElapsed>22){Finish(false,FString::Printf(TEXT("Rain walk blocked phase%d point%d hero%s target%s floor%s"),Phase,RainWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString()));return;}
        if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<(Move->IsMovingOnGround()?160:80))
        {if(RainWaypoint%10==0)UE_LOG(LogTemp,Display,TEXT("RAIN WALK phase%d point%d hero%s"),Phase,RainWaypoint,*Hero->GetActorLocation().ToString());++RainWaypoint;RainLegElapsed=0;return;}
        Walk(Target);return;
    }
    if(Phase==141)
    {
        if(Elapsed<.85f)return;
        if(Health->GetHealth()!=7||TotalHits!=1){Finish(false,TEXT("One real rain drop damaged more than once"));return;}
        Tap(PC,EKeys::R);Phase=142;Elapsed=0;return;
    }
    if(Phase==142||Phase==146)
    {
        const bool bFirst=Phase==142;const int32 Recovery=bFirst?1:2;
        if(Hero->RecoveryCount==Recovery-1&&Elapsed<2)return;
        if(Hero->RecoveryCount!=Recovery||Health->GetHealth()!=8||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)
        {Finish(false,TEXT("Ordinary R did not restore actual rain-test hero and course entrance"));return;}
        for(AReferenceRainSpawner* Spawner:Rains)if(!Spawner->IsIdle()){Finish(false,TEXT("R did not clear every own rain drop"));return;}
        if(Elapsed<.35f)return;
        if(bFirst){RainWaypoint=0;RainLegElapsed=0;RainSpawnBaseline=Rain->GetSpawnCount();Phase=143;Elapsed=0;return;}
        bRainResetVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRainReset.png"),false,false);Tap(PC,EKeys::M);Phase=9;Elapsed=0;return;
    }
    if(Phase==144)
    {
        if(Hero->RecoveryCount!=1||Health->GetHealth()!=8||TotalHits!=1)
        {Finish(false,TEXT("Ordinary movement did not avoid the next real rain drop"));return;}
        Walk(RainDodgeTarget);
        // 普通に離れながらカメラで振り返り、水玉の跳ね返りと影も映します。
        PC->SetControlRotation(FRotator(-18,(Rain->GetRainPosition()-Hero->GetActorLocation()).Rotation().Yaw+20,0));
        if(!bRainBounceShot&&Rain->IsFalling()&&Rain->GetBouncePeak()>120)
        {bRainBounceShot=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRainBounce.png"),false,false);}
        if(Rain->GetBurstCount()>RainBurstBaseline&&FVector::Dist2D(Hero->GetActorLocation(),Rain->GetRainTarget())>250)
        {
            if(Rain->GetBouncePeak()<120){Finish(false,TEXT("Actual water did not physically rise after its floor bounce"));return;}
            bRainDodgeVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRainAvoided.png"),false,false);
            UE_LOG(LogTemp,Display,TEXT("RAIN QA: actual burst%d after ordinary dodge, hits unchanged%d health%d"),Rain->GetBurstCount(),Rain->GetConfirmedHits(),Health->GetHealth());Phase=145;Elapsed=0;return;
        }
        if(Elapsed>10)Finish(false,TEXT("Real rain did not burst after the ordinary dodge"));return;
    }
    if(Phase==145)
    {
        if(Health->GetHealth()!=8||TotalHits!=1){Finish(false,TEXT("Another rain drop damaged the hero after the ordinary dodge"));return;}
        if(Elapsed>.8f){Tap(PC,EKeys::R);Phase=146;Elapsed=0;}
    }
}
