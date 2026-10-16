#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceWalker.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ReferenceBoulderField.h"
#include "ThunderRollingBoulder.h"
#include "ThunderMeadowStage.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
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
    // 近づきすぎてから止まると、実攻撃の命中時間より先に接触されます。
    if(Enemy->IsDefeated()||Distance>220)
    {
        const FVector Desired=(Delta.GetSafeNormal2D()*Distance*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        if(Distance>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
    }
    else if(!bFacing)Hero->AddMovementInput(Delta.GetSafeNormal2D(),.25f);
    else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
    if(!Enemy->IsDefeated()&&Distance<240&&bFacing&&RouteAttackClock>.28f){Tap(PC,EKeys::LeftMouseButton);RouteAttackClock=0;}
    if(RouteCombatElapsed>15)Finish(false,FString::Printf(TEXT("Ordinary route combat blocked hero%s enemy%s health%d distance%.1f defeated%d"),*Hero->GetActorLocation().ToString(),*Enemy->GetActorLocation().ToString(),Hero->GetVitality()->GetHealth(),Distance,Enemy->IsDefeated()));
    return true;
}

// 実鉄球と実床を読み、通常の横移動/Spaceだけを送ります。
bool UReferencePaintingTestSubsystem::UpdateReferenceRoadAvoidance(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    if(!Stage)return false;auto* Movement=Hero->GetCharacterMovement();
        if(BossAvoidRemaining<=0&&Movement->IsMovingOnGround())
        {
            auto* Field=Stage->GetReferenceBoulderField();auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
            if(Field&&Terrain)for(int32 Index=0;Index<FMath::Min(2,Field->GetBalls().Num());++Index)
            {
                AThunderRollingBoulder* Ball=Field->GetBalls()[Index]; // 山道の2体を避けます。くぼ地の周回を追いかけません。
                const FVector D=Ball->GetActorLocation()-Hero->GetActorLocation();const float Feet=Hero->GetActorLocation().Z-82;
                if(Ball->IsWaiting()||D.Size2D()>800||FMath::Abs(Ball->GetActorLocation().Z-Feet-100)>350)continue;
                const FVector Side=FVector(D.Y,-D.X,0).GetSafeNormal();float Best=99999;FVector Choice=FVector::ZeroVector;
                for(float Distance:{150.f,100.f})for(float Sign:{-1.f,1.f})
                {
                    const FVector Start=Hero->GetActorLocation()-FVector(0,0,82);FVector Floor;
                    if(!Terrain->FindFloor(Start+Side*Sign*Distance,Floor)||FMath::Abs(Floor.Z-Feet)>70)continue;
                    // 点だけに床があっても、途中や着地の外周が崖なら横へ避けません。
                    bool bSafe=true;
                    for(int32 Step=1;Step<=5&&bSafe;++Step)for(int32 Edge=0;Edge<8;++Edge)
                    {
                        const float Angle=Edge*(2*PI/8);const FVector Probe=FMath::Lerp(Start,Floor,Step/5.f)+FVector(FMath::Cos(Angle)*70,FMath::Sin(Angle)*70,0);
                        FVector Support;if(!Terrain->FindFloor(Probe,Support)||FMath::Abs(Support.Z-Probe.Z)>70){bSafe=false;break;}
                    }
                    const float Cost=FMath::Abs(Floor.Z-Feet)+(150-Distance);
                    if(bSafe&&Cost<Best){Best=Cost;Choice=Floor;}
                }
                if(Best<99999){BossAvoidTarget=Choice;BossAvoidRemaining=1.2f;bBossAvoidJumped=false;BossLegElapsed=CannonLegElapsed=RedLegElapsed=0;UE_LOG(LogTemp,Display,TEXT("ROUTE QA: ordinary side-step/jump from real ball at%s tofloor%s health%d"),*Ball->GetActorLocation().ToString(),*Choice.ToString(),Hero->GetVitality()->GetHealth());break;}
            }
        }
        if(BossAvoidRemaining>0)
        {
            BossAvoidRemaining-=DeltaSeconds;
            const FVector D=BossAvoidTarget-Hero->GetActorLocation(),Desired=(D.GetSafeNormal2D()*D.Size2D()*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            if(D.Size2D()>8)Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));else if(Movement->IsMovingOnGround())Movement->StopMovementImmediately();
            PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,D.Rotation().Yaw,0),DeltaSeconds,5));
            if(Movement->IsMovingOnGround()&&JumpHold<=0&&D.Size2D()<350&&BossAvoidTarget.Z>Hero->GetActorLocation().Z-82+38)
            {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
            if(!bBossAvoidJumped&&Movement->IsMovingOnGround()&&JumpHold<=0){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bBossAvoidJumped=true;}return true;
        }
    return false;
}
