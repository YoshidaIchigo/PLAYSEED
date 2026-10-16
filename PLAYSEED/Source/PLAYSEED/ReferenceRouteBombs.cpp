#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceBomb.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"

// 近づく実爆弾を普通のLMBで蹴り、実着地からの爆発を離れて待ちます。
// 物理速度・導火線・体力・敵の位置は製品だけが変更します。
bool UReferencePaintingTestSubsystem::UpdateReferenceRouteBombs(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Move=Hero->GetCharacterMovement();
    if(!RouteBombTarget.IsValid()&&Move->IsMovingOnGround())
    {
        float Nearest=600;
        for(TActorIterator<AReferenceBomb> It(GetWorld());It;++It)
        {
            const float Distance=FVector::Dist2D(Hero->GetActorLocation(),It->GetActorLocation());
            if(!It->HasExploded()&&Distance<Nearest&&FMath::Abs(Hero->GetActorLocation().Z-82-(It->GetActorLocation().Z-65))<100)
            {Nearest=Distance;RouteBombTarget=*It;}
        }
        if(RouteBombTarget.IsValid())
        {
            // 通常R後も通算カウンターは残るため、この挑戦の実生成だけを数えます。
            RouteBombExpectedDrops=RouteBombTarget->GetDropGenerationCount()+(RouteBombTarget->GetDropCoin()?0:1);
            RouteBombElapsed=RouteBombAttackClock=RouteBombAfterBurst=0;bRouteBombKicked=bRouteBombJumped=false;UE_LOG(LogTemp,Display,TEXT("ROUTE BOMB: ordinary LMB approach%s phase%d"),*RouteBombTarget->GetActorLocation().ToString(),Phase);
        }
    }
    if(!RouteBombTarget.IsValid())return false;
    AReferenceBomb* Bomb=RouteBombTarget.Get();RouteBombElapsed+=DeltaSeconds;RouteBombAttackClock+=DeltaSeconds;
    if(Bomb->GetBody()->IsSimulatingPhysics())bRouteBombKicked=true;
    if(Bomb->HasExploded())
    {
        if(Move->IsMovingOnGround())Move->StopMovementImmediately();RouteBombAfterBurst+=DeltaSeconds;
        if(RouteBombAfterBurst>.4f)
        {
            if(Bomb->GetDropGenerationCount()!=RouteBombExpectedDrops||!Bomb->GetDropCoin()){Finish(false,FString::Printf(TEXT("Actual bomb burst gold count%d expected%d coin%d"),Bomb->GetDropGenerationCount(),RouteBombExpectedDrops,Bomb->GetDropCoin()!=nullptr));return true;}
            UE_LOG(LogTemp,Display,TEXT("ROUTE BOMB: actual burst after normal input, distance%.1f health%d"),FVector::Dist2D(Hero->GetActorLocation(),Bomb->GetActorLocation()),Hero->GetVitality()->GetHealth());RouteBombTarget.Reset();
        }
        return true;
    }
    if(bRouteBombKicked)
    {if(Move->IsMovingOnGround())Move->StopMovementImmediately();}
    else
    {
        const FVector Delta=Bomb->GetActorLocation()-Hero->GetActorLocation();const float Distance=Delta.Size2D();
        const bool bFacing=FVector::DotProduct(Hero->GetActorForwardVector(),Delta.GetSafeNormal2D())>.85f;
        const bool bClosing=Bomb->IsFuseLit()&&Bomb->GetChaseStarts()>0;
        const float StopDistance=bClosing?220.f:160.f;
        if(Distance>StopDistance)
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Distance*4.5f).GetClampedToMaxSize2D(470),Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else if(!bFacing)Hero->AddMovementInput(Delta.GetSafeNormal2D(),.25f);
        else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Delta.Rotation().Yaw,0),DeltaSeconds,4));
        if(Distance<(bClosing?240.f:175.f)&&bFacing&&RouteBombAttackClock>.28f){Tap(PC,EKeys::LeftMouseButton);RouteBombAttackClock=0;}
        // 普通のジャンプで、導火線の終わりに足元を爆発範囲の上へ出します。
        if(!bRouteBombJumped&&Bomb->GetFuseAge()>4.7f&&Move->IsMovingOnGround()&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;bRouteBombJumped=true;}
    }
    if(RouteBombElapsed>9)Finish(false,FString::Printf(TEXT("Normal LMB bomb response blocked hero%s bomb%s fuse%.2f kicked%d"),*Hero->GetActorLocation().ToString(),*Bomb->GetActorLocation().ToString(),Bomb->GetFuseAge(),bRouteBombKicked));
    return true;
}
