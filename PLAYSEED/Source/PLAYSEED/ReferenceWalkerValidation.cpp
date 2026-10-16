#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWalker.h"
#include "ReferenceWalkerField.h"
#include "ReferenceWalkerWalk.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
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

void UReferencePaintingTestSubsystem::UpdateReferenceWalkers(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));auto* Field=Stage?Stage->GetReferenceWalkerField():nullptr;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Ledger=Mode?Mode->GetCoinLedger():nullptr;
    if(!Hero||!Field||!Ledger||Field->GetFloorFailures()!=0||Field->GetWalkers().Num()!=11||GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount()!=7)
    {Finish(false,TEXT("Walker validation lost eleven actual-floor enemies, ledger or legitimate earned7"));return;}
    const auto& Walkers=Field->GetWalkers();AReferenceWalker* First=Walkers[0];AReferenceWalker* Second=Walkers[1];
    auto* Move=Hero->GetCharacterMovement();auto* Health=Hero->GetVitality();
    const int32 Recovery=Phase==188?2:Phase>=183?1:0;
    if((Phase==188||Phase==189)&&Hero->RecoveryCount==Recovery-1&&Elapsed<2)return;
    if(Hero->RecoveryCount!=Recovery){Finish(false,TEXT("Ordinary walker encounter caused unintended recovery"));return;}
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Photo=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation")/FString(Name),false,false);};
    auto Begin=[&](int32 Next){Phase=Next;Elapsed=0;WalkerWaypoint=0;WalkerLegElapsed=0;WalkerAttackClock=0;};
    auto Walk=[&](FVector Target,bool bJump=false)
    {
        const FVector D=Target-Hero->GetActorLocation(),Desired=(D.GetSafeNormal2D()*D.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(D.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,D.Rotation().Yaw,0),DeltaSeconds,5));
        if(bJump&&Move->IsMovingOnGround()&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    auto Route=[&](const FReferenceWalkPoint* Points,int32 Count)
    {
        if(WalkerWaypoint>=Count){if(Move->IsMovingOnGround())Move->StopMovementImmediately();return true;}
        const auto& P=Points[WalkerWaypoint];const FVector Target(P.X,P.Y,P.Z);WalkerLegElapsed+=DeltaSeconds;
        if(WalkerLegElapsed>22){Finish(false,FString::Printf(TEXT("Walker ordinary route blocked phase%d point%d hero%s target%s"),Phase,WalkerWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString()));return false;}
        if(FVector::Dist2D(Hero->GetActorLocation(),Target)<35&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<160)
        {++WalkerWaypoint;WalkerLegElapsed=0;return false;}
        Walk(Target,Target.Z>Hero->GetActorLocation().Z-82+60&&FVector::Dist2D(Target,Hero->GetActorLocation())<350);return false;
    };
    auto AllReset=[&]()
    {
        if(Health->GetHealth()!=8||Ledger->GetTotal()!=0||FVector::Dist2D(Hero->GetActorLocation(),FVector(-6558,-6464,0))>40)return false;
        for(AReferenceWalker* Enemy:Walkers)if(Enemy->IsDefeated()||Enemy->GetMeleeHealth()!=1||Enemy->GetDropCoin()||FVector::Dist(Enemy->GetActorLocation(),Enemy->GetHomeLocation())>1)return false;
        return true;
    };
    if(Phase==180||Phase==183)
    {if(Route(ReferenceWalkerFirstWalk,UE_ARRAY_COUNT(ReferenceWalkerFirstWalk)))Begin(Phase==180?181:184);return;}
    if(Phase==181)
    {
        if(Health->GetHealth()==7)
        {
            if(First->IsDefeated()||First->GetChaseStarts()<1||FVector::Dist2D(First->GetActorLocation(),First->GetHomeLocation())<80||Health->GetLastImpulse().Size2D()<400)
            {Finish(false,TEXT("One-point walker contact did not follow real chase and physical knockback"));return;}
            bWalkerContactChecked=true;Photo(TEXT("ThunderWalkerContact.png"));Begin(182);return;
        }
        if(Health->GetHealth()!=8||First->IsDefeated()){Finish(false,TEXT("Walker contact did not cause exactly one real damage"));return;}
        Walk(First->GetActorLocation());if(Elapsed>12)Finish(false,TEXT("Ordinary approach did not contact the real chasing walker"));return;
    }
    if(Phase==182){if(Elapsed>.6f){Tap(PC,EKeys::R);Begin(189);}return;}
    if(Phase==188||Phase==189)
    {
        if(!AllReset()){Finish(false,TEXT("Ordinary R did not restore eleven actual enemies, health8, score0, entrance and remove own drops"));return;}
        if(Elapsed<.35f)return;
        if(Phase==189){Photo(TEXT("ThunderWalkerFirstReset.png"));Begin(183);}
        else{bWalkerResetChecked=true;Photo(TEXT("ThunderWalkerReset.png"));Begin(9);}return;
    }
    if(Phase==184)
    {
        if(First->IsDefeated())
        {
            if(First->GetReceivedStrikes()!=1||First->GetSuccessfulStomps()!=0||First->GetDropGenerationCount()!=1||!First->GetDropCoin())
            {Finish(false,TEXT("Normal LMB did not defeat one walker and generate exactly one real gold"));return;}
            WalkerScoreBaseline=Ledger->GetTotal();Photo(TEXT("ThunderWalkerMelee.png"));Begin(185);return;
        }
        const float Distance=FVector::Dist2D(Hero->GetActorLocation(),First->GetActorLocation());
        if(Distance>173)Walk(First->GetActorLocation());else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        WalkerAttackClock+=DeltaSeconds;
        if(Distance<180&&WalkerAttackClock>.28f){Tap(PC,EKeys::LeftMouseButton);WalkerAttackClock=0;}
        if(Elapsed>12)Finish(false,TEXT("Ordinary LMB did not strike the actual walker"));return;
    }
    if(Phase==185||Phase==187)
    {
        AReferenceWalker* Enemy=Phase==185?First:Second;auto* Coin=Enemy->GetDropCoin();
        if(!Coin||Enemy->GetDropGenerationCount()!=1){Finish(false,TEXT("Defeated walker lost or duplicated its own gold drop"));return;}
        if(Coin->IsCollected())
        {
            if(Ledger->GetTotal()<WalkerScoreBaseline+1){Finish(false,TEXT("Actual gold pickup did not enter the real coin ledger"));return;}
            if(Phase==185){bWalkerMeleeChecked=true;WalkerScoreBaseline=Ledger->GetTotal();Photo(TEXT("ThunderWalkerMeleeCoin.png"));Begin(186);}
            else{bWalkerStompChecked=true;Photo(TEXT("ThunderWalkerStompCoin.png"));Tap(PC,EKeys::R);Begin(188);}return;
        }
        const FVector Position=Coin->GetActorLocation();Walk(FVector(Position.X,Position.Y,Enemy->GetActorLocation().Z));
        if(Elapsed>8)Finish(false,TEXT("Ordinary movement did not physically collect the walker gold"));return;
    }
    if(Phase==186)
    {
        if(Second->IsDefeated())
        {
            if(Second->GetSuccessfulStomps()!=1||Second->GetReceivedStrikes()!=0||Second->GetDropGenerationCount()!=1)
            {Finish(false,TEXT("Ordinary Space jump did not stomp one walker and generate one real gold"));return;}
            Photo(TEXT("ThunderWalkerStomp.png"));Begin(187);return;
        }
        if(WalkerWaypoint<UE_ARRAY_COUNT(ReferenceWalkerSecondWalk)){Route(ReferenceWalkerSecondWalk,UE_ARRAY_COUNT(ReferenceWalkerSecondWalk));return;}
        const float Distance=FVector::Dist2D(Hero->GetActorLocation(),Second->GetActorLocation());
        const bool bJump=!bWalkerStompJump&&Move->IsMovingOnGround()&&Distance<300;
        Walk(Second->GetActorLocation(),bJump);if(bJump)bWalkerStompJump=true;
        if(Elapsed>35)Finish(false,TEXT("Normal Space did not stomp the second actual walker"));
    }
}
