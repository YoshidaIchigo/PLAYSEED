#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWalker.h"
#include "WindseedCharacter.h"
#include "PlayseedCoin.h"
#include "PlayseedVitalityComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

// 検証経路で実際に近づいた敵だけに、普通の歩行と攻撃で対処します。
// 敵・主人公・体力・コイン・進行を検証から設定しません。
bool UReferencePaintingTestSubsystem::UpdateReferenceRouteCombat(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Move=Hero->GetCharacterMovement();
    if(!RouteCombatTarget.IsValid()&&Move->IsMovingOnGround())
    {
        float Nearest=600;
        for(TActorIterator<AReferenceWalker> It(GetWorld());It;++It)
        {
            const float Distance=FVector::Dist2D(Hero->GetActorLocation(),It->GetActorLocation());
            if(!It->IsDefeated()&&Distance<Nearest&&FMath::Abs(Hero->GetActorLocation().Z-82-It->GetActorLocation().Z)<80)
            {Nearest=Distance;RouteCombatTarget=*It;}
        }
        if(RouteCombatTarget.IsValid()){RouteCombatElapsed=RouteAttackClock=0;UE_LOG(LogTemp,Display,TEXT("ROUTE COMBAT: ordinary approach to%s phase%d"),*RouteCombatTarget->GetActorLocation().ToString(),Phase);}
    }
    if(!RouteCombatTarget.IsValid())return false;
    AReferenceWalker* Enemy=RouteCombatTarget.Get();RouteCombatElapsed+=DeltaSeconds;RouteAttackClock+=DeltaSeconds;
    FVector Target=Enemy->GetActorLocation();
    if(Enemy->IsDefeated())
    {
        APlayseedCoin* Coin=Enemy->GetDropCoin();
        if(!Coin||Enemy->GetDropGenerationCount()!=1){Finish(false,TEXT("Normal route combat lost its actual enemy gold"));return true;}
        if(Coin->IsCollected())
        {UE_LOG(LogTemp,Display,TEXT("ROUTE COMBAT: actual normal defeat and own gold collected health%d"),Hero->GetVitality()->GetHealth());RouteCombatTarget.Reset();return true;}
        Target=FVector(Coin->GetActorLocation().X,Coin->GetActorLocation().Y,Enemy->GetActorLocation().Z);
    }
    const FVector Delta=Target-Hero->GetActorLocation();const float Distance=Delta.Size2D();
    const bool bFacing=FVector::DotProduct(Hero->GetActorForwardVector(),Delta.GetSafeNormal2D())>.85f;
    if(Enemy->IsDefeated()||Distance>173)
    {
        const FVector Desired=(Delta.GetSafeNormal2D()*Distance*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(Distance>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
    }
    else if(!bFacing)Hero->AddMovementInput(Delta.GetSafeNormal2D(),.25f);
    else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
    if(!Enemy->IsDefeated()&&Distance<180&&bFacing&&RouteAttackClock>.28f){Tap(PC,EKeys::LeftMouseButton);RouteAttackClock=0;}
    if(RouteCombatElapsed>15)Finish(false,FString::Printf(TEXT("Ordinary route combat blocked hero%s enemy%s health%d distance%.1f defeated%d"),*Hero->GetActorLocation().ToString(),*Enemy->GetActorLocation().ToString(),Hero->GetVitality()->GetHealth(),Distance,Enemy->IsDefeated()));
    return true;
}
