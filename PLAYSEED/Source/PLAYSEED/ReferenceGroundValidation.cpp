#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceGroundWalk.h"
#include "ReferenceCoinField.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "PlayseedCoin.h"
#include "ReferenceHero.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceGround(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    if(!Hero||Hero->RecoveryCount!=(Phase==115?1:0)){Finish(false,TEXT("Ground coin walking recovered unexpectedly"));return;}
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Field=Stage?Stage->GetReferenceCoinField():nullptr;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Ledger=Mode?Mode->GetCoinLedger():nullptr;
    if(!Field||!Ledger||Field->GetCoins().Num()!=23||Field->GetPosts().Num()!=4||!Field->GetCoinBox())
    {Finish(false,TEXT("Reference23 golds, four posts or the three-coin box were missing"));return;}
    auto* Move=Hero->GetCharacterMovement();auto* Box=Field->GetCoinBox();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    auto Walk=[&](FVector Target)
    {
        const FVector Delta=Target-Hero->GetActorLocation();const float Distance=Delta.Size2D();
        if(Distance>8)
        {
            const FVector Desired=(Delta.GetSafeNormal2D()*Distance*4.5f).GetClampedToMaxSize2D(470);
            const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
            Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
        }
        else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
        if(Move->IsMovingOnGround()&&Distance<350&&Target.Z>Hero->GetActorLocation().Z-82+38&&JumpHold<=0)
        {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
    };
    auto Sprint=[&](bool bPressed){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,bPressed?IE_Pressed:IE_Released,bPressed?1:0));};
    if(Phase==110)
    {
        if(GroundWaypoint>=UE_ARRAY_COUNT(ReferenceGroundWalk))
        {
            int32 Collected=0;for(int32 I=0;I<13;++I)if(Field->GetCoins()[I]->IsCollected())++Collected;
            for(AReferenceCoinPost* Post:Field->GetPosts())
            {
                if(Post->GetGenerationCount()!=1||Post->GetDropCoins().Num()!=5){Finish(false,TEXT("A post did not generate exactly five actual coins once"));return;}
                for(const auto& Coin:Post->GetDropCoins())if(Coin.IsValid()&&Coin->IsCollected())++Collected;
            }
            for(const auto& Coin:Box->GetDropCoins())if(Coin.IsValid()&&Coin->IsCollected())++Collected;
            if(Collected!=36||Ledger->GetTotal()<36||Ledger->GetTotal()>=100||Box->GetBreakCount()!=1||Box->GetDropGenerationCount()!=1)
            {Finish(false,FString::Printf(TEXT("Ordinary field pickups incomplete: real gold%d score%d"),Collected,Ledger->GetTotal()));return;}
            bGroundCoinsVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderGroundCoins.png"),false,false);
            UE_LOG(LogTemp,Display,TEXT("GROUND QA: actual36 field golds collected; score%d, no star awarded; testing real R reset next"),Ledger->GetTotal());
            Tap(PC,EKeys::R);Phase=115;Elapsed=0;return;
        }
        const auto& Point=ReferenceGroundWalk[GroundWaypoint];const FVector Target(Point.X,Point.Y,Point.Z);
        GroundLegElapsed+=DeltaSeconds;
        if(GroundLegElapsed>22){Finish(false,FString::Printf(TEXT("Ground route blocked point%d hero%s target%s grounded%d floor%s normal%s"),GroundWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),Move->IsMovingOnGround(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString(),*Move->CurrentFloor.HitResult.ImpactNormal.ToString()));return;}
        // 斜面ではCapsuleが中心点より高い隣接面に支えられます。実接地を条件に経由点だけの差を許容します。
        const float HeightTolerance=Move->IsMovingOnGround()?160.f:80.f;
        if(FVector::Dist2D(Hero->GetActorLocation(),Target)<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<HeightTolerance)
        {
            if(Point.Event>=100&&!Field->GetCoins()[Point.Event-100]->IsCollected())
            {Walk(Field->GetCoins()[Point.Event-100]->GetActorLocation());return;}
            ++GroundWaypoint;GroundLegElapsed=0;
            if(Point.Event==1){Phase=111;Elapsed=0;}
            else if(Point.Event>=10&&Point.Event<14){GroundPost=Point.Event-10;GroundCircleAttempts=0;Sprint(true);Phase=113;Elapsed=0;}
            return;
        }
        Walk(Target);return;
    }
    if(Phase==111)
    {
        if(Box->IsBroken())
        {if(Box->GetBreakCount()!=1||Box->GetDropGenerationCount()!=1||Box->GetDropCoins().Num()!=3){Finish(false,TEXT("Real wooden box duplicated its three coins"));return;}GroundDrop=0;Phase=112;Elapsed=0;return;}
        const FVector Delta=Box->GetActorLocation()-Hero->GetActorLocation();
        // 実移動で箱へ向きを合わせ、地上の普通の攻撃キーだけで壊します。
        if(Delta.Size2D()>160||FVector::DotProduct(Hero->GetActorForwardVector(),Delta.GetSafeNormal2D())<.9f)Hero->AddMovementInput(Delta.GetSafeNormal2D(),.25f);
        else if(Move->IsMovingOnGround()){Move->StopMovementImmediately();Tap(PC,EKeys::LeftMouseButton);}
        if(Elapsed>8)Finish(false,TEXT("Ordinary ground punch did not break the source three-coin box"));return;
    }
    if(Phase==112||Phase==114)
    {
        const auto& Drops=Phase==112?Box->GetDropCoins():Field->GetPosts()[GroundPost]->GetDropCoins();
        if(GroundDrop>=Drops.Num())
        {Move->StopMovementImmediately();Phase=110;Elapsed=0;GroundLegElapsed=0;return;}
        if(!Drops[GroundDrop].IsValid()){Finish(false,TEXT("Actual dropped coin disappeared before pickup"));return;}
        if(Drops[GroundDrop]->IsCollected()){++GroundDrop;Elapsed=0;return;}
        Walk(Drops[GroundDrop]->GetActorLocation());
        if(Elapsed>10)Finish(false,TEXT("Ordinary walking did not collect a real box/post gold coin"));return;
    }
    if(Phase==113)
    {
        auto* Post=Field->GetPosts()[GroundPost].Get();
        if(Post->GetGenerationCount()==1)
        {Sprint(false);Move->StopMovementImmediately();GroundDrop=0;Phase=114;Elapsed=0;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderPostCircles.png"),false,false);return;}
        const FVector Radial=(Hero->GetActorLocation()-Post->GetActorLocation()).GetSafeNormal2D();
        const float Radius=FVector::Dist2D(Hero->GetActorLocation(),Post->GetActorLocation());
        const FVector Tangent(-Radial.Y,Radial.X,0);
        const FVector Desired=(Tangent*520+Radial*(145-Radius)*8).GetClampedToMaxSize2D(670);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/30,0.f,1.f));
        if(Post->GetCircleSeconds()>6.7f||Elapsed>8)
        {
            Sprint(false);if(++GroundCircleAttempts>3){Finish(false,FString::Printf(TEXT("Actual post circles missed time: post%d angle%.1f seconds%.2f"),GroundPost,Post->GetAccumulatedDegrees(),Post->GetCircleSeconds()));return;}
            GroundRetryTarget=Post->GetActorLocation()+Radial*460;Phase=116;Elapsed=0;
        }
        return;
    }
    if(Phase==116)
    {
        Walk(GroundRetryTarget);
        if(FVector::Dist2D(Hero->GetActorLocation(),Field->GetPosts()[GroundPost]->GetActorLocation())>425&&Move->IsMovingOnGround())
        {Sprint(true);Phase=113;Elapsed=0;return;}
        if(Elapsed>6)Finish(false,TEXT("Ordinary walking did not leave the post radius for a retry"));return;
    }
    if(Phase==115&&Elapsed>1)
    {
        if(!bGroundCoinsVerified||Ledger->GetTotal()!=0||Box->IsBroken()||Box->GetDropCoins().Num()!=0)
        {Finish(false,TEXT("Actual R did not clear the session score and restore its own coin box"));return;}
        for(APlayseedCoin* Coin:Field->GetCoins())if(Coin->IsCollected()){Finish(false,TEXT("Actual R did not reset the static golds"));return;}
        for(AReferenceCoinPost* Post:Field->GetPosts())if(Post->GetGenerationCount()!=0||Post->GetPoundCount()!=0||Post->GetDropCoins().Num()!=0)
        {Finish(false,TEXT("Actual R did not restore posts and remove only their own drops"));return;}
        bGroundResetVerified=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderGroundReset.png"),false,false);Phase=9;Elapsed=0;
    }
}
