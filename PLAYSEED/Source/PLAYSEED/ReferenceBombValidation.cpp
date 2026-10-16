#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceBomb.h"
#include "ReferenceBombField.h"
#include "ReferenceBombWalk.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedCoin.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceBombs(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));auto* Field=Stage?Stage->GetReferenceBombField():nullptr;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Ledger=Mode?Mode->GetCoinLedger():nullptr;
    if(!Hero||!Field||!Ledger||Field->GetFloorFailures()!=0||Field->GetBombs().Num()!=12||GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Bomb validation lost twelve actual-floor actors, real ledger or legitimate earned7"));return;}
    AReferenceBomb* Bomb=Field->GetBombs()[10];AReferenceBomb* Stationary=Field->GetBombs()[11];auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();
    if(!Stationary->IsStationary()||FVector::Dist(Stationary->GetActorLocation(),Stationary->GetHomeLocation())>1){Finish(false,TEXT("Stationary reference bomb moved without a real interaction"));return;}
    const int32 Recovery=Phase==199?2:Phase>=195?1:0;
    if((Phase==194||Phase==199)&&Hero->RecoveryCount==Recovery-1&&Elapsed<2)return;
    if(Phase!=194&&Hero->RecoveryCount!=Recovery){Finish(false,TEXT("Bomb encounter caused unintended recovery"));return;}
    auto Photo=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation")/FString(Name),false,false);};
    auto Begin=[&](int32 Next){Phase=Next;Elapsed=0;BombWaypoint=0;BombLegElapsed=0;BombInputClock=0;};
    auto Walk=[&](FVector Target)
    {
        const FVector D=Target-Hero->GetActorLocation(),Desired=(D.GetSafeNormal2D()*D.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(D.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,D.Rotation().Yaw,0),DeltaSeconds,5));
    };
    if(Phase==190||Phase==195)
    {
        if(BombWaypoint>=UE_ARRAY_COUNT(ReferenceBombApproach)){if(Move->IsMovingOnGround())Move->StopMovementImmediately();Begin(Phase==190?191:196);return;}
        const auto& P=ReferenceBombApproach[BombWaypoint];FVector Target(P.X,P.Y,P.Z);BombLegElapsed+=DeltaSeconds;
        const float Floor=Move->IsMovingOnGround()&&Move->CurrentFloor.bBlockingHit?Move->CurrentFloor.HitResult.ImpactPoint.Z:Hero->GetActorLocation().Z-82;
        if(FVector::Dist2D(Target,Hero->GetActorLocation())<35&&FMath::Abs(Floor-Target.Z)<100){++BombWaypoint;BombLegElapsed=0;return;}
        Walk(Target);if(BombLegElapsed>22)Finish(false,FString::Printf(TEXT("Bomb ordinary route blocked phase%d point%d hero%s target%s"),Phase,BombWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return;
    }
    if(Phase==191||Phase==196)
    {
        if(Health->GetHealth()!=8||Bomb->HasExploded()){Finish(false,TEXT("Ordinary bomb body contact must not damage or explode before pickup"));return;}
        if(Bomb->IsHeldBy(Hero))
        {
            if(Hero->GetCarry()->GetHeldObject()!=Bomb||!Bomb->IsFuseLit()||Bomb->GetChaseStarts()<1||Bomb->GetChaseTravel()<40)
            {Finish(false,TEXT("Normal E did not hold a real lit chasing bomb"));return;}
            bBombChaseChecked=true;Photo(Phase==191?TEXT("ThunderBombHeld.png"):TEXT("ThunderBombHeldFuse.png"));Begin(Phase==191?192:197);return;
        }
        const float D=FVector::Dist2D(Bomb->GetActorLocation(),Hero->GetActorLocation());
        if(D>(Bomb->IsFuseLit()?142:200))Walk(Bomb->GetActorLocation());else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        BombInputClock+=DeltaSeconds;
        if(Bomb->IsFuseLit()&&Bomb->GetChaseTravel()>40&&D<155&&Move->IsMovingOnGround()&&BombInputClock>.4f){Tap(PC,EKeys::E);BombInputClock=0;}
        if(Elapsed>18)Finish(false,FString::Printf(TEXT("Ordinary approach/E did not pick up bomb hero%s bomb%s home%s lit%d fuse%.2f chase%d"),*Hero->GetActorLocation().ToString(),*Bomb->GetActorLocation().ToString(),*Bomb->GetHomeLocation().ToString(),Bomb->IsFuseLit(),Bomb->GetFuseAge(),Bomb->GetChaseStarts()));return;
    }
    if(Phase==192)
    {
        if(Elapsed<.45f)return;
        if(!Bomb->IsHeldBy(Hero)){Finish(false,TEXT("Bomb did not remain held before normal E throw"));return;}
        Tap(PC,EKeys::E);Begin(193);return;
    }
    if(Phase==193)
    {
        if(Bomb->GetThrowCount()!=1){if(Elapsed<1)return;Finish(false,TEXT("Normal E did not throw actual held bomb"));return;}
        if(!Bomb->HasExploded()){if(Elapsed>8)Finish(false,TEXT("Actual thrown bomb failed to land and explode"));return;}
        if(Bomb->GetThrownLandingCount()!=1||Bomb->GetExplosionCount()!=1||Bomb->GetDropGenerationCount()!=1||Health->GetHealth()!=8)
        {Finish(false,TEXT("Thrown bomb did not explode once on physics landing away from hero"));return;}
        auto* Coin=Bomb->GetDropCoin();if(!Coin){Finish(false,TEXT("Actual thrown bomb lost its one gold"));return;}
        if(Coin->IsCollected())
        {
            if(Ledger->GetTotal()<1){Finish(false,TEXT("Bomb gold was not counted by real ledger"));return;}
            bBombThrowChecked=true;Photo(TEXT("ThunderBombThrowCoin.png"));Tap(PC,EKeys::R);Begin(194);return;
        }
        Walk(Coin->GetActorLocation());if(Elapsed>15)Finish(false,TEXT("Ordinary movement did not collect thrown bomb gold"));return;
    }
    if(Phase==197)
    {
        if(Bomb->HasExploded())
        {
            if(Bomb->IsHeld()||Hero->GetCarry()->GetHeldObject()==Bomb){Finish(false,TEXT("Expired real held fuse did not drop bomb"));return;}
            if(Health->GetHealth()==8&&Elapsed<7)return;
            if(Health->GetHealth()!=6||Bomb->GetExplosionDamageCount()!=1||Health->GetLastImpulse().Size2D()<400||Bomb->GetExplosionCount()!=2||Bomb->GetDropGenerationCount()!=2)
            {Finish(false,TEXT("Real held fuse expiry must cause exactly two damage, knockback and one gold"));return;}
            bBombFuseChecked=true;Photo(TEXT("ThunderBombExplosion.png"));Begin(198);return;
        }
        if(!Bomb->IsHeldBy(Hero)){if(Elapsed<.5f)return;Finish(false,TEXT("Real held bomb disappeared before its fuse expired"));return;}
        if(Elapsed>2&&!bBombHeldPhoto){bBombHeldPhoto=true;Photo(TEXT("ThunderBombFuseLit.png"));}
        if(Elapsed>7)Finish(false,TEXT("Real five-second fuse did not expire"));return;
    }
    if(Phase==198)
    {
        auto* Coin=Bomb->GetDropCoin();if(!Coin||Bomb->GetDropGenerationCount()!=2){Finish(false,TEXT("Held explosion duplicated or lost actual one gold"));return;}
        if(Coin->IsCollected())
        {if(Ledger->GetTotal()<1){Finish(false,TEXT("Real fuse gold missing from ledger"));return;}Photo(TEXT("ThunderBombFuseCoin.png"));Tap(PC,EKeys::R);Begin(199);return;}
        Walk(Coin->GetActorLocation());if(Elapsed>10)Finish(false,TEXT("Ordinary movement did not collect held explosion gold"));return;
    }
    if(Phase==194||Phase==199)
    {
        const int32 Expected=Phase==194?1:2;if(Hero->RecoveryCount!=Expected){if(Elapsed<1)return;Finish(false,TEXT("Normal R failed after bomb encounter"));return;}
        bool Good=Health->GetHealth()==8&&Ledger->GetTotal()==0&&FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))<40&&Hero->GetCarry()->GetHeldObject()==nullptr;
        for(AReferenceBomb* Actor:Field->GetBombs())Good=Good&&!Actor->HasExploded()&&!Actor->IsFuseLit()&&!Actor->IsHeld()&&!Actor->GetDropCoin()&&FVector::Dist2D(Actor->GetActorLocation(),Actor->GetHomeLocation())<220;
        if(!Good){Finish(false,TEXT("Normal R did not restore all twelve bombs, health8, ledger0 and own drops"));return;}
        if(Elapsed<.4f)return;
        Photo(Phase==194?TEXT("ThunderBombFirstReset.png"):TEXT("ThunderBombReset.png"));if(Phase==194)Begin(195);else{bBombResetChecked=true;Begin(9);}return;
    }
}
